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

#include "analitza.h"
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

Analitza::Analitza() : m_vars(new Variables), m_varsOwned(true) { }

Analitza::Analitza(Variables* v) : m_vars(v), m_varsOwned(false) { Q_ASSERT(v); }

Analitza::Analitza(const Analitza& a) : m_exp(a.m_exp), m_err(a.m_err), m_varsOwned(true)
{
	m_vars = new Variables(*a.m_vars);
}

Analitza::~Analitza()
{
	if(m_varsOwned)
		delete m_vars;
}


void Analitza::setExpression(const Expression & e)
{
	m_exp=e;
	flushErrors();
}

Expression Analitza::evaluate()
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

Expression Analitza::calculate()
{
	Expression e;
	if(m_exp.isCorrect()) {
		bool hasdeps=m_exp.tree()->decorate(varsScope());
		
		if(!hasdeps)
			e.setTree(calc(m_exp.tree()));
		else
			m_err << i18n("Unfulfilled dependencies for: '%1'",
							dependencies(m_exp.tree(), varsScope().keys()).join(
								i18nc("identifier separator in error message", "', '")));
	} else {
		m_err << i18n("Must specify a correct operation");
	}
	return e;
}

Object* Analitza::eval(const Object* branch, bool resolve, const QSet<QString>& unscoped)
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
				Container::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
				
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
			case Container::apply: {
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
							QStringList bvars=cbody->bvarStrings();
							
							int i=1;
							foreach(const QString& bvar, bvars) {
								Object *val = simp(eval(c->m_params[i], resolve, unscoped));
								
								m_vars->stack(bvar, val);
								delete val;
								++i;
							}
							
							ret=eval(cbody->m_params.last(), resolve, unscoped);
							
							foreach(const QString & bvar, bvars)
								m_vars->destroy(bvar);
						} else {
							ret=c->copy();
						}
						
						delete body;
					}	break;
					default: {
						QSet<QString> newUnscoped(unscoped);
						if(op.isBounded()) {
							newUnscoped+=c->bvarStrings().toSet();
						}
						Container *r = c->copy();
						Container::iterator it(r->firstValue());
						for(; it!=r->m_params.end(); ++it) {
							Object *o=*it;
							*it= eval(*it, resolve, newUnscoped);
							delete o;
						}
						
						ret=r;
					}	break;
				}
			}	break;
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
		
		if(m_vars->contains(var->name())) {
			Object* val=m_vars->value(var->name());
			if(val && !unscoped.contains(var->name()) && !Container::equalTree(var, val))
				ret = eval(val, resolve, unscoped);
// 			else
// 				ret=Expression::objectCopy(val);
		}
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
	}
	
	if(!ret)
		ret=branch->copy();
	Q_ASSERT(ret);
	return ret;
}

