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

#include "builtinmethods.h"
#include "expression.h"

using namespace Analitza;

PointerFunctionDefinition::PointerFunctionDefinition(func call)
	: m_function(call)
{}

Expression PointerFunctionDefinition::operator()(const QList< Expression >& args)
{
	return m_function(args);
}

BuiltinMethods::~BuiltinMethods()
{
	qDeleteAll(m_functions);
}

void BuiltinMethods::insertFunction(const QString& id, const ExpressionType& type, FunctionDefinition* f)
{
	if(m_types.contains(id))
		qDebug() << "Replacing a builtin function called: " << id;
	
	m_types.insert(id, type);
	m_functions.insert(id, f);
}
