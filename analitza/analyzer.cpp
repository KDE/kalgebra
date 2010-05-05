/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
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

using namespace AnalitzaUtils;
using namespace Analitza;

Analyzer::Analyzer()
	: m_vars(new Variables), m_varsOwned(true), m_hasdeps(true)
{}

Analyzer::Analyzer(Variables* v)
	: m_vars(v), m_varsOwned(false), m_hasdeps(true)
{ Q_ASSERT(v); }

Analyzer::Analyzer(const Analyzer& a)
	: m_exp(a.m_exp), m_err(a.m_err), m_varsOwned(true), m_hasdeps(true)
{
	m_vars = new Variables(*a.m_vars);
	if(m_exp.isCorrect())
		m_hasdeps=m_exp.tree()->decorate(varsScope());
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
		m_hasdeps=m_exp.tree()->decorate(varsScope());
		
		ExpressionTypeChecker check(m_vars);
		m_currentType=check.check(m_exp);
		
		m_err += check.errors();
	}
}

Expression Analyzer::evaluate()
{
	m_err.clear();
	Expression e;
	
	if(m_exp.isCorrect()) {
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
		e.setTree(calc(m_exp.tree()));
	} else {
		m_err << i18n("Must specify a correct operation");
		
		if(m_exp.isCorrect() && m_hasdeps)
			m_err << i18n("Unknown identifier: '%1'",
							dependencies(m_exp.tree(), varsScope().keys()).join(
								i18nc("identifier separator in error message", "', '")));
	}
	return e;
}

Expression Analyzer::calculateLambda()
{
	Expression e;
	
	if(!m_hasdeps && m_exp.isCorrect()) {
		Q_ASSERT(m_exp.tree()->isContainer());
		Container* math=(Container*) m_exp.tree();
		Q_ASSERT(math->m_params.first()->isContainer());
		if(math->containerType()==Container::math) {
			math=(Container*) math->m_params.first();
			Q_ASSERT(math->m_params.first()->isContainer());
		}
		
		Container* lambda=(Container*) math;
		Q_ASSERT(lambda->containerType()==Container::lambda);
		
		e.setTree(calc(lambda->m_params.last()));
	} else {
		m_err << i18n("Must specify a correct operation");
		
		if(m_exp.isCorrect() && m_hasdeps)
			m_err << i18n("Unknown identifier: '%1'",
							dependencies(m_exp.tree(), varsScope().keys()).join(
								i18nc("identifier separator in error message", "', '")));
	}
	return e;
}

