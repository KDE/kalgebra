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

#include "object.h"
#include "container.h"
#include "expressionwriter.h"
#include "stringexpressionwriter.h"

QString Object::toString() const
{
	StringExpressionWriter e(this);
	return e.result();
}

Ci::Ci(const Object * o) : Object(o->type())
{
	Q_ASSERT(m_type==Object::variable);
	const Ci *c = (Ci*) o;
	m_name = c->name();
	m_function = c->m_function;
}

QString Ci::visit(ExpressionWriter* e) const
{
	return e->accept(this);
}

QString Ci::toMathML() const
{
	if(m_function)
		return QString("<ci type='function'>%1</ci>").arg(m_name);
	else
		return QString("<ci>%1</ci>").arg(m_name);
}

Object::ValueType Object::valueType() const
{
	switch(m_type) {
		case Object::value:
			return Real;
		case Object::container: {
			const Container *c=(const Container*) this;
			if(c->containerType()==Container::vector)
				return Vector;
			break;
		}
		default:
			break;
	}
	return Null;
}
