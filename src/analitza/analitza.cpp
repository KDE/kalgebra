/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include "value.h"
#include "variables.h"
#include "container.h"
#include "object.h"

Analitza::Analitza() : m_vars(new Variables) { }

Analitza::Analitza(const Analitza& a) : m_exp(a.m_exp), m_err(a.m_err)
{
	m_vars = new Variables(*a.m_vars);
}

Analitza::~Analitza()
{
	if(m_vars)
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
		e.m_tree=eval(m_exp.tree(), true, QSet<QString>());
		e.m_tree=simp(e.m_tree);
	} else {
		m_err << i18n("Must specify a correct operation");
	}
	return e;
}

Cn Analitza::calculate()
{
	if(m_exp.isCorrect())
		return calc(m_exp.m_tree);
	else {
		m_err << i18n("Must specify a correct operation");
		return Cn(0.);
	}
}

Object* Analitza::eval(const Object* branch, bool resolve, const QSet<QString>& unscoped)
{
	Q_ASSERT(branch && branch->type()!=Object::none);
	Object *ret=0;
	//Won't calc() so would be a good idea to have it simplified
	if(branch->isContainer()) {
		const Container* c = (Container*) branch;
		Operator op = c->firstOperator();
// 		Q_ASSERT(!c->isEmpty());
		if(c->containerType()==Container::declare) {
			Ci *var = (Ci*) c->m_params[0];
			ret = eval(c->m_params[1], true, unscoped);
			ret=simp(ret);
			m_vars->modify(var->name(), ret);
		} else if(c->containerType()==Container::piecewise) {
			QList<Object*>::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
			for(; !ret && it!=itEnd; ++it) {
				Container *p=static_cast<Container*>(*it);
				Q_ASSERT( (*it)->type()==Object::container &&
					(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
				bool isPiece = p->containerType()==Container::piece;
				if(isPiece) {
					Object *cond=eval(p->m_params[1], resolve, unscoped);
					cond=simp(cond);
					if(cond->type()==Object::value) {
						Cn* cval=static_cast<Cn*>(cond);
						if(cval->isTrue()) {
							ret=eval(p->m_params[0], resolve, unscoped);
						}
					}
					delete cond;
				} else { //FIXME: Maybe should look for more pieces?
					ret=eval(p->m_params[0], resolve, unscoped);
				}
			}
			if(!ret)
				ret=Expression::objectCopy(c);
		} else if(c->containerType()==Container::apply) {
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
						const Container *cbody = c;
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
								
								QList<Object*>::const_iterator bv = cbody->m_params.constBegin();
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
						
// 						qDebug() << "before" << cbody->toString();
						Container *r = new Container(cbody);
						QList<Object*>::iterator it(r->firstValue());
						for(; it!=r->m_params.end(); ++it) {
							Object *o=*it;
							*it = eval(o, resolve, unscoped);
							delete o;
						}
						ret=r;
// 						qDebug() << "after" << ret->toString() << (*cbody->firstValue())->toString() << op.toString();
						
						QStringList::const_iterator iBvars(bvars.constBegin());
						for(; iBvars!=bvars.constEnd(); ++iBvars)
							m_vars->destroy(*iBvars);
						
						/*if(op.operatorType()!=Operator::function) {
							Container *nc = new Container(c);
							QList<Object*>::iterator fval = nc->firstValue();
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
						QList<Object*>::iterator it(r->firstValue());
						for(; it!=r->m_params.end(); ++it) {
							Object *o=*it;
							*it= eval(*it, resolve, newUnscoped);
							delete o;
						}
						
						ret=r;
					}
				} break;
			}
		} else if(c->containerType()==Container::math) {
			//TODO: Multiline. Add a loop here!
			ret=eval(c->m_params[0], resolve, unscoped);
// 			ret=Expression::objectCopy(c->m_params[0]);
		} /*else {
			qDebug() << "Not evaluated: " << c->tagName();
		}*/
	} else if(resolve && branch->type()==Object::variable) {
		Ci* var=(Ci*) branch;
		
		if(m_vars->contains(var->name()) && m_vars->value(var->name()) && !unscoped.contains(var->name()))
			ret = eval(m_vars->value(var->name()), resolve, unscoped);
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
				r->m_params.append(new Operator(op));
				
				QList<Object*>::const_iterator it(c->firstValue());
				for(; it!=c->m_params.end(); ++it)
					r->m_params.append(derivative(var, *it));
				
				return r;
			} break;
			case Operator::times: {
				Container *nx = new Container(Container::apply);
				nx->m_params.append(new Operator(Operator::plus));
				
				QList<Object*>::const_iterator it(c->firstValue());
				for(;it!=c->m_params.end(); ++it) {
					Container *neach = new Container(Container::apply);
					neach->m_params.append(new Operator(Operator::times));
					
					QList<Object*>::const_iterator iobj(c->firstValue());
						for(; iobj!=c->m_params.end(); ++iobj) {
						Object *o;
						o=Expression::objectCopy(*iobj);
						if(iobj==it)
							o=derivative(var, o);
						
						neach->m_params.append(o);
					}
					nx->m_params.append(neach);
				}
				return nx;
			} break;
			case Operator::power: {
				if(hasVars(c->m_params[2], var)) {
					//http://en.wikipedia.org/wiki/Table_of_derivatives
					//else [if f(x)**g(x)] -> (e**(g(x)*ln f(x)))'
					Container *nc = new Container(Container::apply);
					nc->m_params.append(new Operator(Operator::times));
					nc->m_params.append(Expression::objectCopy(c));
					Container *nAss = new Container(Container::apply);
					nAss->m_params.append(new Operator(Operator::plus));
					nc->m_params.append(nAss);
					
					Container *nChain1 = new Container(Container::apply);
					nChain1->m_params.append(new Operator(Operator::times));
					nChain1->m_params.append(derivative(var, *c->firstValue()));
					
					Container *cDiv = new Container(Container::apply);
					cDiv->m_params.append(new Operator(Operator::divide));
					cDiv->m_params.append(Expression::objectCopy(*(c->firstValue()+1)));
					cDiv->m_params.append(Expression::objectCopy(*c->firstValue()));
					nChain1->m_params.append(cDiv);
					
					Container *nChain2 = new Container(Container::apply);
					nChain2->m_params.append(new Operator(Operator::times));
					nChain2->m_params.append(derivative(var, *(c->firstValue()+1)));
					
					Container *cLog = new Container(Container::apply);
					cLog->m_params.append(new Operator(Operator::ln));
					cLog->m_params.append(Expression::objectCopy(*c->firstValue()));
					nChain2->m_params.append(cLog);
					
					nAss->m_params.append(nChain1);
					nAss->m_params.append(nChain2);
					return nc;
				} else {
					Container *cx = new Container(Container::apply);
					cx->m_params.append(new Operator(Operator::times));
					cx->m_params.append(Expression::objectCopy(c->m_params[2]));
					cx->m_params.append(derivative(var, *c->firstValue()));
					
					Container* nc= new Container(Container::apply);
					nc->m_params.append(Expression::objectCopy(c->m_params[0]));
					nc->m_params.append(Expression::objectCopy(c->m_params[1]));
					cx->m_params.append(nc);
					
					Container *degree = new Container(Container::apply);
					degree->m_params.append(new Operator(Operator::minus));
					degree->m_params.append(Expression::objectCopy(c->m_params[2]));
					degree->m_params.append(new Cn(1.));
					nc->m_params.append(degree);
					return cx;
				}
			} break;
			case Operator::sin: {
				Container *ncChain = new Container(Container::apply);
				ncChain->m_params.append(new Operator(Operator::times));
				ncChain->m_params.append(derivative(var, *c->firstValue()));
				Container *nc = new Container(Container::apply);
				nc->m_params.append(new Operator(Operator::cos));
				nc->m_params.append(Expression::objectCopy(*c->firstValue()));
				ncChain->m_params.append(nc);
				return ncChain;
			} break;
			case Operator::cos: {
				Container *ncChain = new Container(Container::apply);
				ncChain->m_params.append(new Operator(Operator::times));
				ncChain->m_params.append(derivative(var, *c->firstValue()));
				
				Container *nc = new Container(Container::apply);
				nc->m_params.append(new Operator(Operator::sin));
				nc->m_params.append(Expression::objectCopy(*c->firstValue()));
				Container *negation = new Container(Container::apply);
				negation->m_params.append(new Operator(Operator::minus));
				negation->m_params.append(nc);
				ncChain->m_params.append(negation);
				return ncChain;
			} break;
			case Operator::tan: {
				Container *ncChain = new Container(Container::apply);
				ncChain->m_params.append(new Operator(Operator::divide));
				ncChain->m_params.append(derivative(var, *c->firstValue()));
				
				Container *nc = new Container(Container::apply);
				nc->m_params.append(new Operator(Operator::power));
				
				Container *lilcosine = new Container(Container::apply);
				lilcosine->m_params.append(new Operator(Operator::cos));
				lilcosine->m_params.append(Expression::objectCopy(*c->firstValue()));
				nc->m_params.append(lilcosine);
				nc->m_params.append(new Cn(2.));
				ncChain->m_params.append(nc);
				return ncChain;
			} break;
			case Operator::divide: {
				Object *f, *g; //referring to f/g
				f=*c->firstValue();
				g=*(c->firstValue()+1);
				
				Container *nc = new Container(Container::apply);
				nc->m_params.append(new Operator(Operator::divide));
				
				Container *cmin = new Container(Container::apply);
				cmin->m_params.append(new Operator(Operator::minus));
				
				Container *cmin1 =new Container(Container::apply);
				cmin1->m_params.append(new Operator(Operator::times));
				cmin1->m_params.append(derivative(var, f));
				cmin1->m_params.append(Expression::objectCopy(g));
				cmin->m_params.append(cmin1);
				nc->m_params.append(cmin);
				
				Container *cmin2 =new Container(Container::apply);
				cmin2->m_params.append(new Operator(Operator::times));
				cmin2->m_params.append(Expression::objectCopy(f));
				cmin2->m_params.append(derivative(var, g));
				cmin->m_params.append(cmin2);
				
				Container *cquad = new Container(Container::apply);
				cquad->m_params.append(new Operator(Operator::power));
				cquad->m_params.append(Expression::objectCopy(g));
				cquad->m_params.append(new Cn(2.));
				nc->m_params.append(cquad);
				
	// 			qDebug() << "iei!" << cmin->toString();
				return nc;
			} break;
			case Operator::ln: {
				Container *nc = new Container(Container::apply);
				nc->m_params.append(new Operator(Operator::divide));
				nc->m_params.append(derivative(var, *c->firstValue()));
				nc->m_params.append(Expression::objectCopy(*c->firstValue()));
				return nc;
			} break;
			default: {
				m_err.append(i18n("The %1 derivative has not been implemented.", op.toString()));
				Container* obj = new Container(Container::apply);
				obj->m_params.append(new Operator(Operator::diff));
				obj->m_params.append(Expression::objectCopy(c));
				return obj;
			}
		}
	} else if(c->containerType()==Container::piecewise) {
		Container *newPw = new Container(c);
		
		QList<Object*>::const_iterator it;
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
		QList<Object*>::const_iterator it = c->m_params.begin(), end=c->m_params.end();
		for(; it!=end; it++) {
			cret->m_params.append(derivative(var, *it));
		}
		return cret;
	}
	return 0;
}

Cn Analitza::calc(const Object* root)
{
	Q_ASSERT(root && root->type()!=Object::none);
	Cn ret=Cn(0.);
	Ci *a;
	
	switch(root->type()) {
		case Object::container:
			ret = operate((Container*) root);
			break;
		case Object::value:
			ret=(Cn*) root;
			break;
		case Object::variable:
			a=(Ci*) root;
			
			if(KDE_ISLIKELY(m_vars->contains(a->name())))
				ret = calc(m_vars->value(a->name()));
			else if(a->isFunction())
				m_err << i18n("The function <em>%1</em> does not exist", a->name());
			else
				m_err << i18n("The variable <em>%1</em> does not exist", a->name());
			
			break;
		case Object::oper:
		default:
			break;
	}
	return ret;
}

Cn Analitza::operate(const Container* c)
{
	Q_ASSERT(c);
	Operator *op=0;
	Cn ret(0.);
	QList<Cn> numbers;
	
	if(KDE_ISUNLIKELY(c->isEmpty())) {
		m_err << i18n("Empty container: %1", c->tagName());
		return Cn(0.);
	}
	
	if(c->m_params[0]->type() == Object::oper)
		op = (Operator*) c->m_params[0]; //FIXME: c->firstOperator
	if(op!= 0 && op->operatorType()==Operator::sum) 
		ret = sum(*c);
// 	else if(op!= 0 && op->operatorType()==Operator::diff)
// 		ret = sum(*c);
	else if(op!= 0 && op->operatorType()==Operator::product)
		ret = product(*c);
	else switch(c->containerType()) { //TODO: Diffs should be implemented here.
		case Container::apply:
		case Container::math:
		case Container::bvar:
		case Container::uplimit:
		case Container::downlimit:
		{
			if(KDE_ISUNLIKELY(c->isEmpty())) {
				m_err << i18n("Container without values found.");
			} else if(c->m_params[0]->type() == Object::variable) {
				Ci* var= (Ci*) c->m_params[0];
				
				if(var->isFunction())
					ret = func(c);
				else
					ret = calc(c->m_params[0]);
			} else {
				QList<Object*>::const_iterator it = c->firstValue();
				for(; it!=c->m_params.end(); it++) {
					Q_ASSERT((*it)!=0);
					if((*it)->type() != Object::oper)
						numbers.append(calc(*it));
				}
				
				if(op==0) {
					ret = numbers.first();
				} else if(op->nparams()>-1 && numbers.count()!=op->nparams() && op->operatorType()!=Operator::minus) {
					m_err << i18n("Too much operators for <em>%1</em>", op->toString());
					ret = Cn(0.);
                                } else if(KDE_ISLIKELY(numbers.count()>=1 && op->type()==Object::oper)) {
					if(numbers.count()>=2) {
						QList<Cn>::iterator it = numbers.begin();
						ret = *it;
						
						++it;
						for(; it != numbers.end(); ++it)
							reduce(op->operatorType(), &ret, *it, false);
						
					} else {
						ret=numbers.first();
						reduce(op->operatorType(), &ret, 0., true);
					}
				} else {
					ret = numbers.first();
				}
			}
		}
			break;
		case Container::declare:
		{
			if(c->m_params.count()<=1) {
				m_err << i18n("Need a var name and a value");
				return Cn(0.);
			}
			
			Ci *var = (Ci*) c->m_params[0];
			//NOTE: Should not procrastinate the variable resolution... I think :)
// 			m_vars->modify(var->name(), Expression::objectCopy(c->m_params[1]));
			
			//I wonder why eval is inside calc...
			Object* o = eval(c->m_params[1], true, QSet<QString>());
			o=simp(o);
			m_vars->modify(var->name(), o);
			if(o->type()==Object::value)
				ret=*static_cast<Cn*>(o);
			//else
			//	should return NaN
			delete o;
		} break;
		case Container::lambda:
			ret = calc(*c->firstValue());
			break;
		case Container::piecewise: {
				//Here we have a list of options and finally the otherwise option
				const Container *otherwise=0;
				const Object *r=0;
				QList<Object*>::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
				for(; !r && it!=itEnd; ++it) {
					Container *p=static_cast<Container*>(*it);
					Q_ASSERT( (*it)->type()==Object::container &&
							(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
					bool isPiece = p->containerType()==Container::piece;
					if(isPiece) {
						Cn ret=calc(p->m_params[1]);
						if(ret.isTrue()) {
							r=p->m_params[0];
						}
					} else {
						//it is an otherwise
						if(otherwise)
							m_err << i18n("Too much <em>otherwise</em> parameters");
						else
							otherwise=p;
					}
				}
				if(!r) {
					if(otherwise)
						r=otherwise->m_params[0];
					else
						m_err << i18nc("Piecewise is how the contitional is called here",
							"Piecewise without otherwise nor matching piece.");
				}
				
				if(r)
					ret=calc(r);
		}	break;
		case Container::piece:
		case Container::otherwise:
			m_err << i18n("piece or otherwise in the wrong place");
			break;
		case Container::none:
			break;
	}
	return ret;
}

Cn Analitza::sum(const Container& n)
{
	Cn ret(.0), *c;
	QString var= n.bvarList()[0];
	double ul= Expression::uplimit(n).value();
	double dl= Expression::downlimit(n).value();
	
	m_vars->stack(var, 0.);
	c = (Cn*) m_vars->value(var);
	
	for(double a = dl; a<=ul && isCorrect(); a++){
		*c = a;
		reduce(Operator::plus, &ret, calc(n.m_params.last()), false);
	}
	
	m_vars->destroy(var);
	
	return ret;
}

Cn Analitza::product(const Container& n)
{
	Cn ret(1.), *c;
	QString var= n.bvarList()[0];
	double ul= Expression::uplimit(n).value();
	double dl= Expression::downlimit(n).value();
	
	m_vars->stack(var, 0.);
	c = (Cn*) m_vars->value(var);
	
	for(double a = dl; a<=ul && isCorrect(); a++){
		*c = a;
		reduce(Operator::times, &ret, calc(n.m_params.last()), false);
	}
	
	m_vars->destroy(var);
	
	return ret;
}

bool Analitza::isFunction(const Ci& func) const
{
	if(!m_vars->contains(func.name()))
		return false;
	
	Container *c = (Container*) m_vars->value(func.name());
	return (c && c->type()==Container::container && c->containerType() == Container::lambda);
}

Cn Analitza::func(const Container& n)
{
	Cn ret(.0);
	Ci funct(n.m_params[0]);
	
	if(funct.type()!=Object::variable || !funct.isFunction() || !m_vars->contains(funct.name())) {
		m_err << i18n("The function <em>%1</em> does not exist", funct.name());
		return ret;
	}
	
	Container *function = (Container*) m_vars->value(funct.name());
	bool cont=function->isContainer();
	if(cont) {
		QStringList var = function->bvarList();
	
		for(int i=0; i<var.count(); i++) {
			Cn val=calc(n.m_params[i+1]);
			m_vars->stack(var[i], &val);
		}
		
		ret=calc(function->m_params[var.count()]);
		
		for(int i=0; i<var.count(); i++) {
			m_vars->destroy(var[i]);
		}
	} else
		ret=calc(function);
	
	return ret;
}

void Analitza::reduce(enum Operator::OperatorType op, Cn *ret, Cn oper, bool unary)
{
	int residu;
	double a=ret->value(), b=oper.value(), c;
	bool boolean=false;
	
	switch(op) {
		case Operator::plus:
			a += b;
			break;
		case Operator::times:
			a *= b;
			break;
		case Operator::divide:
			a /=b;
			break;
		case Operator::minus:
			a = unary ? -a : a-b;
			break;
		case Operator::power:
			a = pow(a, b);
			break;
		case Operator::rem:
			if(floor(b)!=0.)
				a = static_cast<int>(floor(a)) % static_cast<int>(floor(b));
			else
				m_err << i18n("Cannot calculate the <em>remainder</em> of 0.");
			break;
		case Operator::quotient:
			a = floor(a / b);
			break;
		case Operator::factorof:
			if(floor(b)!=0.)
				a = (((int)a % (int)b)==0) ? 1.0 : 0.0;
			else {
				a = 0.;
				m_err << i18n("Cannot calculate the <em>factor</em> of 0.");
			}
			boolean = true;
			break;
		case Operator::factorial:
			b = floor(a);
			for(a=1.; b>1.; b--)
				a*=b;
			break;
		case Operator::sin:
			a=sin(a);
			break;
		case Operator::cos:
			a=cos(a);
			break;
		case Operator::tan:
			a=tan(a);
			break;
		case Operator::sec:
			a=1./cos(a);
			break;
		case Operator::csc:
			a=1./sin(a);
			break;
		case Operator::cot:
			a=1./tan(a);
			break;
		case Operator::sinh:
			a=sinh(a);
			break;
		case Operator::cosh:
			a=cosh(a);
			break;
		case Operator::tanh:
			a=tanh(a);
			break;
		case Operator::sech:
			a=1.0/cosh(a);
			break;
		case Operator::csch:
			a=1.0/sinh(a);
			break;
		case Operator::coth:
			a=cosh(a)/sinh(a);
			break;
		case Operator::arcsin:
			a=asin(a);
			break;
		case Operator::arccos:
			a=acos(a);
			break;
		case Operator::arctan:
			a=acos(a);
			break;
		case Operator::arccot:
			a=log(a+pow(a*a+1., 0.5));
			break;
		case Operator::arcsinh:
			a=0.5*(log(1.+1./a)-log(1.-1./a));
			break;
		case Operator::arccosh:
			a=log(a+sqrt(a-1.)*sqrt(a+1.));
			break;
	// 	case Operator::arccsc:
	// 	case Operator::arccsch:
	// 	case Operator::arcsec:
	// 	case Operator::arcsech:
	// 	case Operator::arcsinh:
	// 	case Operator::arctanh:
		case Operator::exp:
			a=exp(a);
			break;
		case Operator::ln:
			a=log(a);
			break;
		case Operator::log:
			a=log10(a);
			break;
		case Operator::abs:
			a= a>=0. ? a : -a;
			break;
		//case Object::conjugate:
		//case Object::arg:
		//case Object::real:
		//case Object::imaginary:
		case Operator::floor:
			a=floor(a);
			break;
		case Operator::ceiling:
			a=ceil(a);
			break;
		case Operator::min:
			a= a < b? a : b;
			break;
		case Operator::max:
			a= a > b? a : b;
			break;
		case Operator::gt:
			a= a > b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::lt:
			a= a < b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::eq:
			a= a == b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::approx:
			a= fabs(a-b)<0.001? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::neq:
			a= a != b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::geq:
			a= a >= b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::leq:
			a= a <= b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::_and:
			a= a && b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::_not:
			a=!a;
			boolean = true;
			break;
		case Operator::_or:
			a= a || b? 1.0 : 0.0;
			boolean = true;
			break;
		case Operator::_xor:
			a= (a || b) && !(a&&b)? 1.0 : 0.0;
			boolean = true;
			break;
		case Operator::implies:
			a= (a && !b)? 0.0 : 1.0;
			boolean = true;
			break;
		case Operator::gcd: //code by michael cane aka kiko :)
			while (b > 0.) {
				residu = (int) floor(a) % (int) floor(b);
				a = b;
				b = residu;
			}
			break;
		case Operator::lcm: //code by michael cane aka kiko :)
			c=a*b;
			while (b > 0.) {
				residu = (int) floor(a) % (int) floor(b);
				a = b;
				b = residu;
			}
			a=(int)c/(int)a;
			break;
		case Operator::root:
			a = b==2.0 ? sqrt(a) : pow(a, 1.0/b);
			break;
		default:
			m_err << i18n("The operator <em>%1</em> has not been implemented", Operator::m_words[op]);
			break;
	}
	
	ret->setValue(a);
	ret->setBoolean(boolean);
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
	if(m_exp.isCorrect())
		m_exp.m_tree = simp(m_exp.m_tree);
}

void Analitza::levelOut(Container *c, Container *ob, QList<Object*>::iterator &pos)
{
	QList<Object*>::iterator it = ob->firstValue();
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
			root = new Cn(calc(root));
			delete aux;
		}
	} else if(root->isContainer()) {
		Container *c= (Container*) root;
		bool d;
		if(c->containerType()==Container::piecewise) {
			root=simpPiecewise(c);
		} else if(c->containerType()==Container::lambda) {
			*c->firstValue()=simp(*c->firstValue());
		} else if(c->containerType()==Container::apply) {
			QList<Object*>::iterator it;
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
							if(intr->firstOperator()==Operator::plus) {
								levelOut(c, intr, it);
								d=true;
							}
						}
						
						if((*it)->type() == Object::value) {
							Cn* n = (Cn*) (*it);
							if(n->value()==0.) //0+-exp=exp
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
					} else if(somed && c->isUnary() && c->firstOperator()==Operator::minus) {
						if(!lastdel) {
							Container *aux=c;
							root=*c->firstValue();
							*aux->firstValue()=0;
							delete aux;
							c=0;
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
					for(it = c->firstValue(); it!=c->m_params.end(); it++)
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
							cm->m_params.append(new Operator(Operator::times));
							cm->m_params.append(Expression::objectCopy(c->m_params[2]));
							cm->m_params.append(Expression::objectCopy(cp->m_params[2]));
							c->m_params[2] = cm;
							delete cp;
							c->m_params[2]=simp(c->m_params[2]);
						}
					}
				} break;
				case Operator::divide:
					for(it = c->firstValue(); it!=c->m_params.end(); it++)
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
					for(it = c->m_params.begin(); it!=c->m_params.end(); it++) {
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
						cDiff->m_params.append(new Operator(Operator::minus));
						cDiff->m_params.append(Expression::objectCopy(c->ulimit()));
						cDiff->m_params.append(Expression::objectCopy(c->dlimit()));
						
						Container *nc=new Container(Container::apply);
						nc->m_params.append(new Operator(Operator::times));
						nc->m_params.append(cDiff);
						nc->m_params.append(*it);
						
						c->m_params.last()=0;
						delete c;
						root=simp(nc);
					} else if((*it)->type()==Object::container)
					   root=simpSum(c);
					
					break;
				default:
					it = c->firstValue();
					
					for(; it!=c->m_params.end(); it++)
						*it = simp(*it);
					break;
			}
		} else {
			QList<Object*>::iterator it = c->firstValue();
					
			for(; it!=c->m_params.end(); it++)
				*it = simp(*it);
		}
	}
	return root;
}


