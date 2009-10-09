/*************************************************************************************
 *  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "vector.h"
#include "expression.h"
#include "expressionwriter.h"
#include "container.h"

Vector::Vector(const Vector& v)
	: Object(Object::vector)//, m_elements(v.m_elements.size())
{
	foreach(const Object* o, v.m_elements)
	{
		m_elements.append(o->copy());
	}
}

Vector::~Vector()
{
	qDeleteAll(m_elements);
}

Vector::Vector(int size)
	: Object(Object::vector)//, m_elements(size)
{}

Object* Vector::copy() const
{
	Vector *v=new Vector(size());
// 	m_elements.reserve(v->m_elements.size());
	foreach(const Object* o, m_elements)
	{
		v->m_elements.append(o->copy());
	}
	return v;
}

void Vector::appendBranch(Object* o)
{
	m_elements.append(o);
}

QString Vector::visit(ExpressionWriter* e) const
{
	return e->accept(this);
}

bool Vector::isCorrect() const
{
	bool corr = !m_elements.isEmpty();
	foreach(const Object* o, m_elements) {
		corr |= o->isCorrect();
	}
	return corr;
}

void Vector::negate()
{
	foreach(Object* o, m_elements) {
		o->negate();
	}
}

bool Vector::isZero() const
{
	foreach(const Object* o, m_elements) {
		if(!o->isZero())
			return false;
	}
	return true;
}

bool Vector::matches(const Object* exp, QMap< QString, const Object* >* found) const
{
	if(Object::vector!=exp->type())
		return false;
	const Vector* c=(const Vector*) exp;
	if(m_elements.count()!=c->m_elements.count())
		return false;
	
	bool matching=true;
	Vector::const_iterator it, it2, itEnd=m_elements.constEnd();
	for(it=m_elements.constBegin(), it2=c->m_elements.constBegin(); matching && it!=itEnd; ++it, ++it2)
	{
		matching &= (*it)->matches(*it2, found);
	}
	return matching;
}

bool Vector::operator==(const Vector& v) const
{
	bool eq=v.size()==size();
	
	for(int i=0; eq && i<m_elements.count(); ++i) {
		eq = eq && Container::equalTree(m_elements[i], v.m_elements[i]);
	}
	return eq;
}

bool Vector::decorate(const QMap< QString, Object** >& scope)
{
	bool ret=false;
	foreach(Object* o, m_elements) {
		ret |= o->decorate(scope);
	}
	return ret;
}
