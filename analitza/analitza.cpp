/*************************************************************************************
 *  Copyright (C) 2007-2008 by Aleix Pol <aleixpol@gmail.com>                        *
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
	if(m_exp.isCorrect()) {
		return Expression(calc(m_exp.tree()));
	} else {
		m_err << i18n("Must specify a correct operation");
		return Expression();
	}
}

Object* Analitza::eval(const Object* branch, bool resolve, const QSet<QString>& unscoped)
{
	Q_ASSERT(branch && branch->type()!=Object::none);
	Object *ret=0;
	
	//Won't calc() so would be a good idea to have it simplified
	if(branch->isContainer()) {
		const Container* c = (Container*) branch;
		
// 		Q_ASSERT(!c->isEmpty());
		if(c->containerType()==Container::declare) {
			Ci *var = (Ci*) c->m_params[0];
			ret = eval(c->m_params[1], true, unscoped);
			ret=simp(ret);
			insertVariable(var->name(), ret);
		} else if(c->containerType()==Container::piecewise) {
			Container::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
			
			bool boundeddep=false;
			for(; !ret && it!=itEnd; ++it) {
				Container *p=static_cast<Container*>(*it);
				Q_ASSERT( (*it)->type()==Object::container &&
					(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
				bool isPiece = p->containerType()==Container::piece;
				if(isPiece) {
					Object *cond=eval(p->m_params[1], resolve, unscoped);
					cond=simp(cond);
					boundeddep=boundeddep || hasTheVar(unscoped.toList(), cond);
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
				ret=Expression::objectCopy(c);
		} else if(c->containerType()==Container::apply) {
			Operator op = c->firstOperator();
			switch(op.operatorType()) {
				case Operator::diff: {
					//FIXME: Must support multiple bvars
					QStringList bvars = c->bvarList();
					ret = derivative(bvars.empty() ? "x" : bvars[0], *c->firstValue());
					break;
				} case Operator::none:
					ret = eval(c->m_params[0], resolve, unscoped);
					break;
				default: { //FIXME: Should we replace the function? the only problem appears if undeclared var in func
					//This code aims to scope any bounded function
					const Container *c = (Container*) branch;
					Operator op(c->firstOperator());
					if(op.operatorType()==Operator::function && op.isBounded()) {
						//it is a function. I'll take only this case for the moment
						//it is only meant for operations with scoped variables that _change_ its value => have a value
						const Container *cbody=0;
						QStringList bvars;
						if(op.operatorType()==Operator::function) {
							Ci *func= (Ci*) c->m_params[0];
							if(m_vars->contains(func->name())) { //FIXME: Don't really know if i fixed that properly
								Object* body= (Object*) m_vars->value(func->name()); //body is the value
								if(body->type()!=Object::container) { //if it is a value variable
									ret = eval(body, resolve, unscoped);
									break;
								}
								cbody = (Container*) body;
								
								Container::const_iterator bv = cbody->m_params.constBegin();
								for(; bv!=cbody->m_params.constEnd(); ++bv) {
									if((*bv)->isContainer()) {
										const Container *ibv = (Container*) *bv;
										if(ibv->containerType()==Container::bvar) {
											if(ibv->m_params[0]->type()==Object::variable) {
												const Ci* ivar = (Ci*) ibv->m_params[0];
												bvars.append(ivar->name());
											}
										}
									}
								}
								
								QStringList::const_iterator iBvars = bvars.constBegin();
								int i=0;
								for(; iBvars!=bvars.constEnd(); ++iBvars) {
									Object *val = simp(eval(c->m_params[++i], resolve, unscoped));
									m_vars->stack(*iBvars, val);
									delete val;
								}
							}
						}
						
						if(cbody) {
// 							qDebug() << "before" << cbody->toString();
							/*Container *r = new Container(cbody);
							Container::iterator it(r->firstValue());
							for(; it!=r->m_params.end(); ++it) {
								Object *o=*it;
								*it = eval(o, resolve, unscoped);
								delete o;
							}*/
							ret=eval(cbody->m_params.last(), resolve, unscoped);
