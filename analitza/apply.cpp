/*************************************************************************************
 *  Copyright (C) 2007-2010 by Aleix Pol <aleixpol@kde.org>                          *
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

#include "apply.h"
#include "expressionwriter.h"
#include <QStringList>
#include "variable.h"
#include "analitzautils.h"
#include "container.h"

using namespace Analitza;

Apply::Apply()
	: Object(apply), m_ulimit(0), m_dlimit(0), m_domain(0), m_op(0)
{}

Apply::~Apply()
{
	delete m_dlimit;
	delete m_ulimit;
	delete m_domain;
	delete m_op;
	qDeleteAll(m_bvars);
	qDeleteAll(m_params);
}

//TODO: Improve this
void Apply::appendBranch(Object *o)
{
	if(o->type()==Object::oper) {
		m_op = static_cast<Operator*>(o);
	} else if(o->isContainer()) {
		Container* c=static_cast<Container*>(o);
		switch(c->containerType()) {
			case Container::uplimit:
				Q_ASSERT(!m_ulimit);
				m_ulimit = c->m_params[0];
				c->m_params[0]=0;
				delete c;
				break;
			case Container::downlimit:
				Q_ASSERT(!m_dlimit);
				m_dlimit = c->m_params[0];
				c->m_params[0]=0;
				delete c;
				break;
			case Container::domainofapplication:
				Q_ASSERT(!m_domain);
				m_domain = c->m_params[0];
				c->m_params[0]=0;
				delete c;
				break;
			case Container::bvar:
				Q_ASSERT(c->m_params[0]->type()==variable);
				m_bvars += static_cast<Ci*>(c->m_params[0]);
				c->m_params[0]=0;
				delete c;
				break;
			default:
				m_params.append(o);
				break;
		}
	} else
		m_params.append(o);
}

QString Apply::visit(ExpressionWriter* exp) const
{
	return exp->accept(this);
}

QStringList Apply::bvarStrings() const
{
	QStringList ret;
	foreach(const Ci* bvar, m_bvars) {
		ret.append(bvar->name());
	}
	return ret;
}

bool Apply::operator==(const Apply& a) const
{
	bool eq=a.m_params.count()==m_params.count();
	
	for(int i=0; eq && i<m_params.count(); ++i) {
		Object *o=m_params[i], *o1=a.m_params[i];
		eq = eq && AnalitzaUtils::equalTree(o, o1);
	}
	return eq;
}

Apply* Apply::copy() const
{
	Apply* ret=new Apply;
	ret->m_dlimit = m_dlimit? m_dlimit->copy() : 0;
	ret->m_ulimit = m_ulimit? m_ulimit->copy() : 0;
	ret->m_domain = m_domain? m_domain->copy() : 0;
	ret->m_op = m_op? (Operator*) m_op->copy() : 0;
	
	foreach(const Ci* var, m_bvars)
		ret->m_bvars += (Ci*) var->copy();
	
	foreach(const Object* o, m_params)
		ret->m_params += o->copy();
	
	return ret;
}

bool Apply::decorate(const Object::ScopeInformation& scope)
{
	ScopeInformation newScope(scope);
	
	foreach(Ci* var, m_bvars) {
		Object** o=new Object*(0);
		var->setValue(o, true);
		newScope.insert(var->name(), o);
	}
	
	bool ret=false;
	ret |= m_ulimit ? m_ulimit->decorate(scope) : false;
	ret |= m_dlimit ? m_dlimit->decorate(scope) : false;
	ret |= m_domain ? m_domain->decorate(scope) : false;
	
	foreach(Object* o, m_params)
		ret |= o->decorate(newScope);
	
	return ret;
}

bool Apply::isCorrect() const
{
	int ops=firstOperator().nparams();
	return ops<0 || m_params.size()==ops;
}

bool Apply::matches(const Object* exp, QMap<QString, const Object*>* found) const
{
	if(Object::apply!=exp->type())
		return false;
	const Apply* c=(const Apply*) exp;
	if(m_params.count()!=c->m_params.count())
		return false;
	
	bool matching=true;
	Apply::const_iterator it, it2, itEnd=m_params.constEnd();
	for(it=m_params.constBegin(), it2=c->m_params.constBegin(); matching && it!=itEnd; ++it, ++it2)
	{
		matching &= (*it)->matches(*it2, found);
	}
	return matching;
}
