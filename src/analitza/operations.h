/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "operator.h"

#include <QStringList>

class Cn;
class Container;

class Operations
{
	public:
		enum ValueType { Null=0, Real, Vector };
		
		QStringList errors() const { return m_err; }
		
		static ValueType valueType(const Object* obj);
		
		static Object* reduce(Operator::OperatorType op, Object* oper, Object* oper1);
		static Object* reduceUnary(Operator::OperatorType op, Object* oper);
		
	private:
		static Cn* reduceRealReal(Operator::OperatorType op, Cn *oper, const Cn* oper1);
		static Cn* reduceUnaryReal(Operator::OperatorType op, Cn *oper);
		
		static Object* reduceRealVector(Operator::OperatorType op, Cn *oper, Container* vector);
		static Object* reduceVectorReal(Operator::OperatorType op, Container* vector, Cn *oper);
		static Object* reduceVectorVector(Operator::OperatorType op, Container* v1, Container* v2);
		static Object* reduceUnaryVector(Operator::OperatorType op, Container* c);
		
		QStringList m_err;
};

#endif
