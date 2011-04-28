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

#include "htmlexpressionwriter.h"
#include "value.h"
#include "operator.h"
#include "container.h"
#include "vector.h"
#include <QStringList>
#include <KLocale>
#include "list.h"
#include "variable.h"
#include "stringexpressionwriter.h"
#include "apply.h"
#include "analitzautils.h"

using namespace Analitza;

//we use the one in string*
QMap<Operator::OperatorType, QString> initOperators();

template <class T>
QStringList HtmlExpressionWriter::allValues(T it, const T& itEnd, ExpressionWriter* writer)
{
	QStringList elements;
	for(; it!=itEnd; ++it)
		elements += (*it)->visit(writer);
	
	return elements;
}


static const QMap<Operator::OperatorType, QString> s_operators=initOperators();

QString oper(const QString& op) { return i18nc("html representation of an operator", "<span class='op'>%1</span>", op); }
QString oper(const QChar& op) { return i18nc("html representation of an operator", "<span class='op'>%1</span>", op); }
QString keyword(const QString& op) { return i18nc("html representation of an operator", "<span class='keyword'>%1</span>", op); }

HtmlExpressionWriter::HtmlExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString HtmlExpressionWriter::accept(const Vector* vec)
{
	return keyword("vector ")+oper("{ ")+allValues<Vector::const_iterator>(vec->constBegin(), vec->constEnd(), this).join(QString(oper(", ")))+oper(" }");
}

QString HtmlExpressionWriter::accept(const List* vec)
{
	if(!vec->isEmpty() && vec->at(0)->type()==Object::value && static_cast<Cn*>(vec->at(0))->format()==Cn::Char) {
		return "<span class='string'>&quot;"+AnalitzaUtils::listToString(vec)+ "&quot;</span>";
	} else
		return keyword("list ")+oper("{ ")+allValues<List::const_iterator>(vec->constBegin(), vec->constEnd(), this).join(QString(oper(", ")))+oper(" }");
}

QString HtmlExpressionWriter::accept(const Cn* var)
{
	if(var->isBoolean())
		return "<span class='var'>"+QString(var->isTrue() ? "true" : "false")+"</span>";
	else
		return "<span class='num'>"+QString::number(var->value(), 'g', 12)+"</span>";
}

QString HtmlExpressionWriter::accept(const Analitza::Ci* var)
{
	return var->toHtml();
}

QString HtmlExpressionWriter::accept(const Analitza::Operator* o)
{
	return "<span class='func'>"+o->toString()+"</span>";
}

QString HtmlExpressionWriter::accept ( const Analitza::Apply* a )
{
	Operator op=a->firstOperator();
	QStringList ret;
	QString toret;
	QString bounds;
	
	if(a->ulimit() || a->dlimit()) {
		bounds += oper('=');
		if(a->dlimit())
			bounds += a->dlimit()->visit(this);
		bounds += oper("..");
		if(a->ulimit())
			bounds += a->ulimit()->visit(this);
	}
	else if(a->domain())
		bounds += oper('@')+a->domain()->visit(this);
	
	foreach(Object* o, a->m_params) {
		Object::ObjectType type=o->type();
		switch(type) {
			if(type == Object::oper)
				Q_ASSERT(false);
				break;
			case Object::variable:
				ret << static_cast<const Ci*>(o)->visit(this);
				break;
			case Object::apply: {
				Apply *c = (Apply*) o;
				QString s = c->visit(this);
				if(s_operators.contains(op.operatorType())) {
					Operator child_op = c->firstOperator();
					
					if(child_op.operatorType() && 
							StringExpressionWriter::weight(&op, c->countValues())>=StringExpressionWriter::weight(&child_op, c->countValues()))
						s=oper('(')+s+oper(')');
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
			n=oper('(')+n+oper(')');
		
		toret += n+oper('(') + ret.join(oper(", ")) + oper(')');
	} else if(ret.count()>1 && s_operators.contains(op.operatorType())) {
		toret += ret.join(oper(s_operators.value(op.operatorType())));
	} else if(ret.count()==1 && op.operatorType()==Operator::minus)
		toret += oper('-')+ret[0];
	else if(op.operatorType()==Operator::selector) {
		QString value = ret.takeLast();
		if(a->m_params.last()->isApply())
			value = oper('(')+value+oper(')');
		toret += value + oper('[')+ret.join(oper(", "))+oper(']');
	}else {
		QString bounding;
		QStringList bvars;
		foreach(const Ci* bvar, a->bvarCi())
			bvars += bvar->visit(this);
		
		if(!bounds.isEmpty() || !bvars.isEmpty()) {
			if(bvars.count()!=1) bounding +=oper('(');
			bounding += bvars.join(oper(", "));
			if(bvars.count()!=1) bounding +=oper(')');
			
			bounding = ':'+bounding +bounds;
		}
		
		toret += op.visit(this)+oper('(')+ret.join(oper(", "))+bounding+oper(')');
	}
	
	return toret;
}

QString HtmlExpressionWriter::accept(const Container* var)
{
	QStringList ret = allValues(var->constBegin(), var->constEnd(), this);
	
	
	QString toret;
	switch(var->containerType()) {
		case Container::declare:
			toret += ret.join(oper(":="));
			break;
		case Container::lambda: {
			QString last=ret.takeLast();
			QStringList bvars;
			foreach(const Ci* bvar, var->bvarCi())
				bvars += bvar->visit(this);
			
			if(bvars.count()!=1) toret +=oper('(');
			toret += bvars.join(", ");
			if(bvars.count()!=1) toret +=oper(')');
			toret += oper("->") + last;
		}	break;
		case Container::math:
			toret += ret.join(oper("; "));
			break;
		case Container::uplimit: //x->(n1..n2) is put at the same time
		case Container::downlimit:
			break;
		case Container::bvar:
			if(ret.count()>1) toret += oper('(');
			toret += ret.join(", ");
			if(ret.count()>1) toret += oper(')');
			break;
		case Container::piece:
			toret += ret[1]+oper(" ? ")+ret[0];
			break;
		case Container::otherwise:
			toret += oper("? ")+ret[0];
			break;
		default:
			toret += var->tagName()+oper(" { ")+ret.join(oper(", "))+oper(" }");
			break;
	}
	return toret;
}

QString HtmlExpressionWriter::accept(const CustomObject*)
{
	return "Custom Object";
}
