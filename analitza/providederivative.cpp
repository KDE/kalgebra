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
#include <QtCore/qsharedpointer.h>

using namespace Analitza;
using namespace AnalitzaUtils;

struct Transformation
{
	Transformation(const Object* first, const Object* second)
		: first(first), second(second)
	{}
	
	typedef bool (*treeCheck)(const Object* o);
	
	Transformation(const Object* first, const Object* second, const QMap<QString, treeCheck>& conditions)
		: first(first), second(second), conditions(conditions)
	{}
	
	Object* applyTransformation(const Object* input) const {
		QMap<QString, const Object*> matchedValues;
		bool match = first->matches(input, &matchedValues);
		
// 		qDebug() << "beeeeee" << a->toString() << t.first->toString() << match;
		if(match) {
			bool satisfied=true;
			for(QMap<QString, treeCheck>::const_iterator it=conditions.constBegin(), itEnd=conditions.constEnd(); satisfied && it!=itEnd; ++it) {
				Q_ASSERT(matchedValues.contains(it.key()));
				const Object* value = matchedValues.value(it.key());
				
				satisfied = it.value()(value);
			}
			
			if(satisfied) {
				SubstituteExpression exp;
				Object* obj=exp.run(second.data(), matchedValues);
// 				qDebug() << "match!";
				return obj;
			}
		}
		return 0;
	}
	
	QSharedPointer<const Object> first, second;
	QMap<QString, treeCheck> conditions;
};


QList<Transformation> s_transformations;

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

bool independentTree(const Object* o)
{
	return !hasVars(o);
}

ProvideDerivative::ProvideDerivative(const QString& var) : var(var)
{
	if(s_transformations.isEmpty()) {
		QMap<QString, Transformation::treeCheck> nat;
		nat.insert("Real", independentTree);
		
		s_transformations += Transformation(parse("diff(x:x)"), parse("1"));
		s_transformations += Transformation(parse("diff(sin(p):x)"), parse("diff(p:x)*cos(p)"));
		s_transformations += Transformation(parse("diff(cos(p):x)"), parse("diff(p:x)*(-sin p)"));
		s_transformations += Transformation(parse("diff(tan(p):x)"), parse("diff(p:x)/(cos(p)**2)"));
		s_transformations += Transformation(parse("diff(f/g:x)"), parse("(diff(f:x)*g-f*diff(g:x))/g**2"));
		s_transformations += Transformation(parse("diff(ln(p):x)"), parse("diff(p:x)/p"));
		s_transformations += Transformation(parse("diff(log(p):x)"), parse("diff(p:x)/(ln(10)*p)"));
		s_transformations += Transformation(parse("diff(f**Real:x)"), parse("Real*diff(f:x)*f**(Real-1)"), nat); //this is just a simplification, should be deprecated
		s_transformations += Transformation(parse("diff(f**g:x)"), parse("f**g*(diff(g:x)*ln f+g/f*diff(f:x))"));
	}
}

Object* ProvideDerivative::run(const Object* o)
{
	Apply* a=makeDiff(const_cast<Object*>(o));
	Object* ret = walk(a);
	clearDiff(a);
	return ret;
}

Object* ProvideDerivative::walkApply(const Apply* a)
{
	if(a->firstOperator().operatorType()==Operator::diff) {
		Object* val = *a->firstValue();
		if(!hasTheVar(QSet<QString>() << var, val))
			return new Cn(0.);
		
		foreach(const Transformation& t, s_transformations) {
			Object* newTree = t.applyTransformation(a);
			if(newTree) {
				Object* ret=walk(newTree);
				delete newTree;
				return ret;
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

#define ITERATE(T, ...)\
Object* ProvideDerivative::derivateContent##T(const T * v)\
{\
	T* ret = new T(__VA_ARGS__);\
	T::const_iterator it=v->constBegin(), itEnd=v->constEnd();\
	for(; it!=itEnd; ++it) {\
		Apply* a=makeDiff(*it);\
		ret->appendBranch(walk(a));\
		clearDiff(a);\
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
			for(; it!=c->constEnd(); ++it) {
				Apply* a=makeDiff(*it);
				r->appendBranch(walk(a));
				clearDiff(a);
			}
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
					if(iobj==it) {
						Apply* a=makeDiff(*iobj);
						o=walk(a);
						clearDiff(a);
					} else
						o=(*iobj)->copy();
					
					neach->appendBranch(o);
				}
				nx->appendBranch(neach);
			}
			return nx;
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
			
			Apply* a=makeDiff(p->m_params[0]);
			np->m_params += walk(a);
			clearDiff(a);
			if(p->m_params.size()>1)
				np->m_params += p->m_params[1]->copy();
			newPw->appendBranch(np);
		}
		return newPw;
	} else if(c->containerType()==Container::declare) {
		Q_ASSERT(false);
		return c->copy();
	} else {
		Container *cret = new Container(c->containerType());
		foreach(Object* o, c->m_params) {
			Apply* a=makeDiff(o);
			cret->appendBranch(walk(a));
			clearDiff(a);
		}
		
		return cret;
	}
	return 0;
}

Apply* ProvideDerivative::makeDiff(Object* o) const
{
	Apply* a = new Apply;
	a->appendBranch(new Operator(Operator::diff));
	a->appendBranch(o);
	a->addBVar(new Ci(var));
	
	return a;
}

void ProvideDerivative::clearDiff(Apply* a)
{
	*a->firstValue()=0;
	delete a;
}
