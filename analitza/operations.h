/*************************************************************************************
 *  Copyright (C) 2007-2008 by Aleix Pol <aleixpol@gmail.com>                        *
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
class Vector;

class Operations
{
	public:
		static Object* reduce(Operator::OperatorType op, Object* oper, Object* oper1, QString &correct);
		static Object* reduceUnary(Operator::OperatorType op, Object* oper, QString &correct);
		
	private:
		static Cn* reduceRealReal(Operator::OperatorType op, Cn *oper, const Cn* oper1, QString &correct);
		static Cn* reduceUnaryReal(Operator::OperatorType op, Cn *oper, QString &correct);
		
		static Object* reduceRealVector(Operator::OperatorType op, Cn *oper, Vector* vector, QString &correct);
		static Object* reduceVectorReal(Operator::OperatorType op, Vector* vector, Cn *oper, QString &correct);
		static Object* reduceVectorVector(Operator::OperatorType op, Vector* v1, Vector* v2, QString &correct);
		static Object* reduceUnaryVector(Operator::OperatorType op, Vector* c, QString &correct);
};

#endif
