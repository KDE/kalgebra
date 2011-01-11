/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "providederivative.h"
#include "apply.h"
#include "substituteexpression.h"
#include "expression.h"
#include "analitzautils.h"
#include "value.h"
#include "container.h"
#include "variable.h"
#include <KLocalizedString>
#include "list.h"
#include "vector.h"

using namespace Analitza;
using namespace AnalitzaUtils;

QList<QPair<const Object*, const Object*> > ProvideDerivative::s_transformations;

const Object* parse(const QString& exp)
{
	Expression e(exp);
// 	if(!e.isCorrect()) qDebug() << "lelele" << exp << e.error();
	Q_ASSERT(e.isCorrect());
	Object* tree = e.tree();
	e.setTree(0);
	
	//We remove the math node
	Container* root = static_cast<Container*>(tree);
	
	Q_ASSERT(root->m_params.size()==1);
	tree=root->m_params.takeFirst();
	delete root;
	
	return tree;
}

ProvideDerivative::ProvideDerivative(const QString& var) : var(var)
{
	if(s_transformations.isEmpty()) {
		s_transformations += qMakePair(parse("diff(x:x)"), parse("1"));
		s_transformations += qMakePair(parse("diff(sin(p):x)"), parse("diff(p:x)*cos(p)"));
		s_transformations += qMakePair(parse("diff(cos(p):x)"), parse("diff(p:x)*(-sin p)"));
		s_transformations += qMakePair(parse("diff(tan(p):x)"), parse("diff(p:x)/(cos(p)**2)"));
		s_transformations += qMakePair(parse("diff(f/g:x)"), parse("(diff(f:x)*g-f*diff(g:x))/g**2"));
		s_transformations += qMakePair(parse("diff(ln(p):x)"), parse("diff(p:x)/p"));
		s_transformations += qMakePair(parse("diff(log(p):x)"), parse("diff(p:x)/(ln(10)*p)"));
	}
}

Object* ProvideDerivative::run(const Object* o)
{
	Object* ret = walk(makeDiff(o));
	return ret;
}

Object* ProvideDerivative::walkApply(const Apply* a)
{
	if(a->firstOperator().operatorType()==Operator::diff) {
		Object* val = *a->firstValue();
		if(!hasTheVar(QSet<QString>() << var, val))
			return new Cn(0.);
		
		foreach(const Transformation& t, s_transformations) {
			QMap<QString, const Object*> matchedValues;
			bool match = t.first->matches(a, &matchedValues);
			
// 			qDebug() << "beeeeee" << a->toString() << t.first->toString() << match;
			if(match) {
				SubstituteExpression exp;
				Object* obj=exp.run(t.second, matchedValues);
// 				qDebug() << "match!";
				return walk(obj);
			}
		}
		Object* ret = 0;
		if(val->isApply()) ret=derivativeApply(static_cast<Apply*>(val));
		else if(val->isContainer()) ret=derivativeContainer(static_cast<Container*>(val));
		else if(val->type()==Object::list) ret=derivateContentList(static_cast<const List*>(val));
		else if(val->type()==Object::vector) ret=derivateContentVector(static_cast<const Vector*>(val));
		
		if(!ret) {
			ret = a->copy();
			m_errors += i18n("Could not calculate the derivative for '%1'", ret->toString());
		}
		return ret;
	} else
		return AbstractExpressionTransformer::walkApply(a);
}

#define ITERATE(T, args...)\
Object* ProvideDerivative::derivateContent##T(const T * v)\
{\
	T* ret = new T(args);\
	T::const_iterator it=v->constBegin(), itEnd=v->constEnd();\
	for(; it!=itEnd; ++it) {\
		ret->appendBranch(walk(makeDiff(*it)));\
	}\
	return ret;\
}

ITERATE(List);
ITERATE(Vector, v->size());

Object* ProvideDerivative::derivativeApply(const Apply* c)
{
	Operator op = c->firstOperator();
	switch(op.operatorType()) {
		case Operator::minus:
		case Operator::plus: {
			Apply *r= new Apply;
			r->appendBranch(new Operator(op));
			
			Container::const_iterator it(c->firstValue());
			for(; it!=c->constEnd(); ++it)
				r->appendBranch(walk(makeDiff(*it)));
			return r;
		} break;
		case Operator::times: {
			Apply *nx = new Apply;
			nx->appendBranch(new Operator(Operator::plus));
			
			Apply::const_iterator it(c->firstValue());
			for(; it!=c->constEnd(); ++it) {
				Apply *neach = new Apply;
				neach->appendBranch(new Operator(Operator::times));
				
				Apply::const_iterator iobj(c->firstValue());
				for(; iobj!=c->constEnd(); ++iobj) {
					Object* o;
					if(iobj==it)
						o=walk(makeDiff(*iobj));
					else
						o=(*iobj)->copy();
					
					neach->appendBranch(o);
				}
				nx->appendBranch(neach);
			}
			return nx;
		} break;
		case Operator::power: {
			if(hasTheVar(QSet<QString>() << var, c->m_params[1])) {
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
				nChain1->appendBranch(walk(makeDiff(*c->firstValue())));
				
				Apply *cDiv = new Apply;
				cDiv->appendBranch(new Operator(Operator::divide));
				cDiv->appendBranch((*(c->firstValue()+1))->copy());
				cDiv->appendBranch((*c->firstValue())->copy());
				nChain1->appendBranch(cDiv);
				
				Apply *nChain2 = new Apply;
				nChain2->appendBranch(new Operator(Operator::times));
				nChain2->appendBranch(walk(makeDiff(*(c->firstValue()+1))));
				
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
				cx->appendBranch(walk(makeDiff(*c->firstValue())));
				
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
		default:
			break;
	}
	return 0;
}

Object* ProvideDerivative::derivativeContainer(const Container *c)
{
	if(c->containerType()==Container::lambda) {
		//TODO REVIEW
		return walk(makeDiff(c->m_params.last()));
	} else if(c->containerType()==Container::piecewise) {
		Container *newPw = new Container(Container::piecewise);
		
		foreach(Object* o, c->m_params) {
			Q_ASSERT(o->isContainer());
			Container *p = (Container *) o;
			Container *np = new Container(p->containerType());
			
			np->m_params += walk(makeDiff(p->m_params[0]));
			if(p->m_params.size()>1)
				np->m_params += p->m_params[1]->copy();
			newPw->appendBranch(np);
		}
		return newPw;
	} else if(c->containerType()==Container::declare) {
		Container* obj = new Container(Container::declare);
		obj->appendBranch(c->m_params.first()->copy());
		obj->appendBranch(walk(makeDiff(c->m_params.last())));
		return obj;
	} else {
		Container *cret = new Container(c->containerType());
		foreach(Object* o, c->m_params) {
			cret->appendBranch(walk(makeDiff(o)));
		}
		
		return cret;
	}
	return 0;
}

Object* ProvideDerivative::makeDiff(const Object* o) const
{
	Apply* a = new Apply;
	a->appendBranch(new Operator(Operator::diff));
	a->appendBranch(o->copy());
	a->addBVar(new Ci(var));
	
	return a;
}

