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

bool Apply::addBranch(Object* o)
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
				return false;
		}
	} else
		return false;
	
	return true;
}

void Apply::appendBranch(Object *o)
{
	if(!addBranch(o))
		m_params.append(o);
}

void Apply::prependBranch(Object* o)
{
	if(!addBranch(o))
		m_params.prepend(o);
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
	bool eq=a.m_params.count()==m_params.count() && a.firstOperator()==firstOperator();
	eq &= bool(a.ulimit())==bool(m_ulimit);
	eq &= bool(a.dlimit())==bool(m_dlimit);
	eq &= bool(a.domain())==bool(m_domain);
	
	if(m_ulimit) eq &= AnalitzaUtils::equalTree(m_ulimit, a.m_ulimit);
	if(m_dlimit) eq &= AnalitzaUtils::equalTree(m_dlimit, a.m_dlimit);
	if(m_domain) eq &= AnalitzaUtils::equalTree(m_domain, a.m_domain);
	
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
	ret->m_op     = m_op    ? m_op->copy()     : 0;
	
	foreach(const Ci* var, m_bvars)
		ret->m_bvars += (Ci*) var->copy();
	
	foreach(const Object* o, m_params)
		ret->m_params += o->copy();
	
	return ret;
}

bool Apply::matches(const Object* exp, QMap<QString, const Object*>* found) const
{
	if(Object::apply!=exp->type())
		return false;
	const Apply* c=(const Apply*) exp;
	if(m_params.count()!=c->m_params.count())
		return false;
	
	QVector<Ci*> vars=m_bvars, cvars = c->bvarCi();
	bool matching=vars.size()==cvars.size();
	matching &= bool(m_op)==bool(c->m_op) && (!m_op || m_op->matches(c->m_op, found));
	
	for(QVector<Ci*>::const_iterator it=vars.constBegin(), cit=cvars.constBegin(), itEnd=vars.constEnd(); matching && it!=itEnd; ++it) {
		matching &= (*it)->matches(*cit, found);
	}
	
// 	bool matching=true;
	Apply::const_iterator it, it2, itEnd=m_params.constEnd();
	for(it=m_params.constBegin(), it2=c->m_params.constBegin(); matching && it!=itEnd; ++it, ++it2)
		matching &= (*it)->matches(*it2, found);
	
	matching &= bool(m_ulimit)==bool(c->m_ulimit) && (!m_ulimit || m_ulimit->matches(c->m_ulimit, found));
	matching &= bool(m_dlimit)==bool(c->m_dlimit) && (!m_dlimit || m_dlimit->matches(c->m_dlimit, found));
	matching &= bool(m_domain)==bool(c->m_domain) && (!m_domain || m_domain->matches(c->m_domain, found));
	return matching;
}

bool Analitza::Apply::hasBoundings() const
{
	return m_dlimit || m_ulimit || m_domain;
}

void Apply::addBVar(Ci* bvar)
{
	m_bvars += bvar;
}
