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
#include <klocalizedstring.h>

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
			ret='<'+contained->toString()+','+QString::number(size)+'>';
			break;
		case ExpressionType::Error:
			ret="err";
			break;
	}
	
	return ret;
}

bool ExpressionType::operator==(const Analitza::ExpressionType& t) const
{
	return t.type==type && t.size==size && ((!t.contained && !contained) || (t.contained && contained && *t.contained==*contained));
}

ExpressionTypeChecker::ExpressionTypeChecker(Variables* v)
	: m_v(v)
{}

ExpressionType ExpressionTypeChecker::check(const Expression& _exp)
{
	Expression exp(_exp);
	current=ExpressionType(ExpressionType::Error);
	
	exp.tree()->decorate(AnalitzaUtils::variablesScope(m_v));
	exp.tree()->visit(this);
	return current;
}

QString ExpressionTypeChecker::accept(const Operator* o)
{
	if(parameters.size()==1) {
		current=Analitza::Operations::typeUnary(o->operatorType(), parameters.first());
	} else {
		QList<ExpressionType>::const_iterator it=parameters.constBegin()+1, itEnd=parameters.constEnd();
		current=parameters.first();
		
		for(; it!=itEnd; ++it) {
			current = Analitza::Operations::type(o->operatorType(), current, *it);
			if(current.type==ExpressionType::Error)
				m_err += i18n("Cannot operate '%1'", o->toString() ); //TODO: Improve error message
		}
		
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
	
	if(current.type==ExpressionType::Error)
		m_err += i18n("Cannot check '%1' type", var->name());
	return QString();
}

QString ExpressionTypeChecker::accept(const Cn*)
{
	current=ExpressionType(Analitza::ExpressionType::Value);
	return QString();
}

const Container* ExpressionTypeChecker::lambdaFor(const Object* o)
{
	if(o->type()==Object::variable) {
		return lambdaFor(static_cast<const Ci*>(o)->value());
	} else if(o->type()==Object::container) {
		Q_ASSERT(((const Container*) o)->containerType()==Container::lambda);
		return (const Container*) o;
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
// 					qDebug() << "peee" << current << c->dlimit()->toString() << c->ulimit()->toString();
					
					typeIs(c->dlimit(), ExpressionType(current));
					
// 					qDebug() << "taaa" << current;
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
					const Container* operation=lambdaFor(c->m_params.first());
	// 				qDebug() << "calling " << c->toString() << operation->toString();
					
					QStringList names=operation->bvarStrings();
					Container::const_iterator it=c->firstValue()+1, itEnd=c->constEnd();
					
					for(int i=0; it!=itEnd; ++it, ++i) {
						m_typeForBVar[names[i]]=parameters[i];
					}
					
	// 				qDebug() << "vars" << m_typeForBVar;
					operation->visit(this);
// 					qDebug() << "peeeeee" << current << m_err;
				} break;
				default:
					o.visit(this);
					break;
			}
		}	break;
		case Container::piecewise:
			c->m_params.first()->visit(this);
			typeIs(c->m_params.constBegin(), c->m_params.constEnd(), current); //condition check
			
			break;
		case Container::piece:
			typeIs(c->m_params.last(), ExpressionType(ExpressionType::Value)); //condition check
			c->m_params.first()->visit(this); //we return the body
			break;
		case Container::lambda:
		case Container::otherwise:
		case Container::math:
		case Container::none:
		case Container::downlimit:
		case Container::uplimit:
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
	t.size=v->size();
	
	typeIs(v->constBegin(), v->constEnd(), *t.contained);
	current=t;
	
	return QString();
}

QString ExpressionTypeChecker::accept(const List* l)
{
	l->at(0)->visit(this);
	
	ExpressionType t(ExpressionType::List);
	t.contained=new ExpressionType(current);
	
	typeIs(l->constBegin(), l->constEnd(), *t.contained);
	
	current=t;
	
	return QString();
}

void ExpressionTypeChecker::typeIs(Vector::const_iterator it,
			const Vector::const_iterator& itEnd, const ExpressionType& type )
{
	for(; it!=itEnd; ++it)
		typeIs(*it, type);
}

void ExpressionTypeChecker::typeIs(List::const_iterator it,
			const List::const_iterator& itEnd, const ExpressionType& type )
{
	for(; it!=itEnd; ++it)
		typeIs(*it, type);
}

void ExpressionTypeChecker::typeIs(const Object* o, const ExpressionType& type)
{
	o->visit(this);
	
	if(current!=type)
		m_err += i18n("Cannot convert %1 to %2", type.toString(), current.toString());
}

}