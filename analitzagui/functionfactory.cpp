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

#include "functionfactory.h"
#include "analitza/expressiontype.h"

using Analitza::Expression;
using Analitza::Variables;

FunctionFactory* FunctionFactory::m_self=0;

bool FunctionFactory::contains(const FunctionFactory::Id& bvars) const
{
	return m_items.contains(bvars.join("|"));
}

FunctionImpl* FunctionFactory::item(const Id& bvars, const Expression& exp, Variables* v) const
{
	return m_items[bvars.join("|")](exp, v);
}

Analitza::ExpressionType FunctionFactory::type(const FunctionFactory::Id& bvars)
{
	return m_types[bvars.join("|")]();
}

bool FunctionFactory::registerFunction(const Id& bvars, registerFunc_fn f, expectedType_fn ft, const QStringList& examples)
{
	Q_ASSERT(!contains(bvars));
	m_items[bvars.join("|")]=f;
	m_types[bvars.join("|")]=ft;
	m_examples.append(examples);
	
	return true;
}

QStringList FunctionFactory::examples() const
{
	return m_examples;
}

FunctionFactory* FunctionFactory::self()
{
	if(!m_self)
		m_self=new FunctionFactory;
	return m_self;
}