Object* Analitza::derivative(const QString &var, const Object* o)
{
	Q_ASSERT(o);
	Object *ret=0;
	const Container *c;
	const Ci* v;
	if(o->type()!=Object::oper && !hasVars(o, var))
		ret = new Cn(0.);
	else switch(o->type()) {
		case Object::container:
			c=(Container*) o;
			ret = derivative(var, c);
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
				c1->appendBranch(calc(*it));
			}
			ret=c1;
		}	break;
		case Object::list: {
			List *v=(List*) o;
			List *c1=new List;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				c1->appendBranch(calc(*it));
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

Object* Analitza::derivative(const QString &var, const Container *c)
{
	if(c->containerType()==Container::apply) {
		Operator op = c->firstOperator();
		switch(op.operatorType()) {
			case Operator::minus:
			case Operator::plus: {
				Container *r= new Container(Container::apply);
				r->appendBranch(new Operator(op));
				
				Container::const_iterator it(c->firstValue());
				for(; it!=c->m_params.end(); ++it)
					r->appendBranch(derivative(var, *it));
				
				return r;
			} break;
			case Operator::times: {
				Container *nx = new Container(Container::apply);
				nx->appendBranch(new Operator(Operator::plus));
				
				Container::const_iterator it(c->firstValue());
				for(; it!=c->m_params.end(); ++it) {
					Container *neach = new Container(Container::apply);
					neach->appendBranch(new Operator(Operator::times));
					
					Container::const_iterator iobj(c->firstValue());
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
				if(hasVars(c->m_params[2], var)) {
					//http://en.wikipedia.org/wiki/Table_of_derivatives
					//else [if f(x)**g(x)] -> (e**(g(x)*ln f(x)))'
					Container *nc = new Container(Container::apply);
					nc->appendBranch(new Operator(Operator::times));
					nc->appendBranch(c->copy());
					Container *nAss = new Container(Container::apply);
					nAss->appendBranch(new Operator(Operator::plus));
					nc->appendBranch(nAss);
					
					Container *nChain1 = new Container(Container::apply);
					nChain1->appendBranch(new Operator(Operator::times));
					nChain1->appendBranch(derivative(var, *c->firstValue()));
					
					Container *cDiv = new Container(Container::apply);
					cDiv->appendBranch(new Operator(Operator::divide));
					cDiv->appendBranch((*(c->firstValue()+1))->copy());
					cDiv->appendBranch((*c->firstValue())->copy());
					nChain1->appendBranch(cDiv);
					
					Container *nChain2 = new Container(Container::apply);
					nChain2->appendBranch(new Operator(Operator::times));
					nChain2->appendBranch(derivative(var, *(c->firstValue()+1)));
					
					Container *cLog = new Container(Container::apply);
					cLog->appendBranch(new Operator(Operator::ln));
					cLog->appendBranch((*c->firstValue())->copy());
					nChain2->appendBranch(cLog);
					
					nAss->appendBranch(nChain1);
					nAss->appendBranch(nChain2);
					return nc;
				} else {
					Container *cx = new Container(Container::apply);
					cx->appendBranch(new Operator(Operator::times));
					cx->appendBranch(c->m_params[2]->copy());
					cx->appendBranch(derivative(var, *c->firstValue()));
					
					Container* nc= new Container(Container::apply);
					nc->appendBranch(c->m_params[0]->copy());
					nc->appendBranch(c->m_params[1]->copy());
					cx->appendBranch(nc);
					
					Container *degree = new Container(Container::apply);
					degree->appendBranch(new Operator(Operator::minus));
					degree->appendBranch(c->m_params[2]->copy());
					degree->appendBranch(new Cn(1.));
					nc->appendBranch(degree);
					return cx;
				}
			} break;
			case Operator::sin: {
				Container *ncChain = new Container(Container::apply);
				ncChain->appendBranch(new Operator(Operator::times));
				ncChain->appendBranch(derivative(var, *c->firstValue()));
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::cos));
				nc->appendBranch((*c->firstValue())->copy());
				ncChain->appendBranch(nc);
				return ncChain;
			} break;
			case Operator::cos: {
				Container *ncChain = new Container(Container::apply);
				ncChain->appendBranch(new Operator(Operator::times));
				ncChain->appendBranch(derivative(var, *c->firstValue()));
				
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::sin));
				nc->appendBranch((*c->firstValue())->copy());
				Container *negation = new Container(Container::apply);
				negation->appendBranch(new Operator(Operator::minus));
				negation->appendBranch(nc);
				ncChain->appendBranch(negation);
				return ncChain;
			} break;
			case Operator::tan: {
				Container *ncChain = new Container(Container::apply);
				ncChain->appendBranch(new Operator(Operator::divide));
				ncChain->appendBranch(derivative(var, *c->firstValue()));
				
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::power));
				
				Container *lilcosine = new Container(Container::apply);
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
				
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::divide));
				
				Container *cmin = new Container(Container::apply);
				cmin->appendBranch(new Operator(Operator::minus));
				
				Container *cmin1 =new Container(Container::apply);
				cmin1->appendBranch(new Operator(Operator::times));
				cmin1->appendBranch(derivative(var, f));
				cmin1->appendBranch(g->copy());
				cmin->appendBranch(cmin1);
				nc->appendBranch(cmin);
				
				Container *cmin2 =new Container(Container::apply);
				cmin2->appendBranch(new Operator(Operator::times));
				cmin2->appendBranch(f->copy());
				cmin2->appendBranch(derivative(var, g));
				cmin->appendBranch(cmin2);
				
				Container *cquad = new Container(Container::apply);
				cquad->appendBranch(new Operator(Operator::power));
				cquad->appendBranch(g->copy());
				cquad->appendBranch(new Cn(2.));
				nc->appendBranch(cquad);
				
	// 			qDebug() << "iei!" << cmin->toString();
				return nc;
			} break;
			case Operator::ln: {
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::divide));
				nc->appendBranch(derivative(var, *c->firstValue()));
				nc->appendBranch((*c->firstValue())->copy());
				return nc;
			} break;
			default: {
				m_err.append(i18n("The %1 derivative has not been implemented.", op.toString()));
				Container* obj = new Container(Container::apply);
				obj->appendBranch(new Operator(Operator::diff));
				obj->appendBranch(c->copy());
				return obj;
			}
		}
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

