/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "substituteexpression.h"
#include <QString>
#include "object.h"
#include "variable.h"
#include "apply.h"
#include "pushvalue.h"

using namespace Analitza;

Object* SubstituteExpression::run(const Object* pattern, const QMap<QString, const Object*>& values)
{
	m_values=values;
	
	return walk(pattern);
}

QString SubstituteExpression::solveRename(const QString& name) const
{
	return m_renames.contains(name) ? m_renames[name] : name;
}

Object* SubstituteExpression::walkApply(const Apply* pattern)
{
	Apply* ret = new Apply;
	Apply::const_iterator it=pattern->firstValue(), itEnd=pattern->constEnd();
	ret->ulimit()=walk(pattern->ulimit());
	ret->dlimit()=walk(pattern->dlimit());
	ret->domain()=walk(pattern->domain());
	
	PushValue<QStringList> v(m_bvars, m_bvars);
	QVector<Ci*> bvars = pattern->bvarCi();
	foreach(Ci* bvar, bvars) {
		Ci* nbvar = bvar->copy();
		QString name = bvar->name();
		
		if(m_values.contains(name) && !m_renames.contains(name)) {
			const Object* val = m_values.value(bvar->name());
			Q_ASSERT(val->type()==Object::variable);
			QString newname = static_cast<const Ci*>(val)->name();
			m_renames.insert(name, newname);
		}
		
		nbvar->setName(solveRename(name));
		ret->addBVar(nbvar);
		m_bvars.append(nbvar->name());
	}
	
	if(pattern->firstOperator().isCorrect()) {
		Operator op = pattern->firstOperator();
		ret->appendBranch(walk(&op));
	}
	
	for(; it!=itEnd; ++it)
		ret->appendBranch(walk(*it));
	
	return ret;
}

Object* SubstituteExpression::walkVariable(const Ci* pattern)
{
	QString name = solveRename(pattern->name());
	QMap<QString, const Object*>::const_iterator it = m_values.constFind(name);
	
	if(it!=m_values.constEnd() /*&& !m_bvars.contains(name)*/)
		return it.value()->copy();
	else
		return pattern->copy(); //Error?
}
