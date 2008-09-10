/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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

#ifndef EXPRESSIONWRITTER_H
#define EXPRESSIONWRITTER_H

#include <QString>

#include "analitzaexport.h"
#include "object.h"

class Ci;
class Cn;
class Container;
class Operator;

/**
 *	This class represents an expression writter.
 *
 *	@author Aleix Pol <aleixpol@gmail.com>
 */

class ExpressionWritter
{
	public:
		virtual ~ExpressionWritter();
		
		virtual QString accept(const Operator* var) = 0;
		virtual QString accept(const Ci* var) = 0;
		virtual QString accept(const Cn* var) = 0;
		virtual QString accept(const Container* var) = 0;
		
		virtual QString result() const=0;
};

#endif