Object* Analitza::calcPiecewise(const Container* c)
{
	Object* ret=0;
	//Here we have a list of options and finally the otherwise option
	const Container *otherwise=0;
	const Object *r=0;
	foreach(Object *o, c->m_params) {
		Container *p=static_cast<Container*>(o);
		Q_ASSERT( o->isContainer() &&
				(p->containerType()==Container::piece ||
				p->containerType()==Container::otherwise) );
		bool isPiece = p->containerType()==Container::piece;
		if(isPiece) {
			Cn* ret=(Cn*) calc(p->m_params[1]);
			if(ret->type()==Object::value && ret->isTrue()) {
				r=p->m_params[0];
			}
			delete ret;
		} else {
			//it is an otherwise
			Q_ASSERT(!otherwise);
			otherwise=p;
		}
	}
	if(!r) {
		if(otherwise)
			r=otherwise->m_params[0];
		else
			m_err << i18nc("Error message, no proper condition found.", "Could not find a proper choice for a condition statement.");
	}
				
	if(r)
		ret=calc(r);
	else
		ret=new Cn(0.);
	Q_ASSERT(ret);
	return ret;
}

Object* Analitza::calcDeclare(const Container* c)
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

Object* Analitza::calc(const Object* root)
{
	Q_ASSERT(root);
	Object* ret=0;
	
	switch(root->type()) {
		case Object::container:
			ret = operate((Container*) root);
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
	
	return ret;
}

Object* Analitza::operate(const Container* c)
{
	Q_ASSERT(c);
	Q_ASSERT(!c->isEmpty());
	Object* ret=0;
	
	switch(c->containerType()) { //TODO: Diffs should be implemented here.
		case Container::math:
			ret=calc(*c->firstValue());
			break;
		case Container::lambda:
			ret=c->copy();
			break;
		case Container::apply:
		{
			Operator op = c->firstOperator();
			Operator::OperatorType opt=op.operatorType();
			
			if(opt==Operator::sum) 
				ret = sum(*c);
			else if(opt==Operator::product)
				ret = product(*c);
			else if(opt==Operator::diff) {
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
				
			} else if(opt==Operator::function) {
				ret = func(*c);
			} else {
				QList<Object*> numbers;
				Container::const_iterator it = c->firstValue(), itEnd=c->m_params.constEnd();
				for(; it!=itEnd; ++it) {
					numbers.append(calc(*it));
				}
				
				Q_ASSERT(	(op.nparams()<0 && numbers.count()>1) ||
							(op.nparams()>-1 && numbers.count()==op.nparams()) ||
							opt==Operator::minus);
				
				if(!opt) {
					ret = numbers.first();
				} else if(KDE_ISLIKELY(!numbers.isEmpty())) {
					ret = numbers.first();
					
					QString correct;
					if(numbers.count()>=2) {
						Container::const_iterator it=numbers.constBegin()+1;
						Container::const_iterator itEnd=numbers.constEnd();
						
						for(; it!=itEnd; ++it) {
							ret=Operations::reduce(opt, ret, *it, correct);
							
							if(KDE_ISUNLIKELY(!ret || !correct.isEmpty())) {
								m_err.append(correct);
								correct.clear();
								if(!ret)
									ret=new Cn(0.);
								break;
							}
						}
					} else {
						ret=Operations::reduceUnary(opt, ret, correct);
						if(KDE_ISUNLIKELY(!correct.isEmpty()))
							m_err.append(correct);
					}
				} else {
					ret = numbers.first();
				}
			}
		}	break;
		case Container::declare:
			ret=calcDeclare(c);
			break;
		case Container::piecewise:
			ret=calcPiecewise(c);
			break;
		case Container::piece:
		case Container::otherwise:
		case Container::bvar:
		case Container::uplimit:
		case Container::downlimit:
		case Container::none:
			Q_ASSERT(false && "tried to calculate a wrong item");
			break;
	}
	Q_ASSERT(ret);
	return ret;
}

Object* Analitza::boundedOperation(const Container& n, const Operator& t, Object* initial)
{
	Object* ret=initial;
	
	Object *objul=calc(n.ulimit());
	Object *objdl=calc(n.dlimit());
	double ul, dl;
	
	bool corr=isCorrect();
	if(corr && objul->type()==Object::value && objdl->type()==Object::value) {
		Cn *u=static_cast<Cn*>(objul);
		Cn *d=static_cast<Cn*>(objdl);
		ul=u->value();
		dl=d->value();
		
		if(dl>ul) {
			corr=false;
			m_err.append(i18n("The downlimit is greater than the uplimit"));
		}
	} else {
		m_err.append(i18n("Incorrect uplimit or downlimit."));
		
		corr=false;
	}
	
	if(!corr)
	{
		delete objul;
		delete objdl;
		return new Cn(0.);
	}
	
	QList<Ci*> bvars=n.bvarCi();
	Q_ASSERT(!bvars.isEmpty());
	Ci* var= bvars.first();
	
	Cn* c = new Cn(0.);
	var->value()=c;
	
	QString correct;
	Operator::OperatorType type=t.operatorType();
	for(double a = dl; a<=ul; a++) {
// 		Q_ASSERT(isCorrect());
// 		qDebug() << "<>" << dl << ul << m_err;
		c->setValue(a);
		Object *val=calc(n.m_params.last());
		ret=Operations::reduce(type, ret, val, correct);
	}
	var->value()=0;
	delete c;
	
	return ret;
}

Object* Analitza::product(const Container& n)
{
	return boundedOperation(n, Operator(Operator::times), new Cn(1.));
}

Object* Analitza::sum(const Container& n)
{
	return boundedOperation(n, Operator(Operator::plus), new Cn(0.));
}

bool Analitza::isFunction(const Ci& func) const
{
	if(!m_vars->contains(func.name()))
		return false;
	
	Container *c = (Container*) m_vars->value(func.name());
	return (c && c->type()==Container::container && c->containerType() == Container::lambda);
}

Object* Analitza::func(const Container& n)
{
	Object* obj=calc(n.m_params[0]);
	obj->decorate(Object::ScopeInformation());
	
	if(!obj->isContainer() || static_cast<Container*>(obj)->containerType()!=Container::lambda) {
		m_err << i18n("Trying to call an invalid function");
		return new Cn(0.);
	}
	
	Object* ret=0;
	Container *function = (Container*) obj;
	QList<Ci*> vars = function->bvarCi();
	
	int i=0;
	
	foreach(Ci* param, vars) {
		Object* val=calc(n.m_params[++i]);
		param->value()=val;
	}
	
	ret=calc(function->m_params.last());
	
	foreach(Ci* param, vars) {
		delete param->value();
	}
	
	return ret;
}

QStringList Analitza::bvarList() const //FIXME: if
{
	return m_exp.bvarList();
}

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void Analitza::simplify()
{
	if(m_exp.isCorrect()) {
		Object* o=simp(m_exp.tree());
		m_exp.setTree(o);
	}
}

void Analitza::levelOut(Container *c, Container *ob, Container::iterator &pos)
{
	Container::iterator it = ob->firstValue();
	for(; it!=ob->m_params.end();) {
		pos=c->m_params.insert(pos, *it);
		
		pos++;
		it=ob->m_params.erase(it);
	}
}

bool Analitza::isLambda(const Object* o)
{
	return o->isContainer() && static_cast<const Container*>(o)->containerType()==Container::lambda;
}

Object* Analitza::simp(Object* root)
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
	} else if(root->isContainer()) {
		Container *c= (Container*) root;
		bool d;
		if(c->containerType()==Container::piecewise) {
			root=simpPiecewise(c);
		} else if(c->containerType()==Container::lambda) {
			c->m_params.last()=simp(c->m_params.last());
		} else if(c->containerType()==Container::apply) {
			Container::iterator it;
			Operator o = c->firstOperator();
			Q_ASSERT(c->isCorrect());
			
			switch(o.operatorType()) {
				case Operator::times:
					for(it=c->firstValue(); c->m_params.count()>1 && it!=c->m_params.end();) {
						d=false;
						*it = simp(*it);
						if((*it)->isContainer()) {
							Container *intr = static_cast<Container*>(*it);
							if(intr->firstOperator()==o) {
								levelOut(c, intr, it);
								d=true;
							}
						}
						
						if(!d && (*it)->type() == Object::value) {
							Cn* n = (Cn*) (*it);
							if(n->value()==1. && c->m_params.count()>2) { //1*exp=exp
								d=true;
							} else if(n->value()==0.) { //0*exp=0
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
						Container *aux=c;
						root=*c->firstValue();
						*aux->firstValue()=0;
						delete aux;
					} else {
						Object *ret=simpScalar(c);
						if(ret->isContainer()) {
							c=static_cast<Container*>(ret);
							ret=simpPolynomials(c);
							c=ret->isContainer() ? static_cast<Container*>(ret) : 0;
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
					bool somed=false, lastdel;
					it=c->m_params.end();
					--it;
					
					for(; it!=c->m_params.begin(); --it) {
						lastdel=false;
						*it = simp(*it);
						
						d=false;
						if((*it)->isContainer()) {
							Container *intr = (Container*) *it;
							if(intr->containerType()==Container::apply && intr->firstOperator()==o) {
								if(!intr->isUnary() || intr->firstOperator()!=Operator::minus) {
									levelOut(c, intr, it);
									d=true;
								}
							}
						}
#ifndef Q_CC_MSVC
						#warning review condition
#endif
						if(((*it)->type()==Object::value || ((*it)->type()==Object::vector && !hasVars(*it))) && (*it)->isZero()) {
							d=true;
						}
						
						if(d) {
							lastdel=true;
							somed=true;
							delete *it;
							it = c->m_params.erase(it);
						}
					}
					
					root=c;
					
					if(c->isUnary() && o==Operator::plus) {
						root=*c->firstValue();
						*c->firstValue()=0;
						delete c;
						c=0;
					} else if(c->isUnary() && o==Operator::minus) {
						if(somed && !lastdel) {
							root=*c->firstValue();
							*c->firstValue()=0;
							delete c;
							c=0;
						} else if((*c->firstValue())->isContainer()) {
							Container *c1=(Container*) *c->firstValue();
							if(c1->containerType()==Container::apply &&
								c1->firstOperator()==Operator::minus &&
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
						
						if(root->isContainer()) {
							c=static_cast<Container*>(root);
							root=simpPolynomials(c);
							
							c=root->isContainer() ? static_cast<Container*>(root) : 0;
						} else
							c=0;
					}
					
					if(c && c->isEmpty()) {
						delete root;
						root = new Cn(0.);
					}
				}	break;
				case Operator::power: {
					for(it = c->firstValue(); it!=c->m_params.end(); ++it)
						*it = simp(*it);
					
					if(c->m_params[2]->type()==Object::value) {
						Cn *n = (Cn*) c->m_params[2];
						if(n->value()==0.) { //0*exp=0
							delete root;
							root = new Cn(1.);
							break;
						} else if(n->value()==1.) { 
							root = c->m_params[1];
							c->m_params[1]=0;
							delete c;
							break;
						}
					}
					
					if(c->m_params[1]->isContainer()) {
						Container *cp = (Container*) c->m_params[1];
						if(cp->firstOperator()==Operator::power) {
							c->m_params[1] = cp->m_params[1]->copy();
							
							Container *cm = new Container(Container::apply);
							cm->appendBranch(new Operator(Operator::times));
							cm->appendBranch(c->m_params[2]->copy());
							cm->appendBranch(cp->m_params[2]->copy());
							c->m_params[2] = cm;
							delete cp;
							c->m_params[2]=simp(c->m_params[2]);
						}
					}
				} break;
				case Operator::divide:
					for(it = c->firstValue(); it!=c->m_params.end(); ++it)
						*it = simp(*it);
					
					Object *f, *g; //f/g
					f=*c->firstValue();
					g=*(c->firstValue()+1);
					
					if(Container::equalTree(f, g)) {
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
				case Operator::sum: {
					
					const int offset=1+c->bvarCi().size(); //op+bvars
					for(it = c->m_params.begin()+offset; it!=c->m_params.end(); ++it) {
						if((*it)->isContainer()) {
							Container *limit=static_cast<Container*>(*it);
							if(limit->containerType()==Container::uplimit || limit->containerType()==Container::downlimit)
								limit->m_params.first()=simp(limit->m_params.first());
							else
								*it = simp(*it);
							
						}else
							*it = simp(*it);
					}
					
					//if bvars is empty, we are dealing with an invalid sum()
					QStringList bvars=c->bvarStrings();
					it = c->firstValue();
					if(!bvars.isEmpty() && !hasTheVar(bvars.toSet(), *it)) {
						Container *cDiff=new Container(Container::apply);
						cDiff->appendBranch(new Operator(Operator::minus));
						cDiff->appendBranch(c->ulimit()->copy());
						cDiff->appendBranch(c->dlimit()->copy());
						
						Container *nc=new Container(Container::apply);
						nc->appendBranch(new Operator(Operator::times));
						nc->appendBranch(cDiff);
						nc->appendBranch(*it);
						
						c->m_params.last()=0;
						delete c;
						root=simp(nc);
					} else if((*it)->isContainer())
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
					c->m_params[1]=simp(c->m_params[1]);
					c->m_params[2]=simp(c->m_params[2]);
					
					Object* idx=c->m_params[1];
					Object* value=c->m_params[2];
					if(idx->type()==Object::value && value->type()==Object::vector) {
						QString err;
						Object* ret=Operations::reduce(Operator::selector, idx->copy(), value->copy(), err);
						
						if(ret) {
							root=ret;
							c->m_params[1]=0;
							c->m_params[2]=0;
							
							delete c;
						}
					}
				}	break;
				case Operator::_union: {
					Container::iterator it=c->firstValue(), itEnd=c->m_params.end();
					
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
						newParams.prepend(c->m_params.takeFirst());
						qDeleteAll(c->m_params);
						c->m_params=newParams;
						root=c;
					}
					
				}	break;
				default:
					it = c->firstValue();
					
					for(; it!=c->m_params.end(); ++it)
						*it = simp(*it);
					break;
			}
		} else {
			Container::iterator it = c->firstValue();
			
			for(; it!=c->m_params.end(); ++it)
				*it = simp(*it);
		}
	}
	return root;
}

Object* Analitza::simpScalar(Container * c)
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

Object* createMono(const Operator& o, const QPair<double, Object*>& p)
{
	Object* toAdd=0;
	if(p.first==0.) {
		delete p.second;
	} else if(p.first==1.) {
		toAdd=p.second;
	} else if(p.first==-1. && (o==Operator::plus || o==Operator::minus)) {
		Container *cint = new Container(Container::apply);
		cint->appendBranch(new Operator(Operator::minus));
		cint->appendBranch(p.second);
		toAdd=cint;
	} else {
		Container *cint = new Container(Container::apply);
		cint->appendBranch(new Operator(o.multiplicityOperator()));
		cint->appendBranch(p.second);
		cint->appendBranch(new Cn(p.first));
		if(o.multiplicityOperator()==Operator::times)
			cint->m_params.swap(1,2);
		toAdd=cint;
	}
	return toAdd;
}

Object* Analitza::simpPolynomials(Container* c)
{
	Q_ASSERT(c!=0 && dynamic_cast<Container*>(c));
	
	QList<QPair<double, Object*> > monos;
	Operator o(c->firstOperator());
	bool sign=true, first=true;
	Container::const_iterator it(c->firstValue());
	
	for(; it!=c->m_params.constEnd(); ++it) {
		Object *o2=*it;
		QPair<double, Object*> imono;
		bool ismono=false;
		
		if(o2->isContainer()) {
			Container *cx = (Container*) o2;
			if(cx->firstOperator()==o.multiplicityOperator() && cx->m_params.count()==3) {
				bool valid=false;
				int scalar, var;
				
				if(cx->m_params[1]->type()==Object::value) {
					scalar=1;
					var=2;
					valid=true;
				} else if(cx->m_params[2]->type()==Object::value) {
					scalar=2;
					var=1;
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
		
		if(o!=Operator::times && imono.second->isContainer()) {
			Container *m = (Container*) imono.second;
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
			if(o2->type()!=Object::oper && Container::equalTree(o1, o2)) {
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
		c= new Container(Container::apply);
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
		Container *cn=new Container(Container::apply);
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

Object* Analitza::simpSum(Container * c)
{
	Object* ret=c;
	QStringList bvars=c->bvarStrings();
	Container* cval=static_cast<Container*>(*c->firstValue());
	Operator o=cval->firstOperator();
	if(o==Operator::times) {
		QList<Object*> sum, out;
		Container::iterator it=cval->m_params.begin(), itEnd=cval->m_params.end(), firstV=cval->firstValue();
		bool firstFound=false;
		int multCount=0;
		for(; it!=itEnd; ++it) {
			if(it==firstV)
				firstFound=true;
			
			if(!firstFound || hasTheVar(bvars.toSet(), *it)) {
				if(firstFound)
					multCount++;
				sum.append(*it);
			} else {
				out.append(*it);
			}
			if(firstFound)
				*it=0;
		}
		
		if(!out.isEmpty()) {
			Container* nc=new Container(Container::apply);
			nc->appendBranch(new Operator(Operator::times));
			nc->m_params << out;
			nc->appendBranch(c);
			if(multCount>1)
				cval->m_params=sum;
			else if(multCount==1) {
				delete cval;
				c->m_params.last()=sum.last();
			}
			ret=simp(nc);
		}
	}
	return ret;
}

Object* Analitza::simpPiecewise(Container *c)
{
	Object *root=c;
	//Here we have a list of options and finally the otherwise option
	const Container *otherwise=0;
	Container::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
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

bool Analitza::hasVars(const Object *o, const QString &var, const QStringList& bvars, const Variables* vars)
{
	Q_ASSERT(o);
	
	bool r=false;
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			if(!var.isEmpty()) {
				r=i->name()==var;
			} else
				r=true;
			
			if(r && bvars.contains(i->name()))
				r=false;
			else if(r && vars && !var.isEmpty() && vars->contains(i->name())) {
				r=hasVars(vars->value(i->name()), var, bvars, vars);
			}
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				r |= hasVars(*it, var, bvars, vars);
			}
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				r |= hasVars(*it, var, bvars, vars);
			}
		}	break;
		case Object::container: {
			Container *c = (Container*) o;
			Container::iterator it = c->firstValue(), first = c->firstValue();
			
			QStringList scope=bvars+c->bvarStrings();
			Object* ul=c->ulimit(), *dl=c->dlimit();
			
			//uplimit and downlimit are in the parent scope
			if(ul) r |= hasVars(ul, var, bvars, vars);
			if(dl) r |= hasVars(dl, var, bvars, vars);
			
			for(; !r && it!=c->m_params.end(); ++it) {
				r |= hasVars(*it, var, scope, vars);
			}
		} break;
		case Object::none:
		case Object::value:
		case Object::oper:
			r=false;
	}
	
	return r;
}

Object::ScopeInformation Analitza::varsScope() const
{
	Object::ScopeInformation varsScope;
	Variables::iterator it, itEnd=m_vars->end();
	for(it=m_vars->begin(); it!=itEnd; ++it) {
		varsScope.insert(it.key(), &it.value());
	}
	
	for(it=m_vars->begin(); it!=itEnd; ++it) {
		(*it)->decorate(varsScope);
	}
	
	return varsScope;
}


Expression Analitza::derivative()
{
	m_err.clear();
	/* TODO: Must support multiple bvars */
	Expression exp;
	if(m_exp.isCorrect()) {
		QStringList vars = bvarList();
		if(vars.empty())
			vars+="x";
		
		Object* o = simp(derivative(vars.first(), m_exp.tree()));
		exp.setTree(o);
	}
	return exp;
}

bool Analitza::insertVariable(const QString & name, const Expression & value)
{
	return insertVariable(name, value.tree());
}

bool Analitza::insertVariable(const QString & name, const Object * value)
{
	bool islambda=false;
	if(value->isContainer()) {
		const Container *c=static_cast<const Container*>(value);
		islambda= c->containerType()==Container::lambda;
	}
	
	bool correct;
	if(!islambda && hasVars(value, name, QStringList(), m_vars)) {
		m_err << i18nc("By a cycle i mean a variable that depends on itself", "Defined a variable cycle");
		correct=false;
	} else {
		m_vars->modify(name, value);
		correct=true;
	}
	return correct;
}

bool Analitza::hasTheVar(const QSet<QString> & vars, const Object * o)
{
	bool found=false;
	const Ci* cand;
	switch(o->type()) {
		case Object::vector: {
			const Vector *v=static_cast<const Vector*>(o);
			Vector::const_iterator it, itEnd=v->constEnd();
			for(it=v->constBegin(); it!=itEnd; ++it) {
				found |= hasTheVar(vars, *it);
			}
		}	break;
		case Object::list: {
			const List *v=static_cast<const List*>(o);
			List::const_iterator it, itEnd=v->constEnd();
			for(it=v->constBegin(); it!=itEnd; ++it) {
				found |= hasTheVar(vars, *it);
			}
		}	break;
		case Object::container: {
			const Container *c=static_cast<const Container*>(o);
			QSet<QString> bvars=c->bvarStrings().toSet(), varsCopy=vars;
			foreach(const QString &var, bvars) {
				if(varsCopy.contains(var))
					varsCopy.remove(var);
			}
			found=hasTheVar(varsCopy, c);
		}	break;
		case Object::variable:
			cand=static_cast<const Ci*>(o);
			found=vars.contains(cand->name());
			break;
		case Object::value:
		case Object::oper:
		case Object::none:
			found=false;
			break;
	}
	return found;
}

bool Analitza::hasTheVar(const QSet<QString> & vars, const Container * c)
{
	bool found=false;
	if(c->containerType()!=Container::bvar) {
		Container::const_iterator it=c->firstValue(), itEnd=c->m_params.constEnd();
		for(; !found && it!=itEnd; ++it) {
			if(hasTheVar(vars, *it))
				found=true;
		}
	}
	return found;
}

typedef QPair<QString, double> StringDoublePair;
double Analitza::derivative(const QList<StringDoublePair>& values )
{
	//c++ numerical recipes p. 192. Only for f'
	//Image
	//TODO: Should not call ::calculate()
	foreach(const StringDoublePair& valp, values) {//TODO: it should be +-hh
		m_vars->modify(valp.first, valp.second);
	}
	
	Expression e1=calculate();
	if(!isCorrect())
		return 0.;
		
	foreach(const StringDoublePair& valp, values) {
		m_vars->destroy(valp.first);
		Q_ASSERT(!m_vars->contains(valp.first));
	}
	
	//Image+h
	double h=0.0000000001;
	foreach(const StringDoublePair& valp, values) {
// 		volatile double temp=valp.second+h;
// 		double hh=temp-valp.second;
		m_vars->stack(valp.first, valp.second+h);
	}
	
	Expression e2=calculate();
	if(!isCorrect())
		return 0.;
	
	foreach(const StringDoublePair& valp, values)
		m_vars->destroy(valp.first);
	
	if(!e1.isReal() || !e2.isReal()) {
		m_err << i18n("The result is not a number");
		return 0;
	}
	double ret=(e2.toReal().value()-e1.toReal().value())/h;
	
	return ret;
}

QStringList Analitza::dependencies(const Object* o, const QStringList& scope)
{
	Q_ASSERT(o);
	
	QStringList ret;
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			if(!scope.contains(i->name()))
				ret += i->name();
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				ret += dependencies(*it, scope);
			}
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				ret += dependencies(*it, scope);
			}
		}	break;
		case Object::container: {
			Container *c = (Container*) o;
			Container::iterator it = c->firstValue(), first = c->firstValue();
			
			QStringList newScope=scope+c->bvarStrings();
			Object* ul=c->ulimit(), *dl=c->dlimit();
			
			//uplimit and downlimit are in the parent scope
			if(ul) ret += dependencies(ul, scope);
			if(dl) ret += dependencies(dl, scope);
			
			for(; it!=c->m_params.end(); ++it) {
				ret += dependencies(*it, newScope);
			}
		} break;
		case Object::none:
		case Object::value:
		case Object::oper:
			break;
	}
	
	return ret;
}
