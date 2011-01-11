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


#ifndef ABSTRACTEXPRESSIONTRANSFORMER_H
#define ABSTRACTEXPRESSIONTRANSFORMER_H

namespace Analitza
{

class Ci;
class Vector;
class Container;
class List;
class Apply;
class Object;

class AbstractExpressionTransformer
{
	public:
		virtual ~AbstractExpressionTransformer();
		
		virtual Object* walk(const Object* o);
		virtual Object* walkApply(const Apply* o);
		virtual Object* walkVariable(const Ci* o);
		virtual Object* walkList(const List* o);
		virtual Object* walkVector(const Vector* o);
		virtual Object* walkContainer(const Container* o);
};

}

#endif // ABSTRACTEXPRESSIONWALKER_H
