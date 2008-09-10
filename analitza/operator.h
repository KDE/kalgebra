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

#ifndef OPERATOR_H
#define OPERATOR_H

#include "object.h"
#include "analitzaexport.h"

/** The Operator class is the operator representation in the trees. */
class ANALITZA_EXPORT Operator : public Object
{
	public:
		/** Specifies the type of an operator */
		enum OperatorType {
			none=0,
			plus, times, minus, divide, quotient,
			power, root, factorial,
			_and, _or, _xor, _not,
			gcd, lcm, rem, factorof,
			max, min,
			lt, gt, eq, neq, leq, geq, implies,
			approx, abs, floor, ceiling,
			sin, cos, tan,
			sec, csc, cot,
			sinh, cosh, tanh,
			sech, csch, coth,
			arcsin, arccos, arctan,
			arccot, arccoth,
			arccosh, arccsc, arccsch,
			arcsec, arcsech, arcsinh, arctanh,
			exp, ln, log,
   			real,
// 			conjugate, arg, real, imaginary,
			sum, product, diff,
			card, scalarproduct, selector,
			function, nOfOps
		};
		/** Constructor. Creates an operator with @p t type .*/
		explicit Operator(OperatorType t) : Object(oper), m_optype(t) {}
		
		/** Constructor. Creates an operator copy of the @p o object. If it is not an object a onone object is created. */
		explicit Operator(Object const *o);
		
		/** Destructor */
		virtual ~Operator() {}
		
		/** Sets an operator type to the object */
		void setOperator(enum OperatorType t) { m_optype=t; }
		
		/** Returns the number of params expected for the operator type. */
		int nparams() const { return nparams(m_optype); }
		
		/** Returns the operator type. */
		enum OperatorType operatorType() const { return m_optype; }
		
		/** Returns this operator precedence. */
		unsigned int weight() const { return operatorWeight(m_optype); }
		
		/** Returns the string expression representation of the operator. */
		QString name() const;
		
		/** Returns whether @p o is equal to this operator. */
		bool operator==(const Operator& o) const { return m_optype==o.m_optype; }
		
		/** Returns whether @p o is equal to this operator. */
		bool operator==(OperatorType o) const { return m_optype==o; }
		
		/** Makes this operator equal to @p a. */
		Operator operator=(const Operator &a) { setType(a.type()); m_optype=a.operatorType(); return *this;}
		
		/** Returns the multiplicity operator. e.g. 5+5+5+5=5*4 -> times is the multiplicityOperator of plus. */
		OperatorType multiplicityOperator() const { return multiplicityOperator(m_optype); }
		
		/** Returns if it is a trigonometric operator. */
		bool isTrigonometric() const { return isTrigonometric(m_optype); }
		
		/** Returns whether this operator needs bounding. */
		bool isBounded() const;
		
		/** Returns whether it is a correct object. */
		bool isCorrect() const { return m_correct && m_type==Object::oper && m_optype!=Operator::none;}
		
		/** Returns the multiplicity operator of an operator @p t. e.g. 5+5+5+5=5*4 -> times is the multiplicityOperator of plus. */
		static OperatorType multiplicityOperator(const OperatorType& t);
		
		/** Returns this operator @p o precedence. */
		static unsigned int operatorWeight(OperatorType o);
		
		/** Converts a @p s operator tag to the operator. */
		static enum OperatorType toOperatorType(const QString &s);
		
		/** Returns the expected number of parameters of a type. If there can be multiple parameters, -1 is returned */
		static int nparams(enum OperatorType);
		
		/** Returns if it is a trigonometric operator. */
		static bool isTrigonometric(enum OperatorType o);
		
		virtual QString visit(ExpressionWriter*) const;
	private:
		static char m_words[nOfOps][14];
		enum OperatorType m_optype;
};

#endif
