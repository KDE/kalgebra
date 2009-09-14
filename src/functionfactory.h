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

#ifndef FUNCTIONFACTORY_H
#define FUNCTIONFACTORY_H

#define REGISTER_FUNCTION(name) \
        static FunctionImpl* create##name(const Expression &exp, Variables* v) { return new name (exp, v); } \
        namespace { bool _##name=FunctionFactory::self()->registerFunction(name::supportedBVars(), create##name); }

#include <QMap>
#include <QStringList>

struct FunctionImpl;
class QStringList;
class Expression;
class Variables;

class FunctionFactory
{
	public:
		typedef FunctionImpl* (*registerFunc_fn)(const Expression&, Variables* );
		typedef QStringList Id;
		FunctionImpl* item(const Id& bvars, const Expression& exp, Variables* v) const;
		static FunctionFactory* self();
		bool registerFunction(const Id& bvars, registerFunc_fn);
		bool contains(const Id& bvars) const;
	private:
		static FunctionFactory* m_self;
		FunctionFactory() { Q_ASSERT(!m_self); m_self=this; }
		QMap<QString, registerFunc_fn> m_items;
};


#endif // FUNCTIONFACTORY_H
