/*************************************************************************************
 *  Copyright (C) 2007-2011 by Aleix Pol <aleixpol@kde.org>                          *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "analyzer.h"
#include <KLocale>
#include <kdemacros.h>

#include "operations.h"
#include "value.h"
#include "vector.h"
#include "variables.h"
#include "container.h"
#include "object.h"
#include "list.h"
#include "variable.h"
#include "analitzautils.h"
#include "expressiontypechecker.h"
#include "apply.h"
#include "providederivative.h"

using namespace AnalitzaUtils;
using namespace Analitza;

namespace Analitza
{
class BoundingIterator
{
	public:
		virtual ~BoundingIterator() {}
		virtual bool hasNext()=0;
};
}

template <class T>
QStringList printAll(const QVector<T*> & p)
{
	QStringList ret;
	foreach(T* v, p)
		if(v)
			ret += v->toString();
		else
			ret += "<null>";
	return ret;
}

const int defsize = /*500*/0;

Analyzer::Analyzer()
	: m_vars(new Variables), m_varsOwned(true), m_hasdeps(true)
{
	m_runStack.reserve(defsize);
}

Analyzer::Analyzer(Variables* v)
	: m_vars(v), m_varsOwned(false), m_hasdeps(true)
{
	m_runStack.reserve(defsize);
	Q_ASSERT(v);
}

Analyzer::Analyzer(const Analyzer& a)
	: m_exp(a.m_exp), m_err(a.m_err), m_varsOwned(true), m_hasdeps(a.m_hasdeps)
{
	m_vars = new Variables(*a.m_vars);
	m_runStack.reserve(defsize);
}

Analyzer::~Analyzer()
{
	if(m_varsOwned)
		delete m_vars;
}

void Analyzer::setExpression(const Expression & e)
{
	m_exp=e;
	flushErrors();
	
	if(m_exp.isCorrect()) {
		ExpressionTypeChecker check(m_vars);
		check.initializeVars(m_builtin.varTypes());
		m_currentType=check.check(m_exp);
		
		m_err += check.errors();
		m_hasdeps = check.hasDependencies();
	}
}

void Analyzer::importScript(QTextStream* stream)
{
	QString line;
	for(bool done=!stream->atEnd(); done; done=!stream->atEnd() || !line.isEmpty()) {
		line += stream->readLine(); // line of text excluding '\n'
		line += '\n'; //make sure the \n is passed so that comments work properly
		
		if(Expression::isCompleteExpression(line) || stream->atEnd()) {
			if(stream->atEnd() && !Expression::isCompleteExpression(line, true))
				break;
			
			setExpression(Expression(line, Expression::isMathML(line)));
			
			calculate();
			line.clear();
			
			if(!isCorrect())
				break;
		}
	}
}

Expression Analyzer::evaluate()
{
	m_err.clear();
	Expression e;
	
	if(m_exp.isCorrect()) {
		m_runStackTop = 0;
		m_runStack.clear();
		Object *o=eval(m_exp.tree(), true, QSet<QString>());
		
		o=simp(o);
		e.setTree(o);
	} else {
		m_err << i18n("Must specify a correct operation");
	}
	return e;
}

Expression Analyzer::calculate()
{
	Expression e;
	
	if(!m_hasdeps && m_exp.isCorrect()) {
		m_runStackTop = 0;
		m_runStack.clear();
		e.setTree(calc(m_exp.tree()));
	} else {
		if(m_exp.isCorrect() && m_hasdeps)
			m_err << i18n("Unknown identifier: '%1'",
							dependencies(m_exp.tree(), m_vars->keys()+m_builtin.identifiers()).join(
								i18nc("identifier separator in error message", "', '")));
		else
			m_err << i18n("Must specify a correct operation");
	}
	return e;
}

Expression Analyzer::calculateLambda()
{
	Expression e;
	
	if(KDE_ISLIKELY(!m_hasdeps && m_exp.isCorrect())) {
		Q_ASSERT(m_exp.tree()->isContainer());
		Container* math=(Container*) m_exp.tree();
		Q_ASSERT(math->m_params.first()->isContainer());
		if(math->containerType()==Container::math) {
			math=(Container*) math->m_params.first();
			Q_ASSERT(math->m_params.first()->isContainer());
		}
		
		Container* lambda=(Container*) math;
		Q_ASSERT(lambda->containerType()==Container::lambda);
		
		if(KDE_ISUNLIKELY(m_runStack.first()!=lambda))
			m_runStack.prepend(lambda);
		m_runStackTop = 0;
		e.setTree(calc(lambda->m_params.last()));
	} else {
		m_err << i18n("Must specify a correct operation");
		
		if(m_exp.isCorrect() && m_hasdeps)
			m_err << i18n("Unknown identifier: '%1'",
							dependencies(m_exp.tree(), m_vars->keys()).join(
								i18nc("identifier separator in error message", "', '")));
	}
	return e;
}

