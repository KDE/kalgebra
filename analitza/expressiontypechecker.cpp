/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "expressiontypechecker.h"
#include "variable.h"
#include "variables.h"
#include "list.h"
#include "vector.h"
#include "container.h"
#include "expression.h"
#include "analitzautils.h"
#include "operations.h"

namespace Analitza
{

QDebug operator<<(QDebug dbg, const ExpressionType &c)
{
	dbg.nospace() << "("<< qPrintable(c.toString()) <<")";
	
	return dbg.space();
}

QString ExpressionType::toString() const
{
	QString ret;
	switch(type) {
		case ExpressionType::Value:
			ret="num";
			break;
		case ExpressionType::List:
			ret='['+contained->toString()+']';
			break;
		case ExpressionType::Vector:
			ret='<'+contained->toString()+'>';
			break;
		case ExpressionType::Error:
			ret="err";
			break;
	}
	
	return ret;
}


ExpressionTypeChecker::ExpressionTypeChecker(const Expression& exp, Variables* v)
	: m_v(v), m_exp(exp)
{}

ExpressionType ExpressionTypeChecker::check()
{
	current=ExpressionType(ExpressionType::Error);
	
	m_exp.tree()->decorate(AnalitzaUtils::variablesScope(m_v));
	m_exp.tree()->visit(this);
	return current;
}

QString ExpressionTypeChecker::accept(const Operator* o)
{
	if(parameters.size()==1) {
		current=Analitza::Operations::typeUnary(o->operatorType(), parameters.first());
	} else {
		QList<ExpressionType>::const_iterator it=parameters.constBegin()+1, itEnd=parameters.constEnd();
		current=parameters.first();
		
		for(; it!=itEnd; ++it)
			current = Analitza::Operations::type(o->operatorType(), current, *it);
	}
	
	return QString();
}

QString ExpressionTypeChecker::accept(const Ci* var)
{
	if(var->isDefined() && var->value()) {
		var->value()->visit(this);
	} else {
		current=m_typeForBVar[var->name()];
	}
	return QString();
}

QString ExpressionTypeChecker::accept(const Cn* val)
{
	current=ExpressionType(Analitza::ExpressionType::Value);
	return QString();
}

Container* ExpressionTypeChecker::lambdaFor(Object* o)
{
	if(o->type()==Object::variable) {
		return lambdaFor(static_cast<Ci*>(o)->value());
	} else if(o->type()==Object::container) {
		Q_ASSERT(((Container*) o)->containerType()==Container::lambda);
		return (Container*) o;
	}
	
	Q_ASSERT(false);
	return 0;
}

//1. Check if parameters are applied correctly
//2. Return the operator result type
QString ExpressionTypeChecker::accept(const Container* c)
{
	parameters.clear();
	
	switch(c->containerType()) {
		case Container::apply: {
			Operator o=c->firstOperator();
			Container::const_iterator it=c->firstValue(), itEnd=c->constEnd();
			if(o.operatorType()==Operator::function)
				++it;
			
			if(o.isBounded()) {
				Object* ul=c->ulimit();
				if(ul) {
					ul->visit(this);
					m_typeForBVar[c->bvarStrings().first()]=current;
				}
			}
			
			for(; it!=itEnd; ++it) {
				(*it)->visit(this);
				
				parameters += current;
			}
			
			switch(o.operatorType()) {
				case Operator::none:
				case Operator::sum:
				case Operator::product:
					(*c->firstValue())->visit(this);
					break;
				case Operator::function: {
					Container* operation=lambdaFor(c->m_params.first());
	// 				qDebug() << "calling " << c->toString() << operation->toString();
					
					QStringList names=operation->bvarStrings();
					Container::const_iterator it=c->firstValue()+1, itEnd=c->constEnd();
					
					for(int i=0; it!=itEnd; ++it, ++i) {
						m_typeForBVar[names[i]]=parameters[i];
					}
					
	// 				qDebug() << "vars" << m_typeForBVar;
					operation->visit(this);
				} break;
				default:
					o.visit(this);
					break;
			}
		}	break;
		case Container::lambda: {
			c->m_params.last()->visit(this);
		}	break;
		case Container::math:
		case Container::none:
		case Container::downlimit:
		case Container::uplimit:
		case Container::piecewise:
		case Container::otherwise:
		case Container::piece:
		case Container::declare:
		case Container::bvar:
		case Container::domainofapplication:
			(*c->firstValue())->visit(this);
			break;
	}
	
	return QString();
}

QString ExpressionTypeChecker::accept(const Vector* v)
{
	v->at(0)->visit(this);
	
	ExpressionType t(ExpressionType::Vector);
	t.contained=new ExpressionType(current);
	
	current=t;
	
	return QString();
}

QString ExpressionTypeChecker::accept(const List* l)
{
	l->at(0)->visit(this);
	
	ExpressionType t(ExpressionType::List);
	t.contained=new ExpressionType(current);
	
	current=t;
	
	return QString();
}

}