// 							qDebug() << "after" << ret->toString() << (*cbody->firstValue())->toString() << op.toString();
						} else {
							ret=Expression::objectCopy(c);
						}
						
						QStringList::const_iterator iBvars(bvars.constBegin());
						for(; iBvars!=bvars.constEnd(); ++iBvars)
							m_vars->destroy(*iBvars);
						
						/*if(op.operatorType()!=Operator::function) {
							Container *nc = new Container(c);
							Container::iterator fval = nc->firstValue();
							delete *fval;
							*fval=ret;
							ret = nc;
						}*/
					} else {
						QSet<QString> newUnscoped(unscoped);
						if(op.isBounded()) {
							newUnscoped+=c->bvarList().toSet();
						}
						Container *r = new Container(c);
						Container::iterator it(r->firstValue());
						for(; it!=r->m_params.end(); ++it) {
							Object *o=*it;
							*it= eval(*it, resolve, newUnscoped);
							delete o;
						}
						
						ret=r;
					}
				} break;
			}
		} else if(c->containerType()==Container::lambda) {
			QSet<QString> newUnscoped(unscoped);
			newUnscoped+=c->bvarList().toSet();
			
			Container *r = new Container(c);
			Object* old=r->m_params.last();
			r->m_params[1]=eval(r->m_params[1], false, newUnscoped);
			delete old;
			
			ret=r;
		} else if(c->containerType()==Container::math && !c->m_params.isEmpty()) {
			//TODO: Multiline. Add a loop here!
			ret=eval(c->m_params[0], resolve, unscoped);
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
	}
	if(!ret)
		ret=Expression::objectCopy(branch);
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
		case Object::vector:
		{
			Vector *v=(Vector*) o;
			Vector *c1=new Vector(v->size());
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
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
				for(;it!=c->m_params.end(); ++it) {
					Container *neach = new Container(Container::apply);
					neach->appendBranch(new Operator(Operator::times));
					
					Container::const_iterator iobj(c->firstValue());
						for(; iobj!=c->m_params.end(); ++iobj) {
						Object *o;
						o=Expression::objectCopy(*iobj);
						if(iobj==it)
							o=derivative(var, o);
						
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
					nc->appendBranch(Expression::objectCopy(c));
					Container *nAss = new Container(Container::apply);
					nAss->appendBranch(new Operator(Operator::plus));
					nc->appendBranch(nAss);
					
					Container *nChain1 = new Container(Container::apply);
					nChain1->appendBranch(new Operator(Operator::times));
					nChain1->appendBranch(derivative(var, *c->firstValue()));
					
					Container *cDiv = new Container(Container::apply);
					cDiv->appendBranch(new Operator(Operator::divide));
					cDiv->appendBranch(Expression::objectCopy(*(c->firstValue()+1)));
					cDiv->appendBranch(Expression::objectCopy(*c->firstValue()));
					nChain1->appendBranch(cDiv);
					
					Container *nChain2 = new Container(Container::apply);
					nChain2->appendBranch(new Operator(Operator::times));
					nChain2->appendBranch(derivative(var, *(c->firstValue()+1)));
					
					Container *cLog = new Container(Container::apply);
					cLog->appendBranch(new Operator(Operator::ln));
					cLog->appendBranch(Expression::objectCopy(*c->firstValue()));
					nChain2->appendBranch(cLog);
					
					nAss->appendBranch(nChain1);
					nAss->appendBranch(nChain2);
					return nc;
				} else {
					Container *cx = new Container(Container::apply);
					cx->appendBranch(new Operator(Operator::times));
					cx->appendBranch(Expression::objectCopy(c->m_params[2]));
					cx->appendBranch(derivative(var, *c->firstValue()));
					
					Container* nc= new Container(Container::apply);
					nc->appendBranch(Expression::objectCopy(c->m_params[0]));
					nc->appendBranch(Expression::objectCopy(c->m_params[1]));
					cx->appendBranch(nc);
					
					Container *degree = new Container(Container::apply);
					degree->appendBranch(new Operator(Operator::minus));
					degree->appendBranch(Expression::objectCopy(c->m_params[2]));
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
				nc->appendBranch(Expression::objectCopy(*c->firstValue()));
				ncChain->appendBranch(nc);
				return ncChain;
			} break;
			case Operator::cos: {
				Container *ncChain = new Container(Container::apply);
				ncChain->appendBranch(new Operator(Operator::times));
				ncChain->appendBranch(derivative(var, *c->firstValue()));
				
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::sin));
				nc->appendBranch(Expression::objectCopy(*c->firstValue()));
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
				lilcosine->appendBranch(Expression::objectCopy(*c->firstValue()));
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
				cmin1->appendBranch(Expression::objectCopy(g));
				cmin->appendBranch(cmin1);
				nc->appendBranch(cmin);
				
				Container *cmin2 =new Container(Container::apply);
				cmin2->appendBranch(new Operator(Operator::times));
				cmin2->appendBranch(Expression::objectCopy(f));
				cmin2->appendBranch(derivative(var, g));
				cmin->appendBranch(cmin2);
				
				Container *cquad = new Container(Container::apply);
				cquad->appendBranch(new Operator(Operator::power));
				cquad->appendBranch(Expression::objectCopy(g));
				cquad->appendBranch(new Cn(2.));
				nc->appendBranch(cquad);
				
	// 			qDebug() << "iei!" << cmin->toString();
				return nc;
			} break;
			case Operator::ln: {
				Container *nc = new Container(Container::apply);
				nc->appendBranch(new Operator(Operator::divide));
				nc->appendBranch(derivative(var, *c->firstValue()));
				nc->appendBranch(Expression::objectCopy(*c->firstValue()));
				return nc;
			} break;
			default: {
				m_err.append(i18n("The %1 derivative has not been implemented.", op.toString()));
				Container* obj = new Container(Container::apply);
				obj->appendBranch(new Operator(Operator::diff));
				obj->appendBranch(Expression::objectCopy(c));
				return obj;
			}
		}
	} else if(c->containerType()==Container::piecewise) {
		Container *newPw = new Container(c);
		
		Container::const_iterator it;
		int i=newPw->m_params.count();
		//NOTE: This i-- is because it overflows the list, looks like a Qt bug
		for(it=newPw->m_params.constBegin(); i-- && it!=c->m_params.constEnd(); ++it) {
			Q_ASSERT((*it)->isContainer());
			Container *p = (Container *) *it;
			Object *aux=p->m_params[0];
			p->m_params[0]=derivative(var, p->m_params[0]);
			delete aux;
		}
		return newPw;
	} else {
		Container *cret = new Container(c->containerType());
		Container::const_iterator it = c->m_params.begin(), end=c->m_params.end();
		for(; it!=end; ++it) {
			cret->appendBranch(derivative(var, *it));
		}
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
	Container::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
	for(; !r && it!=itEnd; ++it) {
		Container *p=static_cast<Container*>(*it);
		Q_ASSERT( (*it)->type()==Object::container &&
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
			if(otherwise)
				m_err << i18nc("this is an error message. otherwise is the else in a mathml condition",
					"Too many <em>otherwise</em> parameters");
			else
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
	if(c->m_params.count()!=2 || c->m_params[0]->type()!=Object::variable) {
		m_err << i18n("Need a var name and a value");
		return new Cn(0.);
	}
			
	const Ci *var = (const Ci*) c->m_params[0];
	//NOTE: Should not procrastinate the variable resolution... I think :)
	//m_vars->modify(var->name(), Expression::objectCopy(c->m_params[1]));
	
	//I wonder why eval is inside calc...
	Object* o = eval(c->m_params[1], true, QSet<QString>());
	o=simp(o);
	insertVariable(var->name(), o);
	if(o->type()==Object::vector || o->type()==Object::value)
		ret=o;
	else {
		//should return NaN
		ret=new Cn(0.);
		delete o;
	}
	Q_ASSERT(ret);
	return ret;
}

Object* Analitza::calc(const Object* root)
{
	Q_ASSERT(root);
	Object* ret=0;
	Ci *a;
	
// 	objectWalker(root);
	switch(root->type()) {
		case Object::container:
			ret = operate((Container*) root);
			break;
		case Object::vector: {
			const Vector *v=static_cast<const Vector*>(root);
			Vector *nv= new Vector(v->size());
			Vector::const_iterator it, itEnd=v->constEnd();
			
			for(it=v->constBegin(); it!=itEnd; ++it)
			{
				nv->appendBranch(calc(*it));
			}
			ret = nv;
		}	break;
		case Object::value:
			ret=Expression::objectCopy(root);
			break;
		case Object::variable:
			a=(Ci*) root;
			if(KDE_ISLIKELY(m_vars->contains(a->name())))
				ret = calc(m_vars->value(a->name()));
			else {
				if(a->isFunction())
					m_err << i18n("The function <em>%1</em> does not exist", a->name());
				else
					m_err << i18n("The variable <em>%1</em> does not exist", a->name());
				ret = new Cn(0.);
			}
			
			break;
		case Object::oper:
		case Object::none:
			break;
	}
	return ret;
}

Object* Analitza::operate(const Container* c)
{
	Q_ASSERT(c);
	Object* ret;
	
	if(KDE_ISUNLIKELY(c->isEmpty())) {
		m_err << i18n("Empty container: %1", c->tagName());
		return new Cn(0.);
	}
	
	switch(c->containerType()) { //TODO: Diffs should be implemented here.
		case Container::math:
		case Container::bvar:
		case Container::uplimit:
		case Container::downlimit:
		case Container::lambda:
			ret=calc(*c->firstValue());
			break;
		case Container::apply:
		{
			Operator op = c->firstOperator();
			Operator::OperatorType opt=op.operatorType();
			
			if(opt==Operator::sum) 
				ret = sum(*c);
			else if(opt==Operator::product)
				ret = product(*c);
			else if(opt==Operator::selector) {
				Object *idx=calc(c->m_params[1]), *vect=calc(c->m_params[2]);
				ret = selector(idx, vect);
				delete idx; delete vect;
			} else if(opt==Operator::diff) {
				//TODO: Make multibvar
				QStringList bvar=c->bvarList();
				Object *deriv=simp(derivative(bvar.isEmpty() ? "x" : bvar.first(), c->m_params.last()));
				ret=calc(deriv);
				delete deriv;
			} else if(opt==Operator::function) {
				Ci* var= (Ci*) c->m_params[0];
				
				if(var->isFunction())
					ret = func(c);
				else
					ret = calc(c->m_params[0]);
			} else {
				QList<Object*> numbers;
				Container::const_iterator it = c->firstValue(), itEnd=c->m_params.constEnd();
				for(; it!=itEnd; ++it) {
					numbers.append(calc(*it));
				}
				
				if(!opt) {
					ret = numbers.first();
				} else if(((op.nparams()<0 && numbers.count()<=1) ||
							(op.nparams()>-1 && numbers.count()!=op.nparams())) && opt!=Operator::minus) {
					if(op.nparams()<0)
						m_err << i18n("<em>%1</em> needs at least 2 parameters", op.toString());
					else
						m_err << i18n("<em>%1</em> requires %2 parameters", op.toString(), op.nparams());
					
					qDeleteAll(numbers);
					ret = new Cn(0.);
				} else if(KDE_ISLIKELY(!numbers.isEmpty())) {
					ret = numbers.first();
					
					QString correct;
					if(numbers.count()>=2) {
						Container::const_iterator it=numbers.constBegin()+1;
						Container::const_iterator itEnd=numbers.constEnd();
						
						for(; it!=itEnd; ++it) {
							ret=Operations::reduce(opt, ret, *it, correct);
							
							if(KDE_ISUNLIKELY(!correct.isEmpty())) {
								m_err.append(correct);
								correct.clear();
							}
						}
					} else {
						ret=Operations::reduceUnary(opt, ret, correct);
						if(KDE_ISUNLIKELY(!correct.isEmpty())) {
							m_err.append(correct);
							correct.clear();
						}
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
			m_err << i18n("piece or otherwise in the wrong place");
			ret=0;
			break;
		case Container::none:
		case Container::last:
			break;
	}
	Q_ASSERT(ret);
	return ret;
}

Object* Analitza::sum(const Container& n)
{
	Object* ret=new Cn(0.);
	QString var= n.bvarList().first();
	
	Object *objul=calc(Expression::uplimit(n).tree());
	Object *objdl=calc(Expression::downlimit(n).tree());
	double ul, dl;
	
	if(objul->type()==Object::value && objdl->type()==Object::value) {
		Cn *u=static_cast<Cn*>(objul);
		Cn *d=static_cast<Cn*>(objdl);
		ul=u->value();
		dl=d->value();
		delete u;
		delete d;
	} else {
		m_err.append(i18n("Missing or uncorrect uplimit or downlimit."));
		return new Cn(0.);
	}
	
	m_vars->stack(var, 0.);
	Cn* c = (Cn*) m_vars->value(var);
	
	QString correct;
	for(double a = dl; a<=ul; a++){
		Q_ASSERT(isCorrect());
		c->setValue(a);
		Object *val=calc(n.m_params.last());
		ret=Operations::reduce(Operator::plus, ret, val, correct);
	}
	
	m_vars->destroy(var);
	return ret;
}

Object* Analitza::product(const Container& n)
{
	Object* ret=new Cn(1.);
	QString var= n.bvarList().first();
	
	Object *objul=calc(Expression::uplimit(n).tree());
	Object *objdl=calc(Expression::downlimit(n).tree());
	double ul, dl;
	
	if(objul->type()==Object::value && objdl->type()==Object::value) {
		Cn *u=static_cast<Cn*>(objul);
		Cn *d=static_cast<Cn*>(objdl);
		ul=u->value();
		dl=d->value();
		delete u;
		delete d;
	} else {
		m_err.append(i18n("Missing or uncorrect uplimit or downlimit."));
		return new Cn(0.);
	}
	
	m_vars->stack(var, 0.);
	Cn* c = (Cn*) m_vars->value(var);
	
	QString correct;
	for(double a = dl; a<=ul; a++){
		Q_ASSERT(isCorrect());
		c->setValue(a);
		Object *val=calc(n.m_params.last());
		ret=Operations::reduce(Operator::times, ret, val, correct);
	}
	
	m_vars->destroy(var);
	return ret;
}

Object* Analitza::selector(const Object* index, const Object* vector)
{
	Object *ret;
	if(index->type()==Object::value && vector->type()==Object::vector) {
		const Cn *cIdx=static_cast<const Cn*>(index);
		const Vector *cVect=static_cast<const Vector*>(vector);
		
		int select=cIdx->intValue();
		if(select<1 || (select-1) > cVect->size()) {
			m_err << i18n("Invalid index for a container");
			ret=new Cn(0.);
		} else {
			ret=Expression::objectCopy(cVect->at(select-1));
		}
	} else {
		ret = new Cn(0.);
		m_err << i18n("We can only select a containers value with its integer index");
	}
	return ret;
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
	Object* ret=0;
	Ci funct(n.m_params[0]);
	
	if(funct.type()!=Object::variable || !funct.isFunction() || !m_vars->contains(funct.name())) {
		m_err << i18n("The function <em>%1</em> does not exist", funct.name());
		return new Cn(0.);
	}
	
	Object* obj = m_vars->value(funct.name());
	if(obj->isContainer()) {
		Container *function = (Container*) obj;
		QStringList var = function->bvarList();
		
		for(int i=0; i<var.count(); i++) {
			Object* val=calc(n.m_params[i+1]);
			m_vars->stack(var[i], val);
			delete val;
		}
		
		ret=calc(function->m_params[var.count()]);
		
		for(int i=0; i<var.count(); i++) {
			m_vars->destroy(var[i]);
		}
	} else
		ret=calc(obj);
	
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

Object* Analitza::simp(Object* root)
{
	Q_ASSERT(root && root->type()!=Object::none);
	
	Object* aux=0;
	if(!hasVars(root)) {
		if(root->type()!=Object::value && root->type() !=Object::oper) {
			aux = root;
			root = calc(root);
			delete aux;
		}
	} else if(root->type()==Object::vector) {
		Vector* v=static_cast<Vector*>(root);
		Vector::iterator it = v->begin(), itEnd=v->end();
		
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
								
// 								qDebug() << "-------- before1" << (*it)->toString();
// 								objectWalker(c);
								
// 								qDebug() << "-------- middle1" << (*it)->toString();
// 								objectWalker(c);
								if(!intr->isUnary() || intr->firstOperator().operatorType()!=Operator::minus) {
									levelOut(c, intr, it);
									d=true;
								}
// 								qDebug() << "-------- after1";
// 								objectWalker(c);
// 								qDebug() << "-------- done1";
							}
						}
						
						if(((*it)->type()==Object::value || (*it)->type()==Object::vector) && !hasVars(*it) && (*it)->isZero()) {
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
					
					if(c->isUnary() && c->firstOperator()==Operator::plus) {
						root=*c->firstValue();
						*c->firstValue()=0;
						delete c;
						c=0;
					} else if(c->isUnary() && c->firstOperator()==Operator::minus) {
						if(somed && !lastdel) {
							root=*c->firstValue();
							*c->firstValue()=0;
							delete c;
							c=0;
						} else if((*c->firstValue())->isContainer()) {
							Container *c1=(Container*) *c->firstValue();
							if(c1->containerType()==Container::apply &&
								c1->firstOperator().operatorType()==Operator::minus &&
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
							c->m_params[1] = Expression::objectCopy(cp->m_params[1]);
							
							Container *cm = new Container(Container::apply);
							cm->appendBranch(new Operator(Operator::times));
							cm->appendBranch(Expression::objectCopy(c->m_params[2]));
							cm->appendBranch(Expression::objectCopy(cp->m_params[2]));
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
				case Operator::sum:
					for(it = c->m_params.begin(); it!=c->m_params.end(); ++it) {
						if((*it)->type()==Object::container) {
							Container *limit=static_cast<Container*>(*it);
							if(limit->containerType()==Container::uplimit || limit->containerType()==Container::downlimit)
								limit->m_params.first()=simp(limit->m_params.first());
							else
								*it = simp(*it);
							
						}else
							*it = simp(*it);
					}
					
					it = c->firstValue();
					if(!hasTheVar(c->bvarList(), *it)) {
						Container *cDiff=new Container(Container::apply);
						cDiff->appendBranch(new Operator(Operator::minus));
						cDiff->appendBranch(Expression::objectCopy(c->ulimit()));
						cDiff->appendBranch(Expression::objectCopy(c->dlimit()));
						
						Container *nc=new Container(Container::apply);
						nc->appendBranch(new Operator(Operator::times));
						nc->appendBranch(cDiff);
						nc->appendBranch(*it);
						
						c->m_params.last()=0;
						delete c;
						root=simp(nc);
					} else if((*it)->type()==Object::container)
					   root=simpSum(c);
					
					break;
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
					if(idx->type()==Object::value && value->type()==Object::vector)
					{
						root=selector(*c->firstValue(), c->m_params.last());
						delete c;
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
	Operator o = c->firstOperator();
// 	bool sign=true;
	for(Container::iterator i = c->firstValue(); i!=c->m_params.end();) {
		bool d=false;
		
		//TODO: hasVars needed? should have already been simplifyed before, just check type==cn
		if(((*i)->type()==Object::value || (*i)->type()==Object::vector) && !hasVars(*i)) {
			Object* aux = *i;
			
			if(value) {
				QString correct;
				value=Operations::reduce(o.operatorType(), value, aux, correct);
			} else
				value=aux;
			d=true;
		}
		
		if(d) {
			i = c->m_params.erase(i);
		} else
			++i;
	}
	
	if(value) {
		if(value->isZero())
			delete value;
		else {
			switch(o.operatorType()) {
				case Operator::minus:
				case Operator::plus:
					c->appendBranch(value);
					break;
				default:
					c->m_params.insert(c->firstValue(), value);
					break;
			}
		}
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
	} else if(p.first==-1. && (o.operatorType()==Operator::plus || o.operatorType()==Operator::minus)) {
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
				if(o.operatorType()==Operator::plus) {
					//detecting -x as QPair<-1, o>
					imono.first = -1.;
					imono.second = *cx->firstValue();
					ismono=true;
				} else if(o.operatorType()==Operator::times) {
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
		
		if(o.operatorType()!=Operator::times && imono.second->isContainer()) {
			Container *m = (Container*) imono.second;
			if(m->firstOperator()==Operator::minus && m->isUnary()) {
				imono.second = *m->firstValue();
				imono.first *= -1.;
			}
		}
		
		if(o.operatorType()==Operator::minus && !first) {
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
			} else if(o.operatorType()==Operator::minus) {
				if(prevPositive ^ postPositive) {
					sign = !sign;
				}
			}
		} else {
			imono.second = Expression::objectCopy(imono.second);
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
		root=createMono(o, monos[0]);
	} else if(monos.count()>1) {
		c= new Container(Container::apply);
		c->appendBranch(new Operator(o));
		
		QList<QPair<double, Object*> >::iterator i=monos.begin();
		bool first=true;
		for(; i!=monos.end(); ++i) {
			if(!first && o.operatorType()==Operator::minus)
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
	QStringList bvars=c->bvarList();
	Container* cval=static_cast<Container*>(*c->firstValue());
	Operator o=cval->firstOperator();
	if(o.operatorType()==Operator::times) {
		QList<Object*> sum, out;
		Container::iterator it=cval->m_params.begin(), itEnd=cval->m_params.end(), firstV=cval->firstValue();
		bool firstFound=false;
		int multCount=0;
		for(; it!=itEnd; ++it) {
			if(it==firstV)
				firstFound=true;
			
			if(!firstFound || hasTheVar(bvars, *it)) {
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
		Q_ASSERT( (*it)->type()==Object::container &&
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
		case Object::container: {
			Container *c = (Container*) o;
			bool firstFound=false;
			Container::iterator it = c->m_params.begin(), first = c->firstValue();
			
			QStringList scope=bvars;
			
			for(; !r && it!=c->m_params.end(); ++it) {
				if(it==first)
					firstFound=true;
				
				if(!firstFound && (*it)->isContainer()) { //We are looking for bvar's
					Container *cont= (Container*) *it;
					if(cont->containerType()==Container::bvar && c->containerType()!=Container::lambda) {
						Ci* bvar=(Ci*) cont->m_params[0];
						if(bvar->isCorrect())
							scope += bvar->name();
					}
				} else if(firstFound)
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

Object* Analitza::removeDependencies(Object * o) const
{
	Q_ASSERT(o);
	if(o->type()==Object::variable) {
		Ci* var=(Ci*) o;
		if(m_vars->contains(var->name()) && m_vars->value(var->name())) {
			Object *value=Expression::objectCopy(m_vars->value(var->name()));
			Object *no = removeDependencies(value);
			delete o;
			return no;
		}
	} else if(o->type()==Object::container) {
		Container *c = (Container*) o;
		Operator op(c->firstOperator());
		if(c->containerType()==Container::apply && op.isBounded()) { //it is a function
			Container *cbody = c;
			QStringList bvars;
			if(op.operatorType()==Operator::function) {
				Ci *func= (Ci*) c->m_params[0];
				Object* body= (Object*) m_vars->value(func->name());
				if(body->type()!=Object::container)
					return body;
				cbody = (Container*) body;
			}
			
			if(op.operatorType()==Operator::function) {
				QStringList::const_iterator iBvars(bvars.constBegin());
				int i=0;
				for(; iBvars!=bvars.constEnd(); ++iBvars)
					m_vars->stack(*iBvars, c->m_params[++i]);
				delete c;
				c = 0;
			}
			
			Container::iterator fval(cbody->firstValue());
			Object *ret= removeDependencies(Expression::objectCopy(*fval));
			
			QStringList::const_iterator iBvars(bvars.constBegin());
			for(; iBvars!=bvars.constEnd(); ++iBvars)
				m_vars->destroy(*iBvars);
			
			
			if(op.operatorType()==Operator::function)
				return ret;
			else {
				delete *fval;
				*fval=ret;
				return c;
			}
		} else {
			Container::iterator it(c->firstValue());
			for(; it!=c->m_params.end(); ++it)
				*it = removeDependencies(*it);
		}
	}
	return o;
}

Expression Analitza::derivative()
{
	m_err.clear();
	/* FIXME: Must support multiple bvars */
	Expression exp;
	if(m_exp.isCorrect()) {
		QStringList vars = bvarList();
		Object *o;
		if(vars.empty())
			o = derivative("x", m_exp.tree());
		else
			o = derivative(vars.first(), m_exp.tree());
		exp.setTree(simp(o));
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

bool Analitza::hasTheVar(const QStringList & vars, const Object * o)
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
		case Object::container: {
			const Container *c=static_cast<const Container*>(o);
			QStringList bvars=c->bvarList(), varsCopy=vars;
			foreach(const QString &var, bvars) {
				if(varsCopy.contains(var))
					varsCopy.removeAll(var);
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

bool Analitza::hasTheVar(const QStringList & vars, const Container * c)
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

double Analitza::derivative(const QList< QPair<QString, double > >& values )
{
	//c++ numerical recipes p. 192. Only for f'
	typedef QPair<QString, double> StringDoublePair;
	//Image
	foreach(const StringDoublePair& valp, values) {//TODO: it should be +-hh
		m_vars->stack(valp.first, valp.second);
	}
		
	Object* v1=calc(m_exp.tree());
		
	foreach(const StringDoublePair& valp, values)
		m_vars->destroy(valp.first);
	
	//Image+h
	double h=0.00000001;
	foreach(const StringDoublePair& valp, values) {
// 		volatile double temp=valp.second+h;
// 		double hh=temp-valp.second;
		m_vars->stack(valp.first, valp.second+h);
	}
	
	Object* v2=calc(m_exp.tree());
		
	foreach(const StringDoublePair& valp, values)
		m_vars->destroy(valp.first);
	Q_ASSERT(v1->type()==Object::value && v2->type()==Object::value);
	Cn *cn1=(Cn*)v1, *cn2=(Cn*)v2;
	double ret=(cn2->value()-cn1->value())/h;
	
	delete v1;
	delete v2;
	return ret;
}

// QList<double> Analitza::discontinuities(const QString& var, const Bounds& b)
// {
// 	QList<double> ret;
// 	if(m_exp.isCorrect()) {
// // 		ret=discon(m_exp.tree(), var, b, QSet<QString>());
// 	} else {
// 		m_err << i18n("Must specify a correct operation");
// 	}
// 	return ret;
// }
// 
// QList<double> Analitza::discon(const Object* branch, const QString& var, const Bounds& b, const QSet<QString>& unscoped)
// {
// 	QList<double> ret;
// 	if(branch->isContainer()) {
// 		const Container* c = (Container*) branch;
// 		
// 		if(c->containerType()==Container::declare) {
// 			
// 		} else if(c->containerType()==Container::apply) {
// 			Operator op = c->firstOperator();
// 			
// 		} else if((c->containerType()==Container::math || c->containerType()==Container::declare)
// 				&& !c->m_params.isEmpty() ) {
// 			//TODO: Multiline. Add a loop here!
// 			ret=discon(c->m_params[0], var, b, unscoped);
// 		}
// 	}
// 	return ret;
// }