Object* Analyzer::eval(const Object* branch, bool resolve, const QSet<QString>& unscoped)
{
	Q_ASSERT(branch);
	Object *ret=0;
// 	qDebug() << "POPOPO" << branch->toString();
	
	//Won't calc() so would be a good idea to have it simplified
	if(branch->isContainer()) {
		const Container* c = (Container*) branch;
		
// 		Q_ASSERT(!c->isEmpty());
		switch(c->containerType()) {
			case Container::declare: {
				Ci *var = (Ci*) c->m_params[0];
				delete m_vars->take(var->name());
				ret = eval(c->m_params[1], true, unscoped);
				ret = simp(ret);
				insertVariable(var->name(), ret);
			}	break;
			case Container::piecewise: {
				Container::const_iterator it=c->m_params.constBegin(), itEnd=c->constEnd();
				
				bool allfalse=true;
				for(; !ret && it!=itEnd; ++it) {
					Container *p=static_cast<Container*>(*it);
					Q_ASSERT( (*it)->isContainer() &&
						(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
					bool isPiece = p->containerType()==Container::piece;
					if(isPiece) {
						Object *cond=simp(eval(p->m_params[1], resolve, unscoped));
						
						if(cond->type()==Object::value) {
							Cn* cval=static_cast<Cn*>(cond);
							if(cval->isTrue()) {
								allfalse=false;
								ret=eval(p->m_params[0], resolve, unscoped);
							}
						} else
							allfalse=false;
// 						qDebug() << "%%%%%" << cond->toString() << p->m_params[1]->toString() << allfalse;
						
						delete cond;
					} else if(allfalse) {
						ret=eval(p->m_params[0], resolve, unscoped);
					}
				}
				if(!ret)
					ret=c->copy();
				
			}	break;
			case Container::lambda: {
				QSet<QString> newUnscoped(unscoped);
				newUnscoped+=c->bvarStrings().toSet();
				
				Container *r = c->copy();
				Object* old=r->m_params.last();
				
				
				int top = m_runStackTop;
				m_runStackTop = m_runStack.size();
				m_runStack.resize(m_runStackTop+c->bvarCount()+1);
				
				r->m_params.last()=eval(old, false, newUnscoped);
				delete old;

				m_runStack.resize(m_runStackTop);
				m_runStackTop = top;
				
				alphaConversion(r, r->bvarCi().first()->depth());
				Expression::computeDepth(r);
				ret=r;
			} break;
			case Container::math:
				ret=0;
				foreach(const Object* o, c->m_params) {
					delete ret;
					ret=eval(o, resolve, unscoped);
				}
				break;
			default:
				Q_ASSERT(false);
				break;
		}
	} else if(resolve && branch->type()==Object::variable) {
		Ci* var=(Ci*) branch;
		
		if(!unscoped.contains(var->name())) {
			Object* val = variableValue(var);
			
			if(val && !equalTree(var, val)) {
				QSet<QString> newUnscoped=unscoped;
				newUnscoped.insert(var->name());
				ret = eval(val, resolve, newUnscoped);
			}
		}
		
// 		qDebug() << "peeee" << var->name() << val << resolve << unscoped;
	} else if(branch->type()==Object::vector) {
		const Vector* v=static_cast<const Vector*>(branch);
		Vector* nv=new Vector(v->size());
		Vector::const_iterator it, itEnd=v->constEnd();
		for(it=v->constBegin(); it!=itEnd; ++it) {
			Object* res=eval(*it, resolve, unscoped);
			nv->appendBranch(res);
		}
		ret=nv;
	} else if(branch->type()==Object::list) {
		const List* v=static_cast<const List*>(branch);
		List* nv=new List;
		List::const_iterator it, itEnd=v->constEnd();
		for(it=v->constBegin(); it!=itEnd; ++it) {
			Object* res=eval(*it, resolve, unscoped);
			nv->appendBranch(res);
		}
// 		qDebug() << "XXX" << ;
		ret=nv;
	} else if(branch->type()==Object::apply) {
		const Apply* c=static_cast<const Apply*>(branch);
		Operator op = c->firstOperator();
		
		switch(op.operatorType()) {
			case Operator::diff: {
				//FIXME: Must support multiple bvars
				QStringList bvars = c->bvarStrings();
				
				Q_ASSERT(!c->isEmpty());
				Object* o=derivative(bvars[0], *c->firstValue());
				
				Container* cc=new Container(Container::lambda);
				foreach(const QString& v, bvars) {
					Container* bvar=new Container(Container::bvar);
					bvar->appendBranch(new Ci(v));
					cc->appendBranch(bvar);
				}
				cc->appendBranch(simp(o));
				ret=cc; 
				
				Expression::computeDepth(ret);
			}	break;
			case Operator::function: {
				//it is a function. I'll take only this case for the moment
				//it is only meant for operations with scoped variables that _change_ its value => have a value
				Object* body=simp(eval(c->m_params[0], true, unscoped));
				
				const Container *cbody = dynamic_cast<Container*>(body);
				if(resolve && cbody && cbody->m_params.size()==c->m_params.size() && cbody->containerType()==Container::lambda) {
					int bvarsSize = cbody->bvarCount();
					QVector<Object*> args(bvarsSize+1);
					
					args[0]=cbody->copy();
					for(int i=0; i<bvarsSize; i++) {
						args[i+1]=simp(eval(c->m_params[i+1], resolve, unscoped));
					}
					int aux = m_runStackTop;
					m_runStackTop = m_runStack.size();
					m_runStack.resize(m_runStackTop+bvarsSize+1);
					
					int i=0;
					foreach(Object* o, args)
						m_runStack[m_runStackTop+i++]=o;
					ret=eval(cbody->m_params.last(), resolve, unscoped);
					
					qDeleteAll(m_runStack.begin()+m_runStackTop, m_runStack.end());
					m_runStack.resize(m_runStackTop);
					m_runStackTop = aux;
					
					Expression::computeDepth(ret);
				}
				
				if(!ret)
					ret=c->copy();
				
				delete body;
			}	break;
			case Operator::forall:
			case Operator::exists:
			case Operator::sum:
			case Operator::product: {
				Apply *r = c->copy();
				
				QSet<QString> newUnscoped(unscoped);
				int top = m_runStack.size();
				bool resolved=false;
				
				QSet<QString> bvars = c->bvarStrings().toSet();
				newUnscoped += bvars;
				m_runStack.resize(top + bvars.size());
				
				if(r->domain()) {
					QScopedPointer<Object> o(r->domain());
					r->domain()=eval(r->domain(), resolve, unscoped);
					resolved=r->domain()->type()==Object::list || r->domain()->type()==Object::vector;
				} else {
					if(r->dlimit()) {
						QScopedPointer<Object> o(r->dlimit());
						r->dlimit()=eval(r->dlimit(), resolve, unscoped);
					}
					if(r->ulimit()) {
						QScopedPointer<Object> o(r->ulimit());
						r->ulimit()=eval(r->ulimit(), resolve, unscoped);
					}
					
					resolved=r->dlimit()->type()==Object::value && r->ulimit()->type()==Object::value;
				}
				
				if(resolved && hasVars(*r->firstValue(), newUnscoped.toList())) {
					BoundingIterator *it = r->domain()? initBVarsContainer(r, top, r->domain()) : initBVarsRange(r, top, r->dlimit(), r->ulimit());
					
					if(it) {
						QList<Object*> values;
						Object* element = r->m_params.first();
						do {
							values += eval(element, resolve, unscoped);
						} while(it->hasNext());
						
						if(values.size()==1)
							ret = values.first();
						else {
							r->ulimit()=0;
							r->dlimit()=0;
							r->domain()=0;
							
							Apply *newC = new Apply;
							Operator::OperatorType t;
							switch(op.operatorType()) {
								case Operator::product: t = Operator::times; break;
								case Operator::sum:		t = Operator::plus; break;
								case Operator::forall:	t = Operator::_and; break;
								default: /*exists*/		t = Operator::_or; break;
							}
							
							newC->appendBranch(new Operator(t));
							newC->m_params = values;
							ret = newC;
						}
						delete r;
					} else
						ret = r;
					
					delete it;
				} else {
					Apply::iterator it=r->firstValue(), itEnd=r->end();
					for(; it!=itEnd; ++it) {
						Object *o=*it;
						*it= eval(*it, resolve, newUnscoped);
						delete o;
					}
					ret=r;
				}
				
// 				qDeleteAll(m_runStack.begin()+top, m_runStack.end());
				m_runStack.resize(top);
				
			}	break;
			default: {
				Q_ASSERT(!op.isBounded());
				Apply *r = c->copy();
				
				Apply::iterator it=r->firstValue(), itEnd=r->end();
				for(; it!=itEnd; ++it) {
					Object *o=*it;
					*it= eval(*it, resolve, unscoped);
					delete o;
				}
				
// 				ret=simp(r);
				ret=r;
				
			}	break;
		}
	}
	
	if(!ret)
		ret=branch->copy();
	Q_ASSERT(ret);
	
	return ret;
}

Object* Analyzer::derivative(const QString &var, const Object* o)
{
	Q_ASSERT(o);
	
	ProvideDerivative v(var);
	Object* ret = v.run(o);
	
	if(!v.isCorrect())
		m_err += v.errors();
	return ret;
}

Object* Analyzer::calcPiecewise(const Container* c)
{
	Object* ret=0;
	//Here we have a list of options and finally the otherwise option
	foreach(Object *o, c->m_params) {
		Container *p=static_cast<Container*>(o);
		Q_ASSERT( o->isContainer() &&
				(p->containerType()==Container::piece ||
				p->containerType()==Container::otherwise) );
		bool isPiece = p->containerType()==Container::piece;
		if(isPiece) {
			QScopedPointer<Cn> condition((Cn*) calc(p->m_params[1]));
			if(condition->isTrue()) {
				ret=calc(p->m_params[0]);
				break;
			}
			
		} else {
			//it is an otherwise
			ret=calc(p->m_params.first());
			break;
		}
	}
	
	if(KDE_ISUNLIKELY(!ret)) {
		m_err << i18nc("Error message, no proper condition found.", "Could not find a proper choice for a condition statement.");
		ret=new Cn(0.);
	}
	
	return ret;
}

Object* Analyzer::calcDeclare(const Container* c)
{
	Object *ret=0;
	
	const Ci *var = (const Ci*) c->m_params[0];
	ret=simp(c->m_params[1]->copy());
		
	bool corr = insertVariable(var->name(), ret);
	
	Q_ASSERT(ret && corr);
	return ret;
}

Object* Analyzer::calc(const Object* root)
{
	Q_ASSERT(root);
	Object* ret=0;
	
	switch(root->type()) {
		case Object::container:
			ret = operate((const Container*) root);
			break;
		case Object::apply:
			ret = operate((const Apply*) root);
			break;
		case Object::vector: {
			const Vector *v=static_cast<const Vector*>(root);
			Vector *nv= new Vector(v->size());
			Vector::const_iterator it, itEnd=v->constEnd();
			
			for(it=v->constBegin(); it!=itEnd; ++it)
				nv->appendBranch(calc(*it));
			
			ret = nv;
		}	break;
		case Object::list: {
			const List *v=static_cast<const List*>(root);
			List *nv= new List;
			List::const_iterator it, itEnd=v->constEnd();
			
			for(it=v->constBegin(); it!=itEnd; ++it)
				nv->appendBranch(calc(*it));
			
			ret = nv;
		}	break;
		case Object::value:
		case Object::custom:
			ret=root->copy();
			break;
		case Object::variable: {
			Ci* a=(Ci*) root;
			
			ret=variableValue(a);
			if(ret)
				ret = calc(ret);
			else {
				Container* c= new Container(Container::lambda);
				c->appendBranch(a->copy());
				ret=c;
			}
		}	break;
		case Object::oper:
		case Object::none:
			break;
	}
	
	Q_ASSERT(ret);
	return ret;
}

Object* Analyzer::operate(const Apply* c)
{
	Object* ret=0;
	Operator op = c->firstOperator();
	Operator::OperatorType opt=op.operatorType();
	
	switch(opt) {
		case Operator::sum:
			ret = sum(*c);
			break;
		case Operator::product:
			ret = product(*c);
			break;
		case Operator::forall:
			ret = forall(*c);
			break;
		case Operator::exists:
			ret = exists(*c);
			break;
		case Operator::function:
			ret = func(*c);
			break;
		case Operator::none:
			ret = calc(*c->firstValue());
			break;
		case Operator::diff: {
			//TODO: Make multibvar
			QList<Ci*> bvars=c->bvarCi();
			
			//We construct the lambda
			Object* o=derivative(bvars[0]->name(), *c->firstValue());
			o=simp(o);
			
			Container* cc=new Container(Container::lambda);
			foreach(const Ci* v, bvars) {
				Container* bvar=new Container(Container::bvar);
				bvar->appendBranch(v->copy());
				cc->appendBranch(bvar);
			}
			
			cc->appendBranch(o);
			ret=cc;
			
			Expression::computeDepth(ret);
		}	break;
		default: {
			int count=c->countValues();
			Q_ASSERT(count>0);
			Q_ASSERT(	(op.nparams()< 0 && count>1) ||
						(op.nparams()>-1 && count==op.nparams()) ||
						opt==Operator::minus);
			
			QString* error=0;
			if(count>=2) {
				Apply::const_iterator it = c->firstValue(), itEnd=c->constEnd();
				
				ret = calc(*it);
				++it;
				for(; it!=itEnd; ++it) {
					ret=Operations::reduce(opt, ret, calc(*it), &error);
					
					if(KDE_ISUNLIKELY(error)) {
						m_err.append(*error);
						delete error;
						error=0;
						break;
					}
				}
			} else {
				ret=Operations::reduceUnary(opt, calc(*c->firstValue()), &error);
				if(KDE_ISUNLIKELY(error)) {
					m_err.append(*error);
					delete error;
				}
			}
		}	break;
	}
	
	Q_ASSERT(ret);
	return ret;
}

Object* Analyzer::operate(const Container* c)
{
	Q_ASSERT(c);
// 	Q_ASSERT(!c->isEmpty()); //Not valid anymore since we can have empty <math/>
	Object* ret=0;
	
	switch(c->containerType()) {
		case Container::math:
			ret=calc(*c->constBegin());
			break;
		case Container::declare:
			ret=calcDeclare(c);
			break;
		case Container::piecewise:
			ret=calcPiecewise(c);
			break;
		case Container::lambda: {
			Container * cc=c->copy();
// 			AnalitzaUtils::objectWalker(cc, "aa");
			if(cc->bvarCount()>0)
				alphaConversion(cc, cc->bvarCi().first()->depth());
// 			AnalitzaUtils::objectWalker(cc, "bb");
			Expression::computeDepth(cc);
// 			AnalitzaUtils::objectWalker(cc, "cc");
// 			qDebug() << "PAAAAAAAAAAAAAA" << printAll(m_runStack);
			ret=cc;
		}	break;
		case Container::piece:
		case Container::otherwise:
		case Container::bvar:
		case Container::uplimit:
		case Container::downlimit:
		case Container::domainofapplication:
		case Container::none:
			Q_ASSERT(false && "tried to calculate a wrong item");
			break;
	}
	Q_ASSERT(ret);
	return ret;
}

namespace Analitza
{
	template <typename T, typename Iterator>
	class TypeBoundingIterator : public BoundingIterator
	{
		public:
			TypeBoundingIterator(QVector<Object*>& runStack, int top, const QVector<Ci*>& vars, T* l)
				: iterators(vars.size()), list(l)
				, itBegin(l->constBegin()), itEnd(l->constEnd())
				, m_runStack(runStack), m_top(top)
			{
				int s=vars.size();
				for(int i=0; i<s; i++) {
					m_runStack[m_top+i]=*itBegin;
					iterators[i]=itBegin;
				}
			}
			
			~TypeBoundingIterator() { delete list; }
			
			virtual bool hasNext()
			{
				bool cont=true;
				for(int i=iterators.size()-1; cont && i>=0; i--) {
					++iterators[i];
					cont=(iterators[i]==itEnd);
					
					if(cont)
						iterators[i]=itBegin;
					m_runStack[m_top+i]=*iterators[i];
				}
				
				return !cont;
			}
		private:
			QVector<Iterator> iterators;
			T* list;
			const Iterator itBegin, itEnd;
			QVector<Object*>& m_runStack;
			int m_top;
	};
	
	class RangeBoundingIterator : public BoundingIterator
	{
		public:
			RangeBoundingIterator(const QVector<Cn*>& values, Cn* oul, Cn* odl, double step)
				: values(values), dl(odl->value()), ul(oul->value()), step(step), objdl(odl), objul(oul)
			{}
			
			~RangeBoundingIterator()
			{
				qDeleteAll(values);
				delete objdl;
				delete objul;
			}
			
			bool hasNext()
			{
				bool cont=true;
				for(int i=values.size()-1; cont && i>=0; i--) {
					Cn* v=values[i];
					double curr=v->value()+step;
					cont=curr>ul;
					
					v->setValue(cont ? dl : curr);
				}
				
				return !cont;
			}
			
		private:
			const QVector<Cn*> values;
			const double dl, ul, step;
			Object* objdl;
			Object* objul;
	};
}

BoundingIterator* Analyzer::initializeBVars(const Apply* n, int base)
{
	BoundingIterator* ret=0;
	
	Object* domain=n->domain();
	
	if(domain)
	{
		domain=calc(domain);
		ret = initBVarsContainer(n, base, domain);
		
		if(!ret)
			delete domain;
	}
	else
	{
		Object *objul=calc(n->ulimit());
		Object *objdl=calc(n->dlimit());
		
		ret = initBVarsRange(n, base, objdl, objul);
		
		if(!ret) {
			delete objdl;
			delete objul;
		}
	}
	return ret;
}
BoundingIterator* Analyzer::initBVarsContainer(const Analitza::Apply* n, int base, Object* domain)
{
	BoundingIterator* ret = 0;
	QList<Ci*> bvars=n->bvarCi();
	
	switch(domain->type()) {
		case Object::list:
			if(static_cast<List*>(domain)->size()>0)
				ret=new TypeBoundingIterator<List, List::const_iterator>(m_runStack, base, bvars.toVector(), static_cast<List*>(domain));
			break;
		case Object::vector:
			if(static_cast<Vector*>(domain)->size()>0)
				ret=new TypeBoundingIterator<Vector, Vector::const_iterator>(m_runStack, base, bvars.toVector(), static_cast<Vector*>(domain));
			break;
		default:
			Q_ASSERT(false && "Type not supported for bounding.");
			m_err.append(i18n("Type not supported for bounding."));
	}
	return ret;
}

BoundingIterator* Analyzer::initBVarsRange(const Apply* n, int base, Object* objdl, Object* objul)
{
	BoundingIterator* ret = 0;
	if(isCorrect() && objul->type()==Object::value && objdl->type()==Object::value) {
		Cn *u=static_cast<Cn*>(objul);
		Cn *d=static_cast<Cn*>(objdl);
		double ul=u->value();
		double dl=d->value();
		
		if(dl<=ul) {
			QList<Ci*> bvars=n->bvarCi();
			QVector<Cn*> rr(bvars.size());
			
			for(int i=0; i<bvars.size(); ++i) {
				rr[i]=new Cn(dl);
				m_runStack[base+i]=rr[i];
			}
			
			ret=new RangeBoundingIterator(rr, u, d, 1);
		} else
			m_err.append(i18n("The downlimit is greater than the uplimit"));
	} else
		m_err.append(i18n("Incorrect uplimit or downlimit."));
	return ret;
}

Object* Analyzer::boundedOperation(const Apply& n, const Operator& t, Object* initial)
{
	Object* ret=initial;
	int top = m_runStack.size();
	m_runStack.resize(top+n.bvarCi().size());
	
	BoundingIterator* it=initializeBVars(&n, top);
	if(!it)
		return initial;
	
	QString* correct=0;
	Operator::OperatorType type=t.operatorType();
	do {
		Object *val=calc(n.m_params.last());
		ret=Operations::reduce(type, ret, val, &correct);
	} while(KDE_ISLIKELY(it->hasNext() && !correct));
	
	m_runStack.resize(top);
	
	delete it;
	Q_ASSERT(ret);
	return ret;
}

Object* Analyzer::product(const Apply& n)
{
	return boundedOperation(n, Operator(Operator::times), new Cn(1.));
}

Object* Analyzer::sum(const Apply& n)
{
	return boundedOperation(n, Operator(Operator::plus), new Cn(0.));
}

Object* Analyzer::forall(const Apply& n)
{
	return boundedOperation(n, Operator(Operator::_and), new Cn(true));
}

Object* Analyzer::exists(const Apply& n)
{
	return boundedOperation(n, Operator(Operator::_or), new Cn(false));
}

Object* Analyzer::func(const Apply& n)
{
// 	qDebug() << "calling" << n.toString();
	bool borrowed = n.m_params[0]->type()==Object::variable;
	Container *function=0;
	if(borrowed)
		function = (Container*) variableValue((Ci*) n.m_params[0]);
	else
		function = (Container*) calc(n.m_params[0]);
	
	int bvarsize = n.m_params.size()-1;
	Object* ret=0;
	if(function && function->m_params.size()>1) {
		int top = m_runStack.size(), aux=m_runStackTop;
		m_runStack.resize(top+bvarsize+1);
		
		m_runStack[top] = function;
		for(int i=0; i<bvarsize; i++) {
	// 		qDebug() << "cp" << n.m_params[i+1]->toString();
			Object* val=calc(n.m_params[i+1]);
			m_runStack[top+i+1] = val;
	// 		qDebug() << "parm" << i << n.m_params[i+1]->toString() << val->toString();
		}
		m_runStackTop = top;
		
// 		qDebug() << "diiiiiiiii" << function->toString() << m_runStack.size() << bvarsize << m_runStackTop << printAll(m_runStack);
		ret=calc(function->m_params.last());
		
		qDeleteAll(m_runStack.begin()+top+1, m_runStack.end());
		if(!borrowed)
			delete function;
		
		m_runStackTop = aux;
		m_runStack.resize(top);
	} else {
		Q_ASSERT(function ? (function->m_params[0]->type()==Object::variable && function->m_params.size()==1) : n.m_params[0]->type()==Object::variable);
		QString id=static_cast<const Ci*>(function ? function->m_params[0] : n.m_params[0])->name();
		FunctionDefinition* func=m_builtin.function(id);
		QList<Expression> args;
		
// 		qDebug() << "calling..." << id << n.m_params.size() << n.toString();
		for(int i=1; i<bvarsize+1; i++) {
	// 		qDebug() << "cp" << n.m_params[i]->toString();
			Object* val=calc(n.m_params[i]);
			args += Expression(val);
// 			qDebug() << "parm" << i << n.m_params[i]->toString() << args.last().toString();
		}
		Expression exp=(*func)(args);
		if(exp.isCorrect()) {
			ret=exp.tree();
			exp.setTree(0);
		} else {
			m_err += exp.error();
			ret = new Cn;
		}
// 		qDebug() << "called" << ret->toString();
	}
	
	return ret;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void Analyzer::simplify()
{
	if(m_exp.isCorrect()) {
		m_runStackTop = 0;
		Object* o=simp(m_exp.tree());
		m_exp.setTree(o);
		setExpression(m_exp);
	}
}

void Analyzer::levelOut(Apply *c, Apply *ob, Apply::iterator &pos)
{
	Container::iterator it = ob->firstValue();
	for(; it!=ob->end(); pos++) {
		pos=c->m_params.insert(pos, *it);
		
		it=ob->m_params.erase(it);
	}
}

template <class T, class Tit>
void Analyzer::iterateAndSimp(T* v)
{
	Tit it = v->begin(), itEnd=v->end();
	
	for(; it!=itEnd; ++it)
		*it = simp(*it);
}

Object* Analyzer::simp(Object* root)
{
	Q_ASSERT(root && root->type()!=Object::none);
	if(!isCorrect())
		return root;
	
	if(!root->isContainer() && !hasVars(root))
	{
		if(root->type()!=Object::value && root->type() !=Object::oper) {
			Object* aux=root;
			root = calc(root);
			delete aux;
			
			if(isLambda(root))
				root = simp(root);
		}
	} else if(root->type()==Object::vector) {
		iterateAndSimp<Vector, Vector::iterator>(static_cast<Vector*>(root));
	} else if(root->type()==Object::list) {
		iterateAndSimp<List, List::iterator>(static_cast<List*>(root));
	} else if(root->type()==Object::apply) {
		root = simpApply((Apply*) root);
	} else if(root->isContainer()) {
		Container *c= (Container*) root;
		
		switch(c->containerType()) {
			case Container::piecewise:
				root=simpPiecewise(c);
				break;
			case Container::lambda: {
				int top = m_runStackTop;
				m_runStackTop = m_runStack.size();
				m_runStack.resize(m_runStackTop+c->bvarCount()+1);
				
				c->m_params.last()=simp(c->m_params.last());
				m_runStack.resize(m_runStackTop);
				m_runStackTop = top;
			}	break;
			default:
				iterateAndSimp<Container, Container::iterator>(c);
				break;
		}
	}
	return root;
}

Object* Analyzer::simpApply(Apply* c)
{
	Object* root=c;
	Container::iterator it;
	Operator o = c->firstOperator();
	bool d;
	
	switch(o.operatorType()) {
		case Operator::times:
			for(it=c->firstValue(); c->m_params.count()>1 && it!=c->end();) {
				d=false;
				*it = simp(*it);
				if((*it)->isApply()) {
					Apply *intr = static_cast<Apply*>(*it);
					if(intr->firstOperator()==o) {
						levelOut(c, intr, it);
						d=true;
					}
				}
				
				if(!d && (*it)->type() == Object::value) {
					Cn* n = (Cn*) (*it);
					if(n->value()==1. && c->m_params.count()>1) { //1*exp=exp
						d=true;
					} else if(n->value()==0.) { //0*exp=0 //TODO Change to isZero and return the same type in 0
						delete root;
						root = new Cn(0.);
						return root;
					}
				}
				
				if(!d)
					++it;
				else {
					delete *it;
					it = c->m_params.erase(it);
				}
			}
			
			if(c->isUnary()) {
				Apply *aux=c;
				root=*c->firstValue();
				*aux->firstValue()=0;
				delete aux;
			} else {
				Object *ret=simpScalar(c);
				if(ret->isApply()) {
					c=static_cast<Apply*>(ret);
					ret=simpPolynomials(c);
					c=ret->isContainer() ? static_cast<Apply*>(ret) : 0;
				} else
					c=0;
				
				if(c && c->isEmpty()) {
					delete root;
					root = new Cn(0.);
				} else
					root=ret;
			}
			break;
		case Operator::minus:
		case Operator::plus: {
			bool somed=false, lastdel=false;
			it=c->end()-1;
			Object* first=*c->firstValue();
			
			bool done=false;
			
			for(; !done; --it) {
				done=it==c->m_params.begin();
				lastdel=false;
				*it = simp(*it);
				
				d=false;
				
				if((*it)->isApply()) {
					Apply *intr = (Apply*) *it;
					Operator op=intr->firstOperator();
					if(op==o || (*it!=first && op==Operator::plus && o==Operator::minus))
					{
						if(!(intr->isUnary() && op==Operator::minus))
						{
							levelOut(c, intr, it);
							d=true;
						}
					}
				}
				
				if(!d && ((*it)->type()==Object::value || ((*it)->type()==Object::vector && !hasVars(*it))) && (*it)->isZero()) {
					d=true;
					lastdel=true;
				}
				
				if(d) {
					somed=true;
					delete *it;
					if(first==*it) first=0;
					it = c->m_params.erase(it);
				}
			}
			
// 			qDebug()<< "KOKOKO" << delme << c->toString() << lastdel;
			
			if(lastdel && o==Operator::minus && c->countValues()>1) {
				Apply::iterator it=c->firstValue();
				Apply* cc=new Apply;
				cc->appendBranch(new Operator(Operator::minus));
				cc->appendBranch(*it);
				*it=cc;
			}
			root=c;
			
// 			qDebug()<< "PEPEPE" << delme << c->toString();
			if(c->isUnary()) {
				if(o==Operator::plus || (somed && !lastdel)) {
					root=*c->firstValue();
					*c->firstValue()=0;
					delete c;
					c=0;
				} else if((*c->firstValue())->isApply()) {
					Apply *c1=(Apply*) *c->firstValue();
					if( c1->firstOperator()==Operator::minus &&
						c1->isUnary())
					{
						root=*c1->firstValue();
						*c->firstValue()=0;
						*c1->firstValue()=0;
						delete c;
						delete c1;
						c=0;
					}
				}
			} else {
				root=simpScalar(c);
				
				if(root->isApply()) {
					c=static_cast<Apply*>(root);
					
					root=simpPolynomials(c);
					
					c=root->isApply() ? static_cast<Apply*>(root) : 0;
				} else
					c=0;
			}
// 			qDebug()<< "PAAPPA" << root->toString();
			
			if(c && c->isEmpty()) {
				delete root;
				root = new Cn(0.);
			}
		}	break;
		case Operator::power: {
			for(it = c->firstValue(); it!=c->end(); ++it)
				*it = simp(*it);
			
			if(c->m_params[1]->type()==Object::value) {
				Cn *n = (Cn*) c->m_params[1];
				if(n->value()==0.) { //0*exp=0
					delete root;
					root = new Cn(1.);
					break;
				} else if(n->value()==1.) { 
					root = c->m_params[0];
					c->m_params[0]=0;
					delete c;
					break;
				}
			}
			
			if(c->m_params[0]->isApply()) {
				Apply *cp = (Apply*) c->m_params[0];
				if(cp->firstOperator()==Operator::power) {
					c->m_params[0] = cp->m_params[0];
					
					Apply *cm = new Apply;
					cm->appendBranch(new Operator(Operator::times));
					cm->appendBranch(c->m_params[1]);
					cm->appendBranch(cp->m_params[1]);
					c->m_params[1] = cm;
					
					cp->m_params[0]=0;
					cp->m_params[1]=0;
					delete cp;
					c->m_params[1]=simp(c->m_params[1]);
				}
			}
		} break;
		case Operator::divide:
			for(it = c->firstValue(); it!=c->end(); ++it)
				*it = simp(*it);
			
			Object *f, *g; //f/g
			f=*c->firstValue();
			g=*(c->firstValue()+1);
			
			if(equalTree(f, g)) {
				delete root;
				root = new Cn(1.);
			} else if(g->type()==Object::value) {
				Cn* gnum=(Cn*) g;
				if(gnum->value()==1.) {
					*c->firstValue()=0;
					delete root;
					root=f;
					break;
				}
			}
			break;
		case Operator::ln:
			it = c->firstValue();
			if((*it)->type()==Object::variable) {
				Ci *val = (Ci*) *it;
				if(val->name()=="e") {
					delete root;
					root = new Cn(1.);
					break;
				}
			} else {
				*it = simp(*it);
			}
			break;
		//TODO: extend to ::product
		case Operator::sum: {
			
			QStringList bvars=c->bvarStrings();
			if(c->ulimit()) c->ulimit()=simp(c->ulimit());
			if(c->dlimit()) c->dlimit()=simp(c->dlimit());
			if(c->domain()) c->domain()=simp(c->domain());
			
			Object *uplimit=c->ulimit(), *downlimit=c->dlimit(), *domain=c->domain();
			
			//TODO: simplify this code
			for(it = c->m_params.begin(); it!=c->end(); ++it)
				*it = simp(*it);
			
			//if bvars is empty, we are dealing with an invalid sum()
			Object* function = *c->firstValue();
			if(!bvars.isEmpty() && !domain && !hasTheVar(bvars.toSet(), function)) {
				Apply *cDiff=new Apply;
				cDiff->appendBranch(new Operator(Operator::minus));
				cDiff->appendBranch(uplimit  ->copy());
				cDiff->appendBranch(downlimit->copy());
				
				Apply *aPlusOne = new Apply;
				aPlusOne->appendBranch(new Operator(Operator::plus));
				aPlusOne->appendBranch(new Cn(1.));
				aPlusOne->appendBranch(cDiff);
				
				Apply *nc=new Apply;
				nc->appendBranch(new Operator(Operator::times));
				nc->appendBranch(aPlusOne);
				nc->appendBranch(function);
				
				c->m_params.last()=0;
				delete c;
				root=simp(nc);
			} else if(function->isApply()) {
				root=simpSum(c);
			}
			
		}	break;
		case Operator::card: {
			Object* val=simp(*c->firstValue());
			if(val->type()==Object::vector)
			{
				c->m_params.last()=0;
				QString* err=0;
				val=Operations::reduceUnary(Operator::card, val, &err);
				if(KDE_ISUNLIKELY(err)) { delete err; }
				delete c;
				root=val;
			}
		}	break;
		case Operator::selector: {
			c->m_params[0]=simp(c->m_params[0]);
			c->m_params[1]=simp(c->m_params[1]);
			
			Object* idx=c->m_params[0];
			Object* value=c->m_params[1];
			if(idx->type()==Object::value && value->type()==Object::vector) {
				QString* err=0;
				Object* ret=Operations::reduce(Operator::selector, idx, value, &err);
				
				if(ret) {
					root=ret;
					c->m_params[0]=0;
					c->m_params[1]=0;
					
					delete c;
				}
			}
		}	break;
		case Operator::_union: {
			Apply::iterator it=c->firstValue(), itEnd=c->end();
			
			QList<Object*> newParams;
			for(; it!=itEnd; ++it) {
				*it=simp(*it);
				
				if(newParams.isEmpty())
					newParams.append(*it);
				else {
					QString* err=0;
					if((*it)->type()==Object::list && newParams.last()->type()==Object::list) {
						Object* ret=Operations::reduce(Operator::_union, newParams.last(), (*it)->copy(), &err);
						newParams.last()=ret;
						delete *it;
					} else {
						newParams.append(*it);
					}
				}
				*it=0;
			}
			
			if(newParams.last()==0)
				newParams.removeLast();
			
			//if only one, remove union
			if(newParams.size()==1) {
				delete c;
				root=newParams.last();
			} else {
				qDeleteAll(c->m_params);
				c->m_params=newParams;
				root=c;
			}
			
		}	break;
		case Operator::eq: {
			Apply::iterator it=c->firstValue()+1, itEnd=c->end();
			
			bool alleq=true;
			for(; alleq && it!=itEnd; ++it) {
				alleq=equalTree(*c->firstValue(), *it);
			}
			
			if(alleq) {
				delete c;
				root = new Cn(true);
			}
			
		}	break;
		case Operator::function: {
			Object* function=c->m_params[0];
			
			Container* cfunc=0;
			if(function->isContainer()) {
				cfunc=(Container*) function;
				Q_ASSERT(cfunc->containerType()==Container::lambda);
			}
			
			bool allvars=true;
			it=c->m_params.begin()+1;
			for(int i=0; it!=c->end(); ++it, ++i) {
				*it = simp(*it);
				allvars &= (*it)->type()==Object::variable;
			}
			
			if(cfunc && allvars) {
				QList<Ci*> bvars=cfunc->bvarCi();
				int i=0;
				foreach(Ci* var, bvars) {
					replaceDepth(var->depth(), cfunc->m_params.last(), c->m_params[i+1]);
					i++;
				}
				
				root=cfunc->m_params.last();
				cfunc->m_params.last()=0;
				delete c;
			}
		}	break;
		default:
			if(c->ulimit())
				c->ulimit()=simp(c->ulimit());
			if(c->dlimit())
				c->dlimit()=simp(c->dlimit());
			if(c->domain())
				c->domain()=simp(c->domain());
			
			iterateAndSimp<Apply, Apply::iterator>(c);
			break;
	}
	
	return root;
}

Object* Analyzer::simpScalar(Apply * c)
{
	Object *value=0;
	Container::iterator i = c->firstValue();
	Operator o = c->firstOperator();
	bool firstvalue = i!=c->end() && ((*i)->type()==Object::value || ((*i)->type()==Object::vector && !hasVars(*i)));
	for(; i!=c->end();) {
		bool d=false;
		
		//TODO: hasVars needed? should have already been simplifyed before, just check type==cn
		if((*i)->type()==Object::value || ((*i)->type()==Object::vector && !hasVars(*i))) {
			Object* aux = *i;
			
			if(value) {
				QString* err=0;
				value=Operations::reduce(o.operatorType(), value, aux, &err);
			} else
				value=aux;
			d=true;
		}
		if(d)
			i = c->m_params.erase(i);
		else
			++i;
	}
	
	if(value) {
		if(value->isZero())
			delete value;
		else if(o==Operator::plus || (!firstvalue && o==Operator::minus))
			c->appendBranch(value);
		else
			c->insertBranch(c->firstValue(), value);
	}
	return c;
}

namespace Analitza
{
typedef QPair<double, Object*> Monomial;

Object* createMono(const Operator& o, const Monomial& p)
{
	Operator::OperatorType mult = o.multiplicityOperator();
	
	Object* toAdd=0;
	if(p.first==0.) {
		delete p.second;
	} else if(p.first==1.) {
		toAdd=p.second;
	} else if(p.first==-1. && mult==Operator::times) {
		Apply *cint = new Apply;
		cint->appendBranch(new Operator(Operator::minus));
		cint->appendBranch(p.second);
		toAdd=cint;
	} else if(mult==Operator::times && p.second->isApply() && static_cast<Apply*>(p.second)->firstOperator()==Operator::times) {
		Apply* res = static_cast<Apply*>(p.second);
		res->prependBranch(new Cn(p.first));
		toAdd=res;
	} else {
		Apply *cint = new Apply;
		cint->appendBranch(new Operator(mult));
		cint->appendBranch(p.second);
		cint->appendBranch(new Cn(p.first));
		if(mult==Operator::times)
			cint->m_params.swap(0,1);
		toAdd=cint;
	}
	return toAdd;
}

Monomial constructMonomial(const Operator& o, Object* o2, bool& sign)
{
	bool ismono=false;
	Monomial imono;
	Operator::OperatorType mult = o.multiplicityOperator();
	
	if(o2->isApply()) {
		Apply *cx = (Apply*) o2;
		if(cx->firstOperator()==mult) {
			if(cx->m_params.count()==2) {
				bool valid=false;
				int scalar, var;
				
				if(mult!=Operator::power && cx->m_params[0]->type()==Object::value) {
					scalar=0;
					var=1;
					valid=true;
				} else if(cx->m_params[1]->type()==Object::value) {
					scalar=1;
					var=0;
					valid=true;
				}
				
				if(valid) {
					Cn* sc= (Cn*) cx->m_params[scalar];
					imono.first = sc->value();
					imono.second = cx->m_params[var];
					
					ismono=true;
				}
			} else if(mult==Operator::times) {
				imono.first=1;
				Apply::iterator it=cx->firstValue(), itEnd=cx->end();
				QList<Object*> vars;
				
				for(; it!=itEnd; ++it) {
					if((*it)->type()==Object::value) {
						imono.first *= static_cast<Cn*>(*it)->value();
						ismono=true;
					} else {
						vars += *it;
					}
				}
				
				if(ismono) {
					cx->m_params = vars;
					imono.second = cx;
				}
			}
		} else if(cx->firstOperator()==Operator::minus && cx->isUnary()) {
			imono = constructMonomial(o, *cx->firstValue(), sign);
			ismono=true;
				
			if(o==Operator::times)
				sign = !sign;
			else if(o==Operator::plus || o==Operator::minus)
				imono.first *= -1;
		}
	}
	
	if(!ismono) {
		imono.first = 1.;
		imono.second = o2;
	}
	
	return imono;
}

}

Object* Analyzer::simpPolynomials(Apply* c)
{
	Q_ASSERT(c!=0 && dynamic_cast<Apply*>(c));
	
	QList<Monomial> monos;
	Operator o(c->firstOperator());
	bool sign=true, first=true;
	
	for(Apply::const_iterator it=c->m_params.constBegin(), itEnd=c->constEnd(); it!=itEnd; ++it) {
		Monomial imono = constructMonomial(o, *it, sign);
		
		if(o==Operator::minus && !first)
			imono.first*=-1;
		
		bool found = false;
		QList<Monomial>::iterator it1(monos.begin());
		for(; it1!=monos.end(); ++it1) {
			Object *o1=it1->second, *o2=imono.second;
			found = equalTree(o1, o2);
			if(found)
				break;
		}
		
// 		qDebug() << "->" << c->toString() << c->firstOperator().toString() << found;
		if(found) {
			it1->first += imono.first;
			
			if(it1->first==0.) {
				delete it1->second;
				monos.erase(it1);
			}
		} else {
			imono.second = imono.second->copy();
			monos.append(imono);
		}
		first=false;
	}
	
	delete c;
	c=0;
	
	Object *root=0;
	if(monos.count()==1) {
		root=createMono(o, monos.first());
	} else if(monos.count()>1) {
		c= new Apply;
		c->appendBranch(new Operator(o));
		
		QList<Monomial>::iterator i=monos.begin();
		bool first=true;
		for(; i!=monos.end(); ++i) {
			if(!first && o==Operator::minus)
				i->first *= -1;
			Object* toAdd=createMono(o, *i);
			
			if(toAdd)
				c->appendBranch(toAdd);
			first=false;
		}
		root=c;
	}
	
	if(!sign && root) {
		Apply *cn=new Apply;
		cn->appendBranch(new Operator(Operator::minus));
		cn->appendBranch(root);
		root=cn;
	}
	
	if(!root) {
		delete c;
		root=new Cn(0.);
	}
	
	return root;
}

Object* Analyzer::simpSum(Apply* c)
{
	Object* ret=c;
	Apply* cval=static_cast<Apply*>(*c->firstValue());
	
	if(cval->isApply() && cval->firstOperator()==Operator::times) {
		QSet<QString> bvars=c->bvarStrings().toSet();
		QList<Object*> sum, out;
		Apply::iterator it=cval->firstValue(), itEnd=cval->end();
		for(; it!=itEnd; ++it) {
			if(hasTheVar(bvars, *it)) {
				sum.append(*it);
			} else {
				out.append(*it);
				*it=0;
			}
		}
		int removed=cval->m_params.removeAll(0);
		
		if(removed>0) {
			Apply* nc=new Apply;
			nc->appendBranch(new Operator(Operator::times));
			nc->m_params=out;
			nc->appendBranch(c);
			
			cval->m_params=sum;
			if(sum.count()==1) {
				cval->m_params.clear();
				delete cval;
				c->m_params.last()=sum.last();
			}
			
			ret=simp(nc);
		}
	}
	
	return ret;
}

Object* Analyzer::simpPiecewise(Container *c)
{
	Object *root=c;
	//Here we have a list of options and finally the otherwise option
	Container *otherwise=0;
	Container::const_iterator it=c->m_params.constBegin(), itEnd=c->constEnd();
	QList<Object*> newList;
	
	for(; !otherwise && it!=itEnd; ++it) {
		Container *p=static_cast<Container*>(*it);
		Q_ASSERT( (*it)->isContainer() &&
				(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
		bool isPiece = p->containerType()==Container::piece;
		
		p->m_params.last()=simp(p->m_params.last());
			
		if(isPiece) {
			if(p->m_params[1]->type()==Object::value) {
				Cn* cond=static_cast<Cn*>(p->m_params[1]);
				if(cond->isTrue()) {
					delete p->m_params.takeLast();
					p->setContainerType(Container::otherwise);
					isPiece=false;
					
					p->m_params[0]=simp(p->m_params[0]);
					otherwise=p;
					newList.append(p);
				} else {
					delete p;
				}
			} else {
				//TODO: It would be nice if we could simplify:
				// if(x=n) simplify x as n
				p->m_params[0]=simp(p->m_params[0]);
				newList.append(p);
			}
		} else { //it is an otherwise
			otherwise=p;
			newList.append(p);
		}
	}
	qDeleteAll(it, itEnd);
	
	if(newList.count()==1 && otherwise) {
		root=otherwise->m_params.takeAt(0);
		delete otherwise;
		c->m_params.clear();
		delete c;
	} else
		c->m_params = newList;
	return root;
}

Expression Analyzer::derivative(const QString& var)
{
	Q_ASSERT(m_exp.isCorrect());
	
	Expression exp;
	QStringList vars;
	Object* deriv=m_exp.tree();
	if(m_exp.isLambda()){
		Q_ASSERT(deriv->isContainer());
		Container* lambda=(Container*) deriv;
		if(lambda->containerType()==Container::math) {
			Q_ASSERT(lambda->m_params.first()->isContainer());
			lambda = (Container*) lambda->m_params.first();
		}
		Q_ASSERT(lambda->containerType()==Container::lambda);
		
		vars=lambda->bvarStrings();
		deriv=lambda->m_params.last();
	} else
		vars += var;
	
// 	Q_ASSERT(hasTheVar(QSet<QString>() << var, deriv));
	Object* o = derivative(var, deriv);
	o=simp(o);
	Container* lambda=new Container(Container::lambda);
	foreach(const QString& dep, vars) {
		Container* bvar=new Container(Container::bvar);
		bvar->appendBranch(new Ci(dep));
		lambda->appendBranch(bvar);
	}
	lambda->appendBranch(o);
	
	exp.setTree(lambda);
	
	return exp;
}

Expression Analyzer::dependenciesToLambda() const
{
	if(m_hasdeps) {
		QStringList deps=dependencies(m_exp.tree(), m_vars->keys());
		Container* cc=new Container(Container::lambda);
		foreach(const QString& dep, deps) {
			Container* bvar=new Container(Container::bvar);
			bvar->appendBranch(new Ci(dep));
			cc->appendBranch(bvar);
		}
		
		Object* root=m_exp.tree()->copy();
		if(root->isContainer()) {
			Container* c=static_cast<Container*>(root);
			if(c->containerType()==Container::math) {
				root=c->m_params.first();
				c->m_params.first()=0;
				delete c;
			}
		}
		cc->appendBranch(root);
		
		Container* math=new Container(Container::math);
		math->appendBranch(cc);
		Expression::computeDepth(math);
		
		return Expression(math);
	} else {
		return m_exp;
	}
}

bool Analyzer::insertVariable(const QString & name, const Expression & value)
{
	return insertVariable(name, value.tree());
}

bool Analyzer::insertVariable(const QString & name, const Object * value)
{
	bool wrong=!isLambda(value) && hasTheVar(QSet<QString>() << name, value);
	if(wrong)
		m_err << i18nc("By a cycle i mean a variable that depends on itself", "Defined a variable cycle");
	else
		m_vars->modify(name, value);
	
	return !wrong;
}

Cn* Analyzer::insertValueVariable(const QString& name, double value)
{
	Cn* val=m_vars->modify(name, value);
	return val;
}

double Analyzer::derivative(const QVector<Object*>& values )
{
	//c++ numerical recipes p. 192. Only for f'
	//Image
	Q_ASSERT(m_exp.isCorrect());
	Q_ASSERT(values.size()==m_exp.bvarList().size());
	
	setStack(values);
	
	Expression e1(calc(m_exp.tree()));
	if(!isCorrect())
		return 0.;
	
	//Image+h
	double h=0.0000000001;
	for(int i=0; i<values.size(); i++) {
// 		volatile double temp=valp.second+h;
// 		double hh=temp-valp.second;
		
		Q_ASSERT(values[i]->type()==Object::value);
		Cn* v=static_cast<Cn*>(values[i]);
		v->setValue(v->value()+h);
	}
	
	Expression e2(calc(m_exp.tree()));
	if(!isCorrect())
		return 0.;
	
	if(!e1.isReal() || !e2.isReal()) {
		m_err << i18n("The result is not a number");
		return 0;
	}
	
	return (e2.toReal().value()-e1.toReal().value())/h;
}

Analitza::Object* Analyzer::variableValue(Ci* var)
{
	Object* ret;
	
// 	qDebug() << "ziiiiiiii" << var->name() << '('<< m_runStackTop << '+' << var->depth() << ')' << printAll(m_runStack);
	if(var->depth()>=0)
		ret = m_runStack[m_runStackTop + var->depth()];
	else
		ret = m_vars->value(var->name());
	
// 	static int hits = 0, misses = 0;
// 	if(var->depth()>=0) hits++; else misses++;
// 	qDebug() << "pepepe" << hits << misses;
	return ret;
}

Object* Analyzer::applyAlpha(Object* o, int min)
{
	if(o)
		switch(o->type()) {
			case Object::container:	alphaConversion(static_cast<Container*>(o), min); break;
			case Object::vector:	alphaConversion<Vector, Vector::iterator>(static_cast<Vector*>(o), min); break;
			case Object::list:	alphaConversion<List, List::iterator>(static_cast<List*>(o), min); break;
			case Object::apply:		alphaConversion(static_cast<Apply*>(o), min); break;
			case Object::variable: {
				Ci *var = static_cast<Ci*>(o);
				int depth = var->depth();
// 				qDebug() << "puuuu" << var->name() << depth << '<' << min << printAll(m_runStack);
				if(depth<min && m_runStackTop+var->depth()<m_runStack.size()) {
					Object* newvalue = variableValue(var);
					if(newvalue) {
						delete var;
						return newvalue->copy();
					}
				}
			}	break;
			default:
				break;
		}
	return o;
}

template <class T, class Tit>
void Analyzer::alphaConversion(T* o, int min)
{
	Q_ASSERT(o);
	Tit it=o->begin(), itEnd=o->end();
	for(; it!=itEnd; ++it) {
		*it = applyAlpha(*it, min);
	}
}

void Analyzer::alphaConversion(Container* o, int min)
{
	Q_ASSERT(o);
	Container::iterator it=o->begin(), itEnd=o->end();
	for(; it!=itEnd; ++it) {
		if((*it)->type()==Object::container && static_cast<Container*>(*it)->containerType()==Container::bvar)
			continue;
		
		*it = applyAlpha(*it, min);
	}
}

void Analyzer::alphaConversion(Apply* o, int min)
{
	Q_ASSERT(o);
	o->ulimit()=applyAlpha(o->ulimit(), min);
	o->dlimit()=applyAlpha(o->dlimit(), min);
	o->domain()=applyAlpha(o->domain(), min);
	
	Apply::iterator it=o->firstValue(), itEnd=o->end();
	for(; it!=itEnd; ++it)
		*it = applyAlpha(*it, min);
}

BuiltinMethods* Analyzer::builtinMethods()
{
	return &m_builtin;
}
