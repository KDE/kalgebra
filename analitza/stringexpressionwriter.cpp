/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "stringexpressionwriter.h"
#include "value.h"
#include "vector.h"
#include "operator.h"
#include "container.h"
#include <QStringList>
#include "list.h"
#include "variable.h"
#include "apply.h"
#include "analitzautils.h"

using namespace Analitza;

template <class T>
QStringList StringExpressionWriter::allValues(T it, const T& itEnd, ExpressionWriter* writer)
{
	QStringList elements;
	for(; it!=itEnd; ++it)
		elements += (*it)->visit(writer);
	
	return elements;
}


QMap<Operator::OperatorType, QString> initOperators()
{
	QMap<Operator::OperatorType, QString> ret;
	ret.insert(Operator::plus, "+");
	ret.insert(Operator::times, "*");
	ret.insert(Operator::divide, "/");
	ret.insert(Operator::eq, "=");
	ret.insert(Operator::neq, "!=");
	ret.insert(Operator::lt, "<");
	ret.insert(Operator::leq, "<=");
	ret.insert(Operator::gt, ">");
	ret.insert(Operator::geq, ">=");
	ret.insert(Operator::power, "^");
	ret.insert(Operator::minus, "-");
	return ret;
}

const QMap<Operator::OperatorType, QString> StringExpressionWriter::s_operators=initOperators();

StringExpressionWriter::StringExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString StringExpressionWriter::accept(const Ci* var)
{
	return var->name();
}

QString StringExpressionWriter::accept(const Operator* op)
{
	return op->name();
}

QString StringExpressionWriter::accept(const Vector* vec)
{
	return QString("vector { %1 }").arg(allValues<Vector::const_iterator>(vec->constBegin(), vec->constEnd(), this).join(QString(", ")));
}

QString StringExpressionWriter::accept(const List* vec)
{
	return QString("list { %1 }").arg(allValues<List::const_iterator>(vec->constBegin(), vec->constEnd(), this).join(QString(", ")));
}

QString StringExpressionWriter::accept(const Cn* var)
{
	if(var->isBoolean())
		return var->isTrue() ? "true" : "false";
	else
		return QString::number(var->value(), 'g', 12);
}

int StringExpressionWriter::weight(const Operator* op, int size)
{
	switch(op->operatorType()) {
		case Operator::lt:
		case Operator::gt:
		case Operator::eq:
		case Operator::neq:
		case Operator::leq:
		case Operator::geq:
			return 1;
		case Operator::plus:
			return 2;
		case Operator::minus:
			return size==1 ? 8 : 3;
		case Operator::times:
			return 4;
		case Operator::divide:
			return 5;
		case Operator::_and:
		case Operator::_or:
		case Operator::_xor:
			return 6;
		case Operator::power:
			return 7;
		default:
			return 1000;
	}
}

QString StringExpressionWriter::accept ( const Analitza::Apply* a )
{
	Operator op=a->firstOperator();
	QStringList ret;
	QString toret;
	QString bounds;
	QStringList bvars=a->bvarStrings();
	
	if(a->ulimit() || a->dlimit()) {
		bounds += '=';
		if(a->dlimit())
			bounds += a->dlimit()->visit(this);
		bounds += "..";
		if(a->ulimit())
			bounds += a->ulimit()->visit(this);
	}
	else if(a->domain())
		bounds += '@'+a->domain()->visit(this);
	
	foreach(Object* o, a->m_params) {
		Object::ObjectType type=o->type();
		switch(type) {
			case Object::oper:
				Q_ASSERT(false);
				break;
			case Object::variable:
				ret << static_cast<const Ci*>(o)->visit(this);
				break;
			case Object::apply: {
				const Apply *c = (const Apply*) o;
				QString s = c->visit(this);
				if(s_operators.contains(op.operatorType()) && !c->isUnary()) {
					Operator child_op = c->firstOperator();
					
					if(child_op.operatorType() && weight(&op, c->countValues())>=weight(&child_op, c->countValues()))
						s=QString("(%1)").arg(s);
				}
				ret << s;
			}	break;
			default:
				ret << o->visit(this);
				break;
		}
	}
	
	bool func=op.operatorType()==Operator::function;
	if(func) {
		QString n = ret.takeFirst();
		if(a->m_params.first()->type()!=Object::variable)
			n='('+n+')';
		
		toret += QString("%1(%2)").arg(n).arg(ret.join(", "));
	} else if(ret.count()>1 && s_operators.contains(op.operatorType())) {
		toret += ret.join(s_operators.value(op.operatorType()));
	} else if(ret.count()==1 && op.operatorType()==Operator::minus)
		toret += '-'+ret[0];
	else {
		QString bounding;
		if(!bounds.isEmpty() || !bvars.isEmpty()) {
			if(bvars.count()!=1) bounding +='(';
			bounding += bvars.join(", ");
			if(bvars.count()!=1) bounding +=')';
			
			bounding = ':'+bounding +bounds;
		}
			
		toret += QString("%1(%2%3)").arg(op.visit(this)).arg(ret.join(", ")).arg(bounding);
	}
	
	return toret;
}

QString StringExpressionWriter::accept(const Container* var)
{
	QStringList ret = allValues(var->constBegin(), var->constEnd(), this);
	
	QString toret;
	switch(var->containerType()) {
		case Container::declare:
			toret += ret.join(":=");
			break;
		case Container::lambda: {
			QString last=ret.takeLast();
			QStringList bvars = var->bvarStrings();
			if(bvars.count()!=1) toret +='(';
			toret += bvars.join(", ");
			if(bvars.count()!=1) toret +=')';
			toret += "->" + last;
		}	break;
		case Container::math:
			toret += ret.join("; ");
			break;
		case Container::uplimit: //x->(n1..n2) is put at the same time
		case Container::downlimit:
			break;
		case Container::bvar:
			if(ret.count()>1) toret += '(';
			toret += ret.join(", ");
			if(ret.count()>1) toret += ')';
			break;
		case Container::piece:
			toret += ret[1]+" ? "+ret[0];
			break;
		case Container::otherwise:
			toret += "? "+ret[0];
			break;
		default:
			toret += var->tagName()+" { "+ret.join(", ")+" }";
			break;
	}
	return toret;
}
