/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef VARIABLE_H
#define VARIABLE_H

#include "object.h"
namespace Analitza
{

/** A variable object, name refers to MathML standard. */
class ANALITZA_EXPORT Ci : public Object
{
	public:
		/** Constructor. Creates a variable with a @p b name */
		explicit Ci(const QString& b);
		
		~Ci();
		
		/** Returns the variable name */
		QString name() const { return m_name; }
		
		/** Returns whether @p var name is equal to this variable one. */
		bool operator==(const Ci& var) const { return var.m_name==m_name; }
		
		/** Sets whether it is a function. */
		void setFunction(bool f) { m_function=f; }
		
		/** Returns whether it is a variable that has to be a function */
		bool isFunction() const { return m_function; }
		
		/** Returns the MathML representation of the variable */
		QString toMathML() const;
		
		/** Returns the HTML representation of the variable */
		QString toHtml() const { return QString("<span class='%1'>%2</span>").arg(m_function ? "func" : "var").arg(m_name); }
		
		///Value for the variable, only valid after colorizing the tree
		Object*& value() const { Q_ASSERT(m_value); return *m_value; }
		
		///@p param value sets the new value for the item
		void setValue(Object** value, bool owner);
		
		bool isDefined() const { return m_value!=0; }
		
		virtual QString visit(ExpressionWriter*) const;
		virtual bool matches(const Object* pattern, QMap<QString, const Object*>* found) const;
		virtual Object* copy() const;
		
		virtual bool decorate(const ScopeInformation& scope);
		
	private:
		QString m_name;
		bool m_function;
		bool m_owner;
		Object** m_value;
};

}
#endif
