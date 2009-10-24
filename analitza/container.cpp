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
#include <KLocale>
#include "variable.h"

char Container::m_typeStr[][10] = {
		"none",
		"math",
		"apply",
		"declare",
		"lambda",
		"bvar",
		"uplimit",
		"downlimit",
		"piece",
		"piecewise",
		"otherwise" };

QMap<QString, Container::ContainerType> createNameToType()
{
	QMap<QString, Container::ContainerType> ret;
	ret["apply"]=Container::apply;
	ret["declare"]=Container::declare;
	ret["math"]=Container::math;
	ret["lambda"]=Container::lambda;
	ret["bvar"]=Container::bvar;
	ret["uplimit"]=Container::uplimit;
	ret["downlimit"]=Container::downlimit;
	ret["piecewise"]=Container::piecewise;
	ret["piece"]=Container::piece;
	ret["otherwise"]=Container::otherwise;
	
	return ret;
}

QMap<QString, Container::ContainerType> Container::m_nameToType=createNameToType();

Container::Container(const Container& c) : Object(Object::container), m_cont_type(c.m_cont_type)
{
	Q_ASSERT(c.type()==Object::container);
	
	Container::const_iterator it=c.m_params.constBegin();
	for(; it!=c.m_params.constEnd(); ++it) {
		appendBranch((*it)->copy());
	}
}

Container* Container::copy() const
{
	return new Container(*this);
}

Operator Container::firstOperator() const
{
	foreach(const Object* o, m_params) {
		if(o->type()==Object::oper)
			return *static_cast<const Operator*>(o);
	}
	
	return Operator(Operator::function);
}

QString Container::visit(ExpressionWriter* e) const
{
	return e->accept(this);
}

void Container::negate()
{
	foreach(Object* o, m_params) {
		o->negate();
	}
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
	QList<Object*>::const_iterator it, itEnd=firstValue();
	
	for(it=m_params.constBegin(); it!=itEnd; ++it) {
		if((*it)->isContainer()) {
			Container* c = (Container*) (*it);
			if(c->containerType() == Container::bvar && !c->m_params.isEmpty() && c->m_params[0]->type()==Object::variable)
				ret.append((Ci*) c->m_params[0]);
		}
	}
	
	return ret;
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

Object* Container::ulimit() const
{
	Container* c=extractType(uplimit);
	if(c)
		return c->m_params.first();
	else
		return 0;
}

Object* Container::dlimit() const
{
	Container* c=extractType(downlimit);
	if(c)
		return c->m_params.first();
	else
		return 0;
}

bool Container::operator==(const Container& c) const
{
	bool eq=c.m_params.count()==m_params.count();
	
	for(int i=0; eq && i<m_params.count(); ++i) {
		Object *o=m_params[i], *o1=c.m_params[i];
		eq = eq && equalTree(o, o1);
	}
	return eq;
}

//TODO: Deprecate that
bool Container::equalTree(const Object * o1, const Object * o2)
{
	Q_ASSERT(o1 && o2);
	if(o1==o2)
		return true;
	else if(o1->type()!=o2->type())
		return false;
	
	bool eq;
	switch(o2->type()) {
		case Object::variable:
			eq = *static_cast<const Ci*>(o1)==*static_cast<const Ci*>(o2);
			break;
		case Object::value:
			eq = *static_cast<const Cn*>(o1)==*static_cast<const Cn*>(o2);
			break;
		case Object::container:
			eq = *static_cast<const Container*>(o1)==*static_cast<const Container*>(o2);
			break;
		case Object::oper:
			eq = *static_cast<const Operator*>(o1)==*static_cast<const Operator*>(o2);
			break;
		case Object::vector:
			eq = *static_cast<const Vector*>(o1)==*static_cast<const Vector*>(o2);
			break;
		case Object::list:
			eq = *static_cast<const List*>(o1)==*static_cast<const List*>(o2);
			break;
		case Object::none:
			eq=false;
			Q_ASSERT(false && "Should not get here");
			break;
	}
	return eq;
}

bool Container::isNumber() const
{
	return m_cont_type==apply || m_cont_type==math || m_cont_type==lambda || m_cont_type==declare ||
		m_cont_type==piecewise || m_cont_type==piece || m_cont_type==otherwise;
}

bool isValue(Object* o)
{
	bool ret=false;
	switch(o->type()) {
		case Object::value:
		case Object::variable:
		case Object::vector:
		case Object::list:
			ret=true;
			break;
		case Object::container:
			if(((Container*) o)->isNumber())
				ret=true;
			break;
		case Object::oper:
		case Object::none:
			break;
	}
	return ret;
}

Container::iterator Container::firstValue()
{
	QList<Object *>::iterator it(m_params.begin()), itEnd(m_params.end());
	for(; it!=itEnd; ++it) {
		if(isValue(*it))
			break;
	}
	
	return it;
}

Container::const_iterator Container::firstValue() const
{
	QList<Object *>::const_iterator it(m_params.constBegin()), itEnd(m_params.constEnd());
	for(; it!=itEnd; ++it) {
		if(isValue(*it))
			break;
	}
	
	return it;
}

bool Container::isUnary() const
{
	QList<Object*>::const_iterator it(firstValue());
	return ++it==m_params.end();
}

bool Container::isCorrect() const
{
	bool ret=m_type==Object::container && m_cont_type!=none;
	if(m_cont_type==Container::apply) {
		Operator o=firstOperator();
		ret &= o.nparams()<0 || countValues()==o.nparams();
	}
	return ret;
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

int Container::countValues() const
{
	Container::const_iterator it=firstValue(), itEnd=m_params.constEnd();
	int count=0;
	
	for(; it!=itEnd; ++it)
		count++;
	
	return count;
}

Container* Container::extractType(Container::ContainerType t) const
{
	for(Container::const_iterator it=m_params.begin(); it!=m_params.end(); ++it) {
		Container *c = (Container*) (*it);
		if(c->isContainer() && c->containerType()==t)
			return c;
	}
	return 0;
}

void Container::appendBranch(Object* o)
{
	m_params.append(o);
}

///Rename to decorate
bool Container::decorate(const ScopeInformation& scope)
{
	if(m_cont_type==bvar)
		return false;
	
	Container::const_iterator it=m_params.constBegin(), itEnd=m_params.constEnd();
	if((*it)->type()==Object::oper)
		++it;
	
	ScopeInformation newScope(scope);
	QList<Ci*> bvars=bvarCi();
	if(m_cont_type==declare) {
		Q_ASSERT((*it)->type()==variable);
		bvars.append((Ci*) *it);
	}
	
	foreach(Ci* var, bvars) {
		Object** o=new Object*(0);
		var->setValue(o, true);
		newScope.insert(var->name(), o);
		++it;
	}
	
	bool ret=false;
	//We need to use the parent scope for (up/down)limit
	for(; it!=itEnd;) {
		Container *c = (Container*) (*it);
		
		if(c->isContainer() && (c->containerType()==uplimit ||  c->containerType()==downlimit)) {
			ret |= (*it)->decorate(scope);
			++it;
		} else
			break;
	}
	
	for(; it!=itEnd; ++it) {
		ret |= (*it)->decorate(newScope);
	}
	
	return ret;
}
