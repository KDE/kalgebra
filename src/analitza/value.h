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

#ifndef VALUE_H
#define VALUE_H

#include "object.h"
#include "operator.h"
#include "analitzaexport.h"

#include <cmath>

/**
*	The Cn class is the one that represents a value in the expression trees.
*	@author Aleix Pol <aleixpol@gmail.com>
*/

class QDomElement;

class ANALITZA_EXPORT Cn : public Object
{
	public:
// 		enum ValueFormat { none, nan, real, integer, boolean };
		/** Copy constructor. Creates a Cn from another one. */
		Cn(const Cn& v) : Object(v), m_value(v.value()), m_boolean(v.m_boolean) {}
		
		/** Constructor. Creates a value with values @p v. */
		Cn(const double &b=0.) : Object(Object::value), m_value(b), m_boolean(false) {}
		
		/** Constructor. Creates a boolean value with value @p v. */
		Cn(bool b) : Object(Object::value), m_value(b?1.:0.), m_boolean(true) {}
		
		/** Constructor. Creates a value from @p o. If @p o is not a Cn, a not correct Cn will be returned. */
		Cn(Object const * o);
		virtual ~Cn() {}
		
		/**
		 *	Extracts the number from the @p e Dom element and saves it.
		 */
		void setValue(const QDomElement& e);
		
		/**
		 *	Sets the new value of this function
		 *	@param v the new value
		 */
		inline void setValue(const double& v) { m_value=v; }
		
		/**
		 *	Returns the value.
		 */
		inline double value() const { return m_value; }
		
		/**
		 *	Returns the value as an int.
		 */
		int intValue() const { return static_cast<int>(std::floor(m_value)); }
		
		/**
		 *	Returns whether it is a boolean value or not.
		 */
		bool isBoolean() const { return m_boolean; }
		
		/**
		 *	Sets whether this value is boolean or not.
		 */
		void setBoolean(bool val) { m_boolean=val; }
		
		/**
		 *	@return If it is a boolean value, returns if it is true or not, otherwise retuns false.
		 */
		bool isTrue() const { return m_boolean && m_value!=0.; }
		
// 		static double toNum(const QString& num, const QString& type, int base);
// 		static enum ValueFormat whatValueFormat(const QDomElement&);
		
		/**
		 *	Returns whether it is an integer value or not.
		 */
		bool isInteger() const { return std::floor(m_value)==m_value; }
		
		/**
		 *	Returns whether @p d is equal than this object.
		 */
		bool operator==(const Cn& d) const { return m_value==d.value(); }
		
		/**
		 *	Returns whether @p d is less than this object.
		 */
		bool operator<(const Cn& d) const { return m_value<d.value(); }
		
		/**
		 *	Returns whether @p d is less than this object's value.
		 */
		bool operator<(double d) const { return m_value<d; }
		
		/**
		 *	Returns whether @p d is less or equal than this object.
		 */
		bool operator<=(const Cn& d) const { return m_value<=d.value(); }
		
		/**
		 *	Returns whether @p d is less or equal than this object's value.
		 */
		bool operator<=(double d) const { return m_value<=d; }
		
		/**
		 *	Sets the new value to @p d.
		 */
		Cn operator=(double d) { m_value=d; return *this; }
		
		/**
		 *	The sign of the value will be inverted.
		 */
		void negate() { m_value*=-1.; }
		
		/**
		 *	Increments by one the value.
		 */
		Cn operator++() { m_value++; return this; }
		
		/**
		 *	Increments by one the value.
		 */
		Cn operator++(int) { m_value++; return this; }
		
		/**
		 *	Returns a string representation of the value.
		 */
		virtual QString toString() const;
		
		/**
		 *	Returns a MathML representation of the value.
		 */
		virtual QString toMathML() const;
		
		/**
		 *	Returns a html representation of the value.
		 */
		virtual QString toHtml() const;
		
		/*/** Sets whether it is a correct Cn. 
		void setCorrect(bool b) {m_correct = b; }*/
		
		/** Returns whether it is a correct Cn. */
		bool isCorrect() const { return m_correct; }
		
		QString reduce(enum Operator::OperatorType op, const Cn& oper);
		
		QString reduceUnary(enum Operator::OperatorType op);
		
		static Cn pi();
		static Cn e();
		static Cn euler();
	private:
		double m_value;
		bool m_boolean;
// 		enum ValueFormat m_vformat;
};

#endif
