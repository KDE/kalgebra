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

using namespace Analitza;

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
	QStringList elements;
	for(Vector::const_iterator it=vec->constBegin(); it!=vec->constEnd(); ++it)
	{
		elements += (*it)->visit(this);
	}
	return QString("vector { %1 }").arg(elements.join(QString(", ")));
}

QString StringExpressionWriter::accept(const List* vec)
{
	QStringList elements;
	for(List::const_iterator it=vec->constBegin(); it!=vec->constEnd(); ++it)
	{
		elements += (*it)->visit(this);
	}
	return QString("list { %1 }").arg(elements.join(QString(", ")));
}

QString StringExpressionWriter::accept(const Cn* var)
{
	if(var->isBoolean()) {
		if(var->isTrue())
			return "true";
		else
			return "false";
	} else
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

QString StringExpressionWriter::accept(const Container* var)
{
	QStringList ret;
	Operator *op=0;
	QString bounds;
	QStringList bvars;
	
	Container::const_iterator it=var->firstValue();
	for(int i=0; i<var->m_params.count(); i++) {
		Q_ASSERT(var->m_params[i]);
		
		Object::ObjectType type=var->m_params[i]->type();
		if(type == Object::oper)
			op = (Operator*) var->m_params[i];
		else if(type == Object::variable) {
			Ci *b = (Ci*) var->m_params[i];
			ret << b->visit(this);
		} else if(type == Object::container) {
			Container *c = (Container*) var->m_params[i];
			QString s = c->visit(this);
			
			if(op && c->containerType()==Container::apply && s_operators.contains(op->operatorType())) {
				Operator child_op = c->firstOperator();
				
				if(child_op.operatorType() && weight(op, var->countValues())>=weight(&child_op, c->countValues()))
					s=QString("(%1)").arg(s);
				
			}
			
			if(c->containerType() == Container::bvar) { //bvar
				Object *ul = var->ulimit(), *dl = var->dlimit(), *domain=var->domain();
				if(dl || ul) {
					bounds += '=';
					if(dl)
						bounds += dl->visit(this);
					bounds += "..";
					if(ul)
						bounds += ul->visit(this);
				}
				else if(domain)
					bounds += "@"+domain->visit(this);
				
				bvars += s;
			}
			else if(c->containerType()!=Container::uplimit && c->containerType()!=Container::downlimit && c->containerType()!=Container::domainofapplication)
				ret << s;
		} else 
			ret << var->m_params[i]->visit(this);
	}
	bool func=!op || (op->operatorType()==Operator::function);
	
	QString toret;
	switch(var->containerType()) {
		case Container::declare:
			toret += ret.join(":=");
			break;
		case Container::lambda: {
			QString last=ret.takeLast();
			if(bvars.count()!=1) toret +='(';
			toret += bvars.join(", ");
			if(bvars.count()!=1) toret +=')';
			if(!bounds.isEmpty()) toret+='='+bounds;
			toret += "->" + last;
		}	break;
		case Container::math:
			toret += ret.join("; ");
			break;
		case Container::apply:
			if(func){
				QString n = ret.takeFirst();
				if(var->m_params.first()->type()!=Object::variable)
					n='('+n+')';
				
				toret += QString("%1(%2)").arg(n).arg(ret.join(", "));
			} else if(op==0)
				toret += ret.join(" ");
			else if(ret.count()>1 && s_operators.contains(op->operatorType())) {
				toret += ret.join(s_operators.value(op->operatorType()));
			} else if(ret.count()==1 && op->operatorType()==Operator::minus) {
				toret += '-'+ret[0];
			} else {
				QString bounding;
				if(!bounds.isEmpty() || !bvars.isEmpty()) {
					if(bvars.count()!=1) bounding +='(';
					bounding += bvars.join(", ");
					if(bvars.count()!=1) bounding +=')';
					
					bounding = ':'+bounding +bounds;
				}
					
				toret += QString("%1(%2%3)").arg(op->visit(this)).arg(ret.join(", ")).arg(bounding);
			}
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
