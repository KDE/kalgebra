/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include "mathmlexpressionwritter.h"
#include "value.h"
#include "operator.h"
#include "container.h"
#include <QStringList>

MathMLExpressionWritter::MathMLExpressionWritter(const Object* o)
{
	m_result=o->visit(this);
}

QString MathMLExpressionWritter::accept(const Ci* var)
{
	return var->name();
}

QString MathMLExpressionWritter::accept(const Operator* op)
{
	return QString("<%1 />").arg(op->name());;
}

QString MathMLExpressionWritter::accept(const Cn* val)
{
	if(val->isBoolean()) {
		if(val->isTrue())
			return "<cn type='constant'>true</cn>";
		else
			return "<cn type='constant'>false</cn>";
	} else
		return QString("<cn>%1</cn>").arg(val->value(), 0, 'g', 12);

}

QString MathMLExpressionWritter::accept(const Container* c)
{
	QString ret;
	QList<Object*>::const_iterator i;
	for(i=c->m_params.constBegin(); i!=c->m_params.constEnd(); ++i) {
		if(*i==0)
			ret += "error;";
		else
			ret += (*i)->visit(this);
	}
	
	return QString("<%1>%2</%1>").arg(c->tagName()).arg(ret);
}
