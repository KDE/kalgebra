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

#include "mathmlexpressionwriter.h"
#include "value.h"
#include "vector.h"
#include "operator.h"
#include "container.h"
#include <QStringList>
#include "list.h"
#include "variable.h"
#include "apply.h"
#include "explexer.h"
#include "analitzautils.h"

using namespace Analitza;

MathMLExpressionWriter::MathMLExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString MathMLExpressionWriter::accept(const Ci* var)
{
	QString attrib;
	if(var->isFunction())
		attrib=" type='function'";
	return "<ci"+attrib+'>'+var->name()+"</ci>";
}

QString MathMLExpressionWriter::accept(const Operator* op)
{
	if(op->operatorType()==Operator::function)
		return QString();
	else
		return QString("<%1 />").arg(op->name());
}

QString MathMLExpressionWriter::accept(const Cn* val)
{
	if(val->isBoolean()) {
		if(val->isTrue())
			return "<cn type='constant'>true</cn>";
		else
			return "<cn type='constant'>false</cn>";
	} else {
		QString type;
		if(val->format()==Cn::Real)
			type = " type='real'";
		
		return QString("<cn%1>%2</cn>").arg(type).arg(val->value(), 0, 'g', 12);
	}
}

QString MathMLExpressionWriter::accept(const Vector* vec)
{
	QStringList elements;
	for(Vector::const_iterator it=vec->constBegin(); it!=vec->constEnd(); ++it)
	{
		elements += (*it)->visit(this);
	}
	return QString("<vector>%1</vector>").arg(elements.join(QString()));
}

QString MathMLExpressionWriter::accept(const List* vec)
{
	QStringList elements;
	if(vec->size()==0)
		return "<list />";
	else if(vec->at(0)->type()==Object::value && static_cast<const Cn*>(vec->at(0))->format()==Cn::Char) {
		QString ret=AnalitzaUtils::listToString(vec);
		ret = ExpLexer::escape(ret);
		ret = "<cs>"+ret+"</cs>";
		return ret;
	} else {
		for(List::const_iterator it=vec->constBegin(); it!=vec->constEnd(); ++it)
			elements += (*it)->visit(this);
		
		return QString("<list>%1</list>").arg(elements.join(QString()));
	}
}

QString MathMLExpressionWriter::accept(const Container* c)
{
	QString ret;
	foreach(const Object* o, c->m_params)
		ret += o->visit(this);
	
	return QString("<%1>%2</%1>").arg(c->tagName()).arg(ret);
}

QString MathMLExpressionWriter::accept(const Apply* a)
{
	QString ret;
	
	ret += a->firstOperator().visit(this);
	foreach(const Ci* bvar, a->bvarCi())
		ret += "<bvar>"+bvar->visit(this)+"</bvar>";
	if(a->ulimit()) ret += "<uplimit>"+a->ulimit()->visit(this)+"</uplimit>";
	if(a->dlimit()) ret += "<downlimit>"+a->dlimit()->visit(this)+"</downlimit>";
	if(a->domain()) ret += "<domainofapplication>"+a->domain()->visit(this)+"</domainofapplication>";
	
	foreach(const Object* o, a->m_params)
		ret += o->visit(this);
	
	return QString("<apply>%1</apply>").arg(ret);
}

QString MathMLExpressionWriter::accept(const Analitza::CustomObject*)
{
	return "<!-- custom object -->";
}