Object* Analitza::simpScalar(Container * c)
{
	Cn value(0.), *aux;
	Operator o = c->firstOperator();
	bool d, changed=false, sign=true;
	
	QList<Object*>::iterator i = c->firstValue();
	
	for(; i!=c->m_params.end();) {
		d=false;
		
		if((*i)->type()==Object::value) {
			aux = (Cn*) *i;
// 			if(!changed && o.operatorType()==Operator::minus && i!=c->firstValue())
// 				aux->negate();
			
			if(changed)
				reduce(o.operatorType(), &value, *aux, false);
			else
				value=*aux;
			d=true;
		} /*else if((*i)->isContainer() && o.operatorType()==Operator::times) {
			Container *m = (Container*) *i;
			
			if(m->firstOperator()==Operator::minus && m->isUnary()) {
				Object* aux = Expression::objectCopy(*m->firstValue());
				delete *m->firstValue();
				*i = aux;
				sign = !sign;
				changed=true;
			}
		}*/
		
		if(d) {
			delete *i;
			changed=true;
			i = c->m_params.erase(i);
		} else
			++i;
	}
	
	if(changed) {
		bool found=false;
		i=c->firstValue();
		for(; !found && i!=c->m_params.end(); ++i) {
			if((*i)->type()==Object::container) {
				Container *c1 = (Container*) *i;
				if(c1->containerType()==Container::apply)
					found=true;
				
			} else if((*i)->type()==Object::value || (*i)->type()==Object::variable) {
				found=true;
			}
		}
		
		if(!sign)
			value.negate();
		
		if(found && value.value()!=0)
			switch(o.operatorType()) {
				case Operator::minus:
				case Operator::plus:
					c->m_params.append(new Cn(value));
					break;
				default:
					c->m_params.insert(c->firstValue(), new Cn(value));
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
		cint->m_params.append(new Operator(Operator::minus));
		cint->m_params.append(p.second);
		toAdd=cint;
	} else {
		Container *cint = new Container(Container::apply);
		cint->m_params.append(new Operator(o.multiplicityOperator()));
		cint->m_params.append(p.second);
		cint->m_params.append(new Cn(p.first));
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
	bool sign=true;
	QList<Object*>::const_iterator it(c->firstValue());
	for(; it!=c->m_params.constEnd(); ++it) {
		Object *o2=*it;
		QPair<double, Object*> imono;
		bool ismono=false;
		
		if(o2->type() == Object::container) {
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
		
		bool found = false;
		QList<QPair<double, Object*> >::iterator it1(monos.begin());
		for(; it1!=monos.end(); ++it1) {
			Object *o1=it1->second, *o2=imono.second;
			if(o2->type()!=Object::oper && Container::equalTree(o1, o2)) {
				found = true;
				break;
			}
		}
		
		if(found) {
			if(o.operatorType()==Operator::minus && it!=c->firstValue())
				imono.first= -imono.first;
			it1->first += imono.first;
		} else {
			imono.second = Expression::objectCopy(imono.second);
			monos.append(imono);
		}
	}
	
	delete c;
	c= new Container(Container::apply);
	
// 	qDeleteAll(c->m_params);
// 	c->m_params.clear();
	Object *root=c;
	if(monos.count()==1 && o.operatorType()!=Operator::minus) {
		delete c;
		c=0;
		root=createMono(o, monos[0]);
	} else {
		QList<QPair<double, Object*> >::iterator i=monos.begin();
		c->m_params.append(new Operator(o));
		for(; i!=monos.end(); ++i) {
			Object* toAdd=createMono(o, *i);
			
			if(toAdd)
				c->m_params.append(toAdd);
		}
	}
	
	if(!root) {
		delete c;
		root=new Cn(0.);
	}
	
	if(!sign) {
		Container *cn=new Container(Container::apply);
		cn->m_params.append(new Operator(Operator::minus));
		cn->m_params.append(root);
		root=cn;
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
		QList<Object*>::iterator it=cval->m_params.begin(), itEnd=cval->m_params.end(), firstV=cval->firstValue();
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
			nc->m_params.append(new Operator(Operator::times));
			nc->m_params << out;
			nc->m_params.append(c);
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
	QList<Object*>::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
	QList<Object*> newList;
	bool stop=false;
	for(; !stop && it!=itEnd; ++it) {
		Container *p=static_cast<Container*>(*it);
		Q_ASSERT( (*it)->type()==Object::container &&
				(p->containerType()==Container::piece || p->containerType()==Container::otherwise) );
		bool isPiece = p->containerType()==Container::piece;
				
		if(stop) {
			delete p;
			continue;
		}
				
		if(isPiece) {
			p->m_params[1]=simp(p->m_params[1]);
			if(p->m_params[1]->type()==Object::value) {
				Cn* cond=static_cast<Cn*>(p->m_params[1]);
				if(cond->isTrue()) {
					delete p->m_params[1];
					p->m_params.removeAt(1);
					p->setContainerType(Container::otherwise);
					isPiece=false;
					stop=true;
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

bool Analitza::hasVars(const Object *o, const QString &var, const QStringList& bvars)
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
		}	break;
		case Object::container: {
			Container *c = (Container*) o;
			bool firstFound=false;
			QList<Object*>::iterator it = c->m_params.begin(), first = c->firstValue();
			
			QStringList scope=bvars;
			
			for(; !r && it!=c->m_params.end(); it++) {
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
					r |= hasVars(*it, var, scope);
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
			
			QList<Object*>::iterator fval(cbody->firstValue());
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
			QList<Object*>::iterator it(c->firstValue());
			for(; it!=c->m_params.end(); ++it)
				*it = removeDependencies(*it);
		}
	}
	return o;
}

void Analitza::setVariables(Variables * v)//FIXME: Copy me!
{
	if(m_vars!=NULL)
		delete m_vars;
	m_vars = v;
}

Expression Analitza::derivative()
{
	m_err.clear();
	/* FIXME: Must support multiple bvars */
	Expression exp;
	if(m_exp.isCorrect()) {
		QStringList vars = bvarList();
		if(vars.empty())
			exp.m_tree = derivative("x", m_exp.m_tree);
		else
			exp.m_tree = derivative(vars.first(), m_exp.m_tree);
		exp.m_tree = simp(exp.m_tree);
	}
	return exp;
}

void Analitza::insertVariable(const QString & name, const Expression & value)
{
	m_vars->modify(name, value);
}

void Analitza::insertVariable(const QString & name, const Object * value)
{
	m_vars->modify(name, value);
}

bool Analitza::hasTheVar(const QStringList & vars, const Object * o)
{
	bool found=false;
	const Ci* cand;
	switch(o->type()) {
		case Object::container: {
			const Container *c=static_cast<const Container*>(o);
			QStringList bvars=c->bvarList(), varsCopy=vars;
			foreach(QString var, bvars) {
				if(varsCopy.contains(var))
					varsCopy.removeAll(var);
			}
			found=hasTheVar(varsCopy, c);
		}	break;
		case Object::variable:
			cand=static_cast<const Ci*>(o);
			if(vars.contains(cand->name()))
				found=true;
			break;
		default:
			found=false;
			break;
	}
	return found;
}

bool Analitza::hasTheVar(const QStringList & vars, const Container * c)
{
	bool found=false;
	if(c->containerType()==Container::apply) {
		QList<Object*>::const_iterator it=c->firstValue(), itEnd=c->m_params.constEnd();
		for(; !found && it!=itEnd; ++it) {
			if(hasTheVar(vars, *it))
				found=true;
		}
	}
	return found;
}

Cn Analitza::derivative(const QList< QPair<QString, double > >& values )
{
	//c++ numerical recipes p. 192. Only for f'
	Cn v1;
	QPair<QString, double> valp;
	//Image
	foreach(valp, values) {//TODO: it should be +-hh
		m_vars->stack(valp.first, valp.second);
	}
		
	v1=calc(m_exp.tree());
		
	foreach(valp, values)
		m_vars->destroy(valp.first);
	
	//Image+h
	double h=0.00000001;
	Cn v2;
	foreach(valp, values) {
// 		volatile double temp=valp.second+h;
// 		double hh=temp-valp.second;
		m_vars->stack(valp.first, valp.second+h);
	}
	
	v2=calc(m_exp.tree());
		
	foreach(valp, values)
		m_vars->destroy(valp.first);
	double ret=(v2.value()-v1.value())/h;
	return ret;
}


