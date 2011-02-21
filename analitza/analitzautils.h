/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
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

#ifndef ANALITZAUTILS_H
#define ANALITZAUTILS_H

#include <QStringList>
#include "object.h"
#include "analitzaexport.h"

namespace Analitza
{
class Object;
class Container;
class Variables;
class Expression;
class Apply;
}

namespace AnalitzaUtils
{
	bool hasTheVar(const QSet<QString>& vars, const Analitza::Object * o);
	bool hasTheVar(const QSet<QString>& vars, const Analitza::Apply * a);
	bool hasTheVar(const QSet<QString>& vars, const Analitza::Container * c);
	
	bool isLambda(const Analitza::Object* o);
	QStringList dependencies(const Analitza::Object* o, const QStringList& scope);
	
	/** Returns whether there is any variable in the @p o tree.
			@p bvars tells the already defined variables (which won't return true). */
	bool hasVars(const Analitza::Object* o,
					const QStringList& bvars=QStringList());
	
	/** Returns whether @p o1 is equal to @p o2. */
	bool equalTree(const Analitza::Object * o1, const Analitza::Object * o2);
	
	/** Prints an expression tree from a node @p o. @p prefix will be the start of every item line */
	void objectWalker(const Analitza::Object* o, const QByteArray& prefix=QByteArray());
	
	/** Convenience function */
	void ANALITZA_EXPORT objectWalker(const Analitza::Expression& o, const QByteArray& prefix=QByteArray());
	
	/** Creates a QVariant out of an Expression @p res */
	QVariant ANALITZA_EXPORT expressionToVariant(const Analitza::Expression& res);
	
	/** Creates an expression out of a QVariant @p v, it will assert if it's not the correct type. */
	Analitza::Expression ANALITZA_EXPORT variantToExpression(const QVariant& v);
}

#endif // ANALITZAUTILS_H
