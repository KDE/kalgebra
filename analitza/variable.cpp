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

#include "variable.h"
#include "expressionwriter.h"
#include "analitzautils.h"

using namespace Analitza;

Ci::Ci(const QString& b)
	: Object(variable), m_name(b), m_function(false), m_owner(false), m_value(0)
{
	Q_ASSERT(!b.isEmpty());
}

Ci::~Ci()
{
	if(m_owner)
		delete m_value;
}

Object* Ci::copy() const
{
	Ci *c = new Ci(m_name);
	c->m_function = m_function;
	if(!m_owner) {
		c->m_value=m_value;
		c->m_owner=false;
	}
	return c;
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

bool Ci::matches(const Object* exp, QMap<QString, const Object*>* found) const
{
	bool ret=false;
	if(found->contains(m_name)) {
		const Object* v=found->value(m_name);
		if(v) { //If already been found
			ret=AnalitzaUtils::equalTree(exp, v);
		} else {
			found->insert(m_name, exp);
			ret=true;
		}
	} else {
		ret = (Object::variable==exp->type());
		if(ret) {
			ret = static_cast<const Ci*>(exp)->name()==m_name;
		}
	}
	return ret;
}

void Ci::setValue(Object** value, bool owner)
{
	if(m_owner)
		delete m_value;
	
	m_value=value;
	m_owner=owner;
}

bool Ci::decorate(const ScopeInformation& scope)
{
	ScopeInformation::const_iterator it=scope.find(m_name);
	bool found=it!=scope.end();
	if(found) {
		setValue(*it, false);
	}
	
	return !isDefined();
}
