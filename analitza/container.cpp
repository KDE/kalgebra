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
#include "container.h"

#include "expression.h"
#include "expressionwriter.h"
#include "vector.h"
#include "value.h"
#include "list.h"

#include <KDebug>
#include "localize.h"
#include "variable.h"
#include "analitzautils.h"

using namespace Analitza;

char Container::m_typeStr[][20] = {
		"none",
		"math",
		"declare",
		"lambda",
		"bvar",
		"uplimit",
		"downlimit",
		"piece",
		"piecewise",
		"otherwise",
		"domainofapplication"
};

QMap<QString, Container::ContainerType> createNameToType()
{
	QMap<QString, Container::ContainerType> ret;
	ret["declare"]=Container::declare;
	ret["math"]=Container::math;
	ret["lambda"]=Container::lambda;
	ret["bvar"]=Container::bvar;
	ret["uplimit"]=Container::uplimit;
	ret["downlimit"]=Container::downlimit;
	ret["piecewise"]=Container::piecewise;
	ret["piece"]=Container::piece;
	ret["otherwise"]=Container::otherwise;
	ret["domainofapplication"]=Container::domainofapplication;
	
	return ret;
}

QMap<QString, Container::ContainerType> Container::m_nameToType=createNameToType();

Container::Container(const Container& c) : Object(Object::container), m_cont_type(c.m_cont_type)
{
	Q_ASSERT(c.type()==Object::container);
	
	foreach(const Object* o, c.m_params)
		appendBranch(o->copy());
}

Container* Container::copy() const
{
	return new Container(*this);
}

QString Container::visit(ExpressionWriter* e) const
{
	return e->accept(this);
}

bool Container::isZero() const
{
	bool a=true;
	foreach(const Object* o, m_params) {
		a = a && o->isZero();
	}
	return a;
}

Container::ContainerType Container::toContainerType(const QString& tag)
{
	return m_nameToType[tag];
}

QList<Ci*> Container::bvarCi() const
{
	QList<Ci*> ret;
	QList<Object*>::const_iterator it, itEnd=m_params.constEnd();
	
	for(it=m_params.constBegin(); it!=itEnd; ++it) {
		if((*it)->isContainer()) {
			Container* c = (Container*) (*it);
			if(c->containerType() == Container::bvar) {
				Q_ASSERT(!c->isEmpty() && c->m_params[0]->type()==Object::variable);
				ret.append((Ci*) c->m_params[0]);
			}
		}
	}
	
	return ret;
}

int Container::bvarCount() const
{
	int r=0;
	QList<Object*>::const_iterator it, itEnd=m_params.constEnd();
	for(it=m_params.constBegin(); it!=itEnd; ++it) {
		Object* o = *it;
		if(o->isContainer() && static_cast<Container*>(o)->containerType() == Container::bvar)
			r++;
	}
	
	return r;
}

QStringList Container::bvarStrings() const
{
	QStringList bvars;
	QList< Ci* > vars=bvarCi();
	foreach(Ci* var, vars) {
		bvars.append(var->name());
	}
	
	return bvars;
}

bool Container::operator==(const Container& c) const
{
	bool eq=c.m_params.count()==m_params.count();
	
	for(int i=0; eq && i<m_params.count(); ++i) {
		Object *o=m_params[i], *o1=c.m_params[i];
		eq = eq && AnalitzaUtils::equalTree(o, o1);
	}
	return eq;
}

bool Container::isNumber() const
{
	return m_cont_type==math || m_cont_type==lambda || m_cont_type==declare ||
		m_cont_type==piecewise || m_cont_type==piece || m_cont_type==otherwise;
}

QString Container::tagName() const
{
	return QString(m_typeStr[m_cont_type]);
}

bool Container::matches(const Object* exp, QMap<QString, const Object*>* found) const
{
	if(Object::container!=exp->type())
		return false;
	const Container* c=(const Container*) exp;
	if(m_params.count()!=c->m_params.count())
		return false;
	
	bool matching=true;
	Container::const_iterator it, it2, itEnd=m_params.constEnd();
	for(it=m_params.constBegin(), it2=c->m_params.constBegin(); matching && it!=itEnd; ++it, ++it2)
	{
		matching &= (*it)->matches(*it2, found);
	}
	return matching;
}

const Container* Container::extractType(Container::ContainerType t) const
{
	for(Container::const_iterator it=m_params.constBegin(); it!=m_params.constEnd(); ++it) {
		const Container *c = (const Container*) (*it);
		if(c->isContainer() && c->containerType()==t)
			return c;
	}
	return 0;
}

void Container::appendBranch(Object* o)
{
	m_params.append(o);
}
