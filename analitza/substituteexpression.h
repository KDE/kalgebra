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


#ifndef SUBSTITUTEEXPRESSION_H
#define SUBSTITUTEEXPRESSION_H

#include <QMap>
#include <QStringList>
#include "abstractexpressiontransformer.h"

namespace Analitza
{

class Object;
class Ci;
class Cn;
class Container;
class Vector;
class List;
class Apply;

class SubstituteExpression : public AbstractExpressionTransformer
{
	public:
		Object* run(const Object* pattern, const QMap<QString, const Object*>& values);
		
	private:
		Object* walkApply(const Apply* pattern);
		Object* walkVariable(const Ci* pattern);
		
		QString solveRename(const QString& name) const;
		
		QMap<QString, const Object*> m_values;
		QMap<QString, QString> m_renames;
		QStringList m_bvars;
};

}
#endif // SUBSTITUTEEXPRESSION_H