Object* Analyzer::eval(const Object* branch, bool resolve, const QSet<QString>& unscoped)
{
	Q_ASSERT(branch && branch->type()!=Object::none);
	Object *ret=0;
	
	//Won't calc() so would be a good idea to have it simplified
	if(branch->isContainer()) {
		const Container* c = (Container*) branch;
		
// 		Q_ASSERT(!c->isEmpty());
		switch(c->containerType()) {
			case Container::declare: {
				Ci *var = (Ci*) c->m_params[0];
				ret = eval(c->m_params[1], true, unscoped);
				ret = simp(ret);
				insertVariable(var->name(), ret);
			}	break;
			case Container::piecewise: {
				Container::const_iterator it=c->m_params.constBegin(), itEnd=c->constEnd();
				
				bool boundeddep=false;
				for(; !ret && it!=itEnd; ++it) {
					Container *p=static_cast<Container*>(*it);
					Q_ASSERT( (*it)->isContainer() &&
						(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
					bool isPiece = p->containerType()==Container::piece;
					if(isPiece) {
						Object *cond=simp(eval(p->m_params[1], resolve, unscoped));
						boundeddep=hasTheVar(unscoped, cond);
						if(cond->type()==Object::value) {
							Cn* cval=static_cast<Cn*>(cond);
							if(cval->isTrue()) {
								ret=eval(p->m_params[0], resolve, unscoped);
							}
						}
						delete cond;
					} else { //FIXME: Maybe should look for more pieces?
						if(!boundeddep)
							ret=eval(p->m_params[0], resolve, unscoped);
					}
				}
				
				if(!ret)
					ret=c->copy();
			} 	break;
			case Container::lambda: {
				QSet<QString> newUnscoped(unscoped);
				newUnscoped+=c->bvarStrings().toSet();
				
				Container *r = c->copy();
				Object* old=r->m_params.last();
				r->m_params.last()=eval(old, false, newUnscoped);
				delete old;
				
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
		//FIXME: Should check if it is that crappy 
		Ci* var=(Ci*) branch;
		 
		Object* val=var->isDefined() ? var->value() : 0;
		if(val && !unscoped.contains(var->name()) && !equalTree(var, val))
			ret = eval(val, resolve, unscoped);
		
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
				
			}	break;
			case Operator::function: {
				//it is a function. I'll take only this case for the moment
				//it is only meant for operations with scoped variables that _change_ its value => have a value
				Object* body=simp(eval(c->m_params[0], true, unscoped));
				
				if(resolve && body && body->isContainer()) {
					const Container *cbody = (Container*) body;
					
					if(cbody->m_params.size()==c->m_params.size()) {
						QStringList bvars=cbody->bvarStrings();
						
						int i=0;
						Object::ScopeInformation scope;
						QVector<Object*> values(bvars.size());
						
						foreach(const QString& bvar, bvars) {
							values[i]=simp(eval(c->m_params[i+1], resolve, unscoped));
							scope.insert(bvar, &values[i]);
							++i;
						}
						
						cbody->m_params.last()->decorate(scope);
						ret=eval(cbody->m_params.last(), resolve, unscoped);
						
						qDeleteAll(values);
					}
				}
				
				if(!ret)
					ret=c->copy();
				
				delete body;
			}	break;
			default: {
				Apply *r = c->copy();
				
				QSet<QString> newUnscoped(unscoped);
				if(op.isBounded()) {
					if(r->domain()) {
						QScopedPointer<Object> o(r->domain());
						r->domain()=eval(r->domain(), resolve, unscoped);
					}
					if(r->dlimit()) {
						QScopedPointer<Object> o(r->dlimit());
						r->dlimit()=eval(r->dlimit(), resolve, unscoped);
					}
					if(r->ulimit()) {
						QScopedPointer<Object> o(r->ulimit());
						r->ulimit()=eval(r->ulimit(), resolve, unscoped);
					}
					
					newUnscoped+=c->bvarStrings().toSet();
					
				}
				
				Container::iterator it=r->firstValue(), itEnd=r->m_params.end();
				for(; it!=itEnd; ++it) {
					Object *o=*it;
					*it= eval(*it, resolve, newUnscoped);
					delete o;
				}
				
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
	Object *ret=0;
	const Ci* v;
	
	if(o->type()!=Object::oper && !hasVars(o, var))
		ret = new Cn(0.);
	else switch(o->type()) {
		case Object::container:
			ret = derivative(var, (const Container*) o);
			break;
		case Object::apply:
			ret = derivative(var, (const Apply*) o);
			break;
		case Object::variable:
			v = (Ci*) o;
			if(v->name()==var)
				ret = new Cn(1.);
			break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			Vector *c1=new Vector(v->size());
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				c1->appendBranch(derivative(var, *it));
			}
			ret=c1;
		}	break;
		case Object::list: {
			List *v=(List*) o;
			List *c1=new List;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				c1->appendBranch(derivative(var, *it));
			}
			ret=c1;
		}	break;
		case Object::value:
		case Object::oper:
		case Object::none:
			break;
	}
	return ret;
}

Object* Analyzer::derivative(const QString &var, const Apply* c)
{
	Operator op = c->firstOperator();
	switch(op.operatorType()) {
		case Operator::minus:
		case Operator::plus: {
			Apply *r= new Apply;
			r->appendBranch(new Operator(op));
			
			Container::const_iterator it(c->firstValue());
			for(; it!=c->m_params.end(); ++it)
				r->appendBranch(derivative(var, *it));
			return r;
		} break;
		case Operator::times: {
			Apply *nx = new Apply;
			nx->appendBranch(new Operator(Operator::plus));
			
			Apply::const_iterator it(c->firstValue());
			for(; it!=c->m_params.end(); ++it) {
				Apply *neach = new Apply;
				neach->appendBranch(new Operator(Operator::times));
				
				Apply::const_iterator iobj(c->firstValue());
				for(; iobj!=c->m_params.end(); ++iobj) {
					Object* o;
					if(iobj==it)
						o=derivative(var, *iobj);
					else
						o=(*iobj)->copy();
					
					neach->appendBranch(o);
				}
				nx->appendBranch(neach);
			}
			return nx;
		} break;
		case Operator::power: {
			if(hasVars(c->m_params[1], var)) {
				//http://en.wikipedia.org/wiki/Table_of_derivatives
				//else [if f(x)**g(x)] -> (e**(g(x)*ln f(x)))'
				Apply *nc = new Apply;
				nc->appendBranch(new Operator(Operator::times));
				nc->appendBranch(c->copy());
				Apply *nAss = new Apply;
				nAss->appendBranch(new Operator(Operator::plus));
				nc->appendBranch(nAss);
				
				Apply *nChain1 = new Apply;
				nChain1->appendBranch(new Operator(Operator::times));
				nChain1->appendBranch(derivative(var, *c->firstValue()));
				
				Apply *cDiv = new Apply;
				cDiv->appendBranch(new Operator(Operator::divide));
				cDiv->appendBranch((*(c->firstValue()+1))->copy());
				cDiv->appendBranch((*c->firstValue())->copy());
				nChain1->appendBranch(cDiv);
				
				Apply *nChain2 = new Apply;
				nChain2->appendBranch(new Operator(Operator::times));
				nChain2->appendBranch(derivative(var, *(c->firstValue()+1)));
				
				Apply *cLog = new Apply;
				cLog->appendBranch(new Operator(Operator::ln));
				cLog->appendBranch((*c->firstValue())->copy());
				nChain2->appendBranch(cLog);
				
				nAss->appendBranch(nChain1);
				nAss->appendBranch(nChain2);
				
				return nc;
			} else {
				Apply *cx = new Apply;
				cx->appendBranch(new Operator(Operator::times));
				cx->appendBranch(c->m_params[1]->copy());
				cx->appendBranch(derivative(var, *c->firstValue()));
				
				Apply* nc= new Apply;
				nc->appendBranch(new Operator(Operator::power));
				nc->appendBranch(c->m_params[0]->copy());
				cx->appendBranch(nc);
				
				Apply *degree = new Apply;
				degree->appendBranch(new Operator(Operator::minus));
				degree->appendBranch(c->m_params[1]->copy());
				degree->appendBranch(new Cn(1.));
				nc->appendBranch(degree);
				
				return cx;
			}
		} break;
		case Operator::sin: {
			Apply *ncChain = new Apply;
			ncChain->appendBranch(new Operator(Operator::times));
			ncChain->appendBranch(derivative(var, *c->firstValue()));
			Apply *nc = new Apply;
			nc->appendBranch(new Operator(Operator::cos));
			nc->appendBranch((*c->firstValue())->copy());
			ncChain->appendBranch(nc);
			return ncChain;
		} break;
		case Operator::cos: {
			Apply *ncChain = new Apply;
			ncChain->appendBranch(new Operator(Operator::times));
			ncChain->appendBranch(derivative(var, *c->firstValue()));
			
			Apply *nc = new Apply;
			nc->appendBranch(new Operator(Operator::sin));
			nc->appendBranch((*c->firstValue())->copy());
			Apply *negation = new Apply;
			negation->appendBranch(new Operator(Operator::minus));
			negation->appendBranch(nc);
			ncChain->appendBranch(negation);
			return ncChain;
		} break;
		case Operator::tan: {
			Apply *ncChain = new Apply;
			ncChain->appendBranch(new Operator(Operator::divide));
			ncChain->appendBranch(derivative(var, *c->firstValue()));
			
			Apply *nc = new Apply;
			nc->appendBranch(new Operator(Operator::power));
			
			Apply *lilcosine = new Apply;
			lilcosine->appendBranch(new Operator(Operator::cos));
			lilcosine->appendBranch((*c->firstValue())->copy());
			nc->appendBranch(lilcosine);
			nc->appendBranch(new Cn(2.));
			ncChain->appendBranch(nc);
			return ncChain;
		} break;
		case Operator::divide: {
			Object *f, *g; //referring to f/g
			f=*c->firstValue();
			g=*(c->firstValue()+1);
			
			Apply *nc = new Apply;
			nc->appendBranch(new Operator(Operator::divide));
			
			Apply *cmin = new Apply;
			cmin->appendBranch(new Operator(Operator::minus));
			
			Apply *cmin1 =new Apply;
			cmin1->appendBranch(new Operator(Operator::times));
			cmin1->appendBranch(derivative(var, f));
			cmin1->appendBranch(g->copy());
			cmin->appendBranch(cmin1);
			nc->appendBranch(cmin);
			
			Apply *cmin2 =new Apply;
			cmin2->appendBranch(new Operator(Operator::times));
			cmin2->appendBranch(f->copy());
			cmin2->appendBranch(derivative(var, g));
			cmin->appendBranch(cmin2);
			
			Apply *cquad = new Apply;
			cquad->appendBranch(new Operator(Operator::power));
			cquad->appendBranch(g->copy());
			cquad->appendBranch(new Cn(2.));
			nc->appendBranch(cquad);
			
// 			qDebug() << "iei!" << cmin->toString();
			return nc;
		} break;
		case Operator::ln: {
			Apply *nc = new Apply;
			nc->appendBranch(new Operator(Operator::divide));
			nc->appendBranch(derivative(var, *c->firstValue()));
			nc->appendBranch((*c->firstValue())->copy());
			return nc;
		} break;
		default: {
			m_err.append(i18n("The %1 derivative has not been implemented.", op.toString()));
			Apply* obj = new Apply;
			obj->appendBranch(new Operator(Operator::diff));
			obj->appendBranch(c->copy());
			return obj;
		}
	}
	return 0;
}

Object* Analyzer::derivative(const QString &var, const Container *c)
{
	if(c->containerType()==Container::lambda) {
		//TODO REVIEW
		return derivative(var, c->m_params.last());
	} else if(c->containerType()==Container::piecewise) {
		Container *newPw = c->copy();
		
		foreach(Object* o, newPw->m_params) {
			Q_ASSERT(o->isContainer());
			Container *p = (Container *) o;
// 			Object *aux=p->m_params[0];
			p->m_params[0]=derivative(var, p->m_params[0]);
// 			delete aux;
		}
		return newPw;
	} else if(c->containerType()==Container::declare) {
		Container* obj = new Container(Container::declare);
		obj->appendBranch(c->m_params.first()->copy());
		obj->appendBranch(derivative(var, c->m_params.last()));
		return obj;
	} else {
		Container *cret = new Container(c->containerType());
		foreach(Object* o, c->m_params)
			cret->appendBranch(derivative(var, o));
		
		return cret;
	}
	return 0;
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
		
	insertVariable(var->name(), ret);
	
	if(!ret || (ret->type()!=Object::vector && ret->type()!=Object::list && ret->type()!=Object::value))
	{
		delete ret;
		
		//Would be nice to return NaN
		ret=new Cn(0.);
	}
	Q_ASSERT(ret);
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
			ret=root->copy();
			break;
		case Object::variable: {
			Ci* a=(Ci*) root;
			ret = calc(a->value());
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
		case Operator::function:
			ret = func(*c);
			break;
		case Operator::none:
			ret=calc(*c->firstValue());
			break;
		case Operator::diff: {
			//TODO: Make multibvar
			QStringList bvars=c->bvarStrings();
			
			//We construct the lambda
			Object* o=derivative(bvars[0], *c->firstValue());
			
			Container* cc=new Container(Container::lambda);
			foreach(const QString& v, bvars) {
				Container* bvar=new Container(Container::bvar);
				bvar->appendBranch(new Ci(v));
				cc->appendBranch(bvar);
			}
			cc->appendBranch(simp(o));
			ret=cc;
			
			ret->decorate(Object::ScopeInformation());
			
		}	break;
		default: {
			int count=c->countValues();
			Q_ASSERT(count>0);
			
			Object* numbers[count];
			Apply::const_iterator it = c->firstValue(), itEnd=c->constEnd();
			for(int i=0; it!=itEnd; ++it, ++i)
				numbers[i]=calc(*it);
			
			Q_ASSERT(	(op.nparams()< 0 && count>1) ||
						(op.nparams()>-1 && count==op.nparams()) ||
						opt==Operator::minus);
			
			ret = numbers[0];
			
			QString correct;
			if(count>=2) {
				Object** it=&numbers[1], **itEnd=&numbers[count];
				
				for(; it!=itEnd; ++it) {
					ret=Operations::reduce(opt, ret, *it, correct);
					
					if(KDE_ISUNLIKELY(!correct.isEmpty())) {
						m_err.append(correct);
						correct.clear();
						break;
					}
				}
			} else {
				ret=Operations::reduceUnary(opt, ret, correct);
				if(KDE_ISUNLIKELY(!correct.isEmpty()))
					m_err.append(correct);
			}
		}	break;
	}
	Q_ASSERT(ret);
	return ret;
}

Object* Analyzer::operate(const Container* c)
{
	Q_ASSERT(c);
	Q_ASSERT(!c->isEmpty());
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
		case Container::lambda:
			ret=c->copy();
			break;
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
			TypeBoundingIterator(const QVector<Ci*>& vars, T* l)
				: m_vars(vars), iterators(vars.size()), list(l)
				, itBegin(l->constBegin()), itEnd(l->constEnd())
			{
				int i=0;
				foreach(Ci* v, vars) {
					v->value()=*itBegin;
					iterators[i++]=itBegin;
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
					m_vars[i]->value()=*iterators[i];
				}
				
				return !cont;
			}
		private:
			QVector<Ci*> m_vars;
			QVector<Iterator> iterators;
			T* list;
			const Iterator itBegin, itEnd;
	};
	
	class RangeBoundingIterator : public BoundingIterator
	{
		public:
			RangeBoundingIterator(const QVector<Cn*>& values, double dl, double ul, double step)
				: values(values), dl(dl), ul(ul), step(step)
			{}
			
			~RangeBoundingIterator()
			{
				qDeleteAll(values);
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
	};
}

BoundingIterator* Analyzer::initializeBVars(const Apply* n)
{
	BoundingIterator* ret=0;
	QList<Ci*> bvars=n->bvarCi();
	
	Object* domain=n->domain();
	
	if(domain)
	{
		domain=calc(domain);
		
		if(isCorrect()) {
			switch(domain->type()) {
				case Object::list:
					ret=new TypeBoundingIterator<List, List::const_iterator>(bvars.toVector(), static_cast<List*>(domain));
					break;
				case Object::vector:
					ret=new TypeBoundingIterator<Vector, Vector::const_iterator>(bvars.toVector(), static_cast<Vector*>(domain));
					break;
				default:
					m_err.append(i18n("Type not supported for bounding."));
			}
		} else
			m_err.append(i18n("Incorrect domain."));
	}
	else
	{
		Object *objul=calc(n->ulimit());
		Object *objdl=calc(n->dlimit());
		
		if(isCorrect() && objul->type()==Object::value && objdl->type()==Object::value) {
			Cn *u=static_cast<Cn*>(objul);
			Cn *d=static_cast<Cn*>(objdl);
			double ul=u->value();
			double dl=d->value();
			
			if(dl<ul) {
				QVector<Cn*> rr(bvars.size());
				int i=0;
				
				foreach(Ci* bvar, bvars) {
					rr[i]=new Cn(dl);
					bvar->value()=rr[i];
					i++;
				}
				
				ret=new RangeBoundingIterator(rr, dl, ul, 1);
			} else
				m_err.append(i18n("The downlimit is greater than the uplimit"));
		} else
			m_err.append(i18n("Incorrect uplimit or downlimit."));
		
		delete objul;
		delete objdl;
		
	}
	return ret;
}

Object* Analyzer::boundedOperation(const Apply& n, const Operator& t, Object* initial)
{
	Object* ret=initial;
	BoundingIterator* it=initializeBVars(&n);
	if(it)
	{
		QString correct;
		Operator::OperatorType type=t.operatorType();
		do {
			Object *val=calc(n.m_params.last());
			
			ret=Operations::reduce(type, ret, val, correct);
		} while(it->hasNext());
		
		foreach(Ci* var, n.bvarCi())
			var->value()=0;
	}
	delete it;
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

Object* Analyzer::func(const Apply& n)
{
	Container *function = (Container*) calc(n.m_params[0]);
	
	function->decorate(Object::ScopeInformation());
	QList<Ci*> vars = function->bvarCi();
	
	int i=0;
	
	foreach(Ci* param, vars) {
		Object* val=calc(n.m_params[++i]);
		param->value()=val;
	}
	
	Object* ret=calc(function->m_params.last());
	
	foreach(Ci* param, vars) {
		delete param->value();
	}
	delete function;
	
	return ret;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void Analyzer::simplify()
{
	if(m_exp.isCorrect()) {
		Object* o=simp(m_exp.tree());
		m_exp.setTree(o);
		setExpression(m_exp);
	}
}

void Analyzer::levelOut(Apply *c, Apply *ob, Apply::iterator &pos)
{
	Container::iterator it = ob->firstValue();
	for(; it!=ob->m_params.end(); pos++) {
		pos=c->m_params.insert(pos, *it);
		
		it=ob->m_params.erase(it);
	}
}

Object* Analyzer::simp(Object* root)
{
	Q_ASSERT(root && root->type()!=Object::none);
	
	if(!isLambda(root) && !hasVars(root))
	{
		if(root->type()!=Object::value && root->type() !=Object::oper) {
			bool deps=root->decorate(Object::ScopeInformation());
			Q_ASSERT(!deps);
			
			Object* aux=root;
			root = calc(root);
			delete aux;
			
			if(isLambda(root))
				root = simp(root);
		}
	} else if(root->type()==Object::vector) {
		Vector* v=static_cast<Vector*>(root);
		Vector::iterator it = v->begin(), itEnd=v->end();
		
		for(; it!=itEnd; ++it)
			*it = simp(*it);
	} else if(root->type()==Object::list) {
		List* v=static_cast<List*>(root);
		List::iterator it = v->begin(), itEnd=v->end();
		
		for(; it!=itEnd; ++it)
			*it = simp(*it);
	} else if(root->type()==Object::apply) {
		root = simpApply((Apply*) root);
	} else if(root->isContainer()) {
		Container *c= (Container*) root;
		
		switch(c->containerType()) {
			case Container::piecewise:
				root=simpPiecewise(c);
				break;
			case Container::lambda:
				c->m_params.last()=simp(c->m_params.last());
				break;
			default: {
				Container::iterator it = c->begin();
				
				for(; it!=c->m_params.end(); ++it)
					*it = simp(*it);
			}	break;
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
			for(it=c->firstValue(); c->m_params.count()>1 && it!=c->m_params.end();) {
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
			it=c->m_params.end()-1;
			Object* first=*c->firstValue();
			
			bool done=false;
			QString delme=c->toString();
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
			for(it = c->firstValue(); it!=c->m_params.end(); ++it)
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
			for(it = c->firstValue(); it!=c->m_params.end(); ++it)
				*it = simp(*it);
			
			Object *f, *g; //f/g
			f=*c->firstValue();
			g=*(c->firstValue()+1);
			
			if(equalTree(f, g)) {
				delete root;
				root = new Cn(1.);
				break;
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
			Object *uplimit=c->ulimit(), *downlimit=c->dlimit(), *domain=c->domain();
			if(uplimit) uplimit=simp(uplimit);
			if(downlimit) downlimit=simp(downlimit);
			if(domain) domain=simp(domain);
			
			//TODO: simplify this code
			for(it = c->m_params.begin(); it!=c->m_params.end(); ++it)
				*it = simp(*it);
			
			//if bvars is empty, we are dealing with an invalid sum()
			Object* function = *c->firstValue();
			if(!bvars.isEmpty() && !domain && !hasTheVar(bvars.toSet(), function)) {
				Apply *cDiff=new Apply;
				cDiff->appendBranch(new Operator(Operator::minus));
				cDiff->appendBranch(uplimit  ->copy());
				cDiff->appendBranch(downlimit->copy());
				
				Apply *nc=new Apply;
				nc->appendBranch(new Operator(Operator::times));
				nc->appendBranch(cDiff);
				nc->appendBranch(function);
				
				c->m_params.last()=0;
				delete c;
				root=simp(nc);
			} else if(function->isApply())
				root=simpSum(c);
		}	break;
		case Operator::card: {
			Object* val=simp(*c->firstValue());
			if(val->type()==Object::vector)
			{
				c->m_params.last()=0;
				QString correct;
				val=Operations::reduceUnary(Operator::card, val, correct);
				//TODO: if(!correct) Handle me!
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
				QString err;
				Object* ret=Operations::reduce(Operator::selector, idx->copy(), value->copy(), err);
				
				if(ret) {
					root=ret;
					c->m_params[0]=0;
					c->m_params[1]=0;
					
					delete c;
				}
			}
		}	break;
		case Operator::_union: {
			Apply::iterator it=c->firstValue(), itEnd=c->m_params.end();
			
			QList<Object*> newParams;
			for(; it!=itEnd; ++it) {
				*it=simp(*it);
				
				if(newParams.isEmpty())
					newParams.append(*it);
				else {
					QString err;
					Object* ret=Operations::reduce(Operator::_union, newParams.last(), *it, err);
					if(ret) {
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
		case Operator::function: {
			it=c->m_params.begin();
			Object* function=*it; ++it;
			
			bool allequal=false;
			Container* cfunc=0;
			QList<Ci*> bvars;
			if(function->isContainer()) {
				cfunc=(Container*) function;
				allequal=true;
				Q_ASSERT(cfunc->containerType()==Container::lambda);
				
				cfunc=(Container*) function;
				bvars=cfunc->bvarCi();
			}
			
			for(int i=0; it!=c->m_params.end(); ++it, ++i) {
				*it = simp(*it);
				allequal = allequal && equalTree(*it, bvars[i]);
			}
			
			if(allequal) { //x->x = x
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
			
			for(it = c->firstValue(); it!=c->m_params.end(); ++it)
				*it = simp(*it);
			break;
	}
	
	return root;
}

Object* Analyzer::simpScalar(Apply * c)
{
	Object *value=0;
	Container::iterator i = c->firstValue();
	Operator o = c->firstOperator();
	bool firstvalue = i!=c->m_params.end() && ((*i)->type()==Object::value || ((*i)->type()==Object::vector && !hasVars(*i)));
	for(; i!=c->m_params.end();) {
		bool d=false;
		
		//TODO: hasVars needed? should have already been simplifyed before, just check type==cn
		if((*i)->type()==Object::value || ((*i)->type()==Object::vector && !hasVars(*i))) {
			Object* aux = *i;
			
			if(value) {
				QString correct;
				value=Operations::reduce(o.operatorType(), value, aux, correct);
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
Object* createMono(const Operator& o, const QPair<double, Object*>& p)
{
	Object* toAdd=0;
	if(p.first==0.) {
		delete p.second;
	} else if(p.first==1.) {
		toAdd=p.second;
	} else if(p.first==-1. && (o==Operator::plus || o==Operator::minus)) {
		Apply *cint = new Apply;
		cint->appendBranch(new Operator(Operator::minus));
		cint->appendBranch(p.second);
		toAdd=cint;
	} else {
		Apply *cint = new Apply;
		cint->appendBranch(new Operator(o.multiplicityOperator()));
		cint->appendBranch(p.second);
		cint->appendBranch(new Cn(p.first));
		if(o.multiplicityOperator()==Operator::times)
			cint->m_params.swap(0,1);
		toAdd=cint;
	}
	return toAdd;
}
}

Object* Analyzer::simpPolynomials(Apply* c)
{
	Q_ASSERT(c!=0 && dynamic_cast<Apply*>(c));
	
	QList<QPair<double, Object*> > monos;
	Operator o(c->firstOperator());
	bool sign=true, first=true;
	Apply::const_iterator it(c->firstValue());
	
	for(; it!=c->constEnd(); ++it) {
		Object *o2=*it;
		QPair<double, Object*> imono;
		bool ismono=false;
		
		if(o2->isApply()) {
			Apply *cx = (Apply*) o2;
			if(cx->firstOperator()==o.multiplicityOperator() && cx->m_params.count()==2) {
				bool valid=false;
				int scalar, var;
				
				if(cx->m_params[0]->type()==Object::value) {
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
			} else if(cx->firstOperator()==Operator::minus && cx->isUnary()) {
				if(o==Operator::plus) {
					//detecting -x as QPair<-1, o>
					imono.first = -1.;
					imono.second = *cx->firstValue();
					ismono=true;
				} else if(o==Operator::times) {
					imono.first = 1.;
					imono.second = *cx->firstValue();
					ismono=true;
					sign = !sign;
				}
			}
		}
		
		if(!ismono) {
			imono.first = 1.;
			imono.second = o2;
		}
		
		if(o!=Operator::times && imono.second->isApply()) {
			Apply *m = (Apply*) imono.second;
			if(m->firstOperator()==Operator::minus && m->isUnary()) {
				imono.second = *m->firstValue();
				imono.first *= -1.;
			}
		}
		
		if(o==Operator::minus && !first) {
			imono.first*=-1;
		}
		
		bool found = false;
		QList<QPair<double, Object*> >::iterator it1(monos.begin());
		for(; it1!=monos.end(); ++it1) {
			Object *o1=it1->second, *o2=imono.second;
			if(o2->type()!=Object::oper && equalTree(o1, o2)) {
				found = true;
				break;
			}
		}
		
// 		qDebug() << "->" << c->toString() << c->firstOperator().toString() << found;
		if(found) {
			bool prevPositive=it1->first>0.;
			it1->first += imono.first;
			bool postPositive=it1->first>0.;
			
// 			qDebug() << "getting near:::" << it1->first << prevPositive << postPositive << o.toString() << foundAtFirst;
			if(it1->first==0.) {
				delete it1->second;
				monos.erase(it1);
			} else if(o==Operator::minus) {
				if(prevPositive != postPositive) {
					sign = !sign;
				}
			}
		} else {
			imono.second = imono.second->copy();
			monos.append(imono);
		}
		first=false;
	}
	
	delete c;
	c=0;
	
	//We delete the empty monomials. Should merge both loops
	QList<QPair<double, Object*> >::iterator i=monos.begin();
	for(; i!=monos.end(); ) {
		//FIXME: REVIEW This loop should not be needed anymore
		if(i->first==0.) {
			delete i->second;
			i=monos.erase(i);
		} else
			++i;
	}
	
	Object *root=0;
	if(monos.count()==1) {
		root=createMono(o, monos.first());
	} else if(monos.count()>1) {
		c= new Apply;
		c->appendBranch(new Operator(o));
		
		QList<QPair<double, Object*> >::iterator i=monos.begin();
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
	const Container *otherwise=0;
	Container::const_iterator it=c->m_params.constBegin(), itEnd=c->constEnd();
	QList<Object*> newList;
	
	for(; /*!stop &&*/ it!=itEnd; ++it) {
		Container *p=static_cast<Container*>(*it);
		Q_ASSERT( (*it)->isContainer() &&
				(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
		bool isPiece = p->containerType()==Container::piece;
		
		if(isPiece) {
			p->m_params[1]=simp(p->m_params[1]);
			if(p->m_params[1]->type()==Object::value) {
				Cn* cond=static_cast<Cn*>(p->m_params[1]);
				if(cond->isTrue()) {
					delete p->m_params[1];
					p->m_params.removeAt(1);
					p->setContainerType(Container::otherwise);
					isPiece=false;
// 					stop=true;
				} else {
					delete p;
				}
			} else {
						//TODO: It would be nice if we could simplify:
						// if(x=n) simplify x as n
				p->m_params[0]=simp(p->m_params[0]);
				newList.append(p);
			}
					
		} else {
					//it is an otherwise
			if(otherwise) {
				delete p;
			} else {
				p->m_params[0] = simp(p->m_params[0]);
				otherwise=p;
				newList.append(p);
			}
		}
	}
	c->m_params = newList;
			
	if(c->m_params.count()==1 && otherwise) {
		root=otherwise->m_params[0];
		c->m_params[0]=0;
		delete c;
	}
	return root;
}

Object::ScopeInformation Analyzer::varsScope() const
{
	return AnalitzaUtils::variablesScope(m_vars);
}

Expression Analyzer::derivative()
{
	m_err.clear();
	//TODO: Must support multiple bvars
	//TODO: return new expression
	Expression exp;
	if(m_exp.isCorrect()) {
		QStringList vars;
		Object* deriv=m_exp.tree();
		if(m_exp.isLambda()){
			Q_ASSERT(deriv->isContainer());
			Container* lambda=(Container*) deriv;
			if(lambda->containerType()==Container::math) {
				Q_ASSERT(lambda->m_params.first()->isContainer());
				lambda = (Container*) lambda->m_params.first();
			}
			vars=lambda->bvarStrings();
			Q_ASSERT(lambda->containerType()==Container::lambda);
			deriv=lambda->m_params.last();
		} else
			vars+="x";
		
		Object* o = derivative(vars.first(), deriv);
// 		qDebug() << "SSS" << o->toString();
		o=simp(o);
		Container* lambda=new Container(Container::lambda);
		foreach(const QString& dep, vars) {
			Container* bvar=new Container(Container::bvar);
			bvar->appendBranch(new Ci(dep));
			lambda->appendBranch(bvar);
		}
		lambda->appendBranch(o);
		
		exp.setTree(lambda);
	}
	return exp;
}

Expression Analyzer::dependenciesToLambda() const
{
	if(m_hasdeps) {
		QStringList deps=dependencies(m_exp.tree(), varsScope().keys());
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
	bool islambda=false;
	if(value->isContainer()) {
		const Container *c=static_cast<const Container*>(value);
		islambda= c->containerType()==Container::lambda;
	}
	
	bool wrong=!islambda && hasVars(value, name, QStringList(), m_vars);
	if(wrong)
		m_err << i18nc("By a cycle i mean a variable that depends on itself", "Defined a variable cycle");
	else
		m_vars->modify(name, value);
	
	return !wrong;
}

Cn* Analyzer::insertValueVariable(const QString& name, double value)
{
	Cn* val=m_vars->modify(name, value);
	m_hasdeps=m_exp.tree()->decorate(varsScope());
	return val;
}

typedef QPair<QString, double> StringDoublePair;
double Analyzer::derivative(const QList<StringDoublePair>& values )
{
	//c++ numerical recipes p. 192. Only for f'
	//Image
	//TODO: Should adapt to insertValueVariable
	
	Q_ASSERT(m_exp.isCorrect());
	
	Object::ScopeInformation scope=varsScope();
	QVector<Object*> deps(values.size());
	
	int i=0;
	foreach(const StringDoublePair& valp, values) {//TODO: it should be +-hh
		deps[i]=new Cn(valp.second);
		scope.insert(valp.first, &deps[i]);
		
		i++;
	}
	bool hasdeps=m_exp.tree()->decorate(scope);
	if(hasdeps)
		return 0.;
	
	Expression e1(calc(m_exp.tree()));
	if(!isCorrect())
		return 0.;
	
	//Image+h
	double h=0.0000000001;
	i=0;
	foreach(const StringDoublePair& valp, values) {
// 		volatile double temp=valp.second+h;
// 		double hh=temp-valp.second;
		((Cn*) deps[i])->setValue(valp.second+h);
		
		i++;
	}
	
	Expression e2(calc(m_exp.tree()));
	if(!isCorrect())
		return 0.;
	
	qDeleteAll(deps);
	
	if(!e1.isReal() || !e2.isReal()) {
		m_err << i18n("The result is not a number");
		return 0;
	}
	
	return (e2.toReal().value()-e1.toReal().value())/h;
}
