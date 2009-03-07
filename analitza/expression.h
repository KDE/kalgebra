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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QStringList>
#include <QSharedDataPointer>

#include "analitzaexport.h"
#include "object.h"

class Container;
class Cn;
class QDomElement;

/**
 *	This class represents an expression.
 *	Expression let to convert it to string, MathML and make some little queries to it, without calculating anything.
 *
 *	@author Aleix Pol <aleixpol@gmail.com>
 */

class ANALITZA_EXPORT Expression
{
	public:
		
		/**
		 *	Constructs an empty Expression.
		 */
		Expression();
		
		/**
		 *	Copy constructor, copies the whole object to the constructed one.
		 */
		Expression(const Expression& e);
		
		/**
		 *	Creates an expression from a value
		 */
		Expression(const Cn& e);
		
		explicit Expression(Object* o);
		
		/**
		 *	Constructor. Parses an expression and creates the object.
		 *	@param exp expression to be assigned
		 *	@param mathml format of the expression
		 */
		Expression(const QString& exp, bool mathml=false);
		
		/** Destructor */
		~Expression();
		
		/**
		 *	Sets an expression @p exp which is not in MathML format. Returns whether it was correctly assigned or not.
		 */
		bool setText(const QString &exp);
		
		/**
		 *	Sets an expression @p exp which is in MathML format. Returns whether it was correctly assigned or not.
		 */
		bool setMathML(const QString &exp);
		
		/**
		 *	Returns the list of errors that had experienced while building the expression.
		 */
		QStringList error() const;
		
		/**
		 *	Returns whether this is a correct expression.
		 */
		bool isCorrect() const;
		
		/**
		 *	Returns whether the @p e is equal.
		 */
		bool operator==(const Expression& e) const;
		
		/**
		 *	Copy assignment. Copies the @p e expression here.
		 */
		Expression operator=(const Expression& e);
		
		/**
		 *	Returns the uplimit field tree.
		 */
		Expression uplimit() const;
		
		/**
		 *	Returns the downlimit field tree.
		 */
		Expression downlimit() const;
		
		/**
		 *	Returns whether it is a lambda-expression.
		 */
		bool isLambda() const;
		
		/**
		 *	Returns the tree associated to this object.
		 */
		const Object* tree() const;
		
		/**
		 *	Returns the tree associated to this object.
		 */
		Object* tree();
		
		void setTree(Object* o);
		/**
		 *	Converts the expression to a string expression.
		 */
		QString toString() const;
		
		/**
		 *	Converts the expression to MathML.
		 */
		QString toMathML() const;
		
		/**
		 *	Exports the expression to HTML.
		 */
		QString toHtml() const;
		
		/**
		 *	Converts the expression to MathML Presentation Markup.
		 */
		QString toMathMLPresentation() const;
		
		/**
		 * Invalidates the data of the expression.
		 */
		void clear();
		
		/**
		 * @returns Lists the global bounded variables in the expression
		 */
		QStringList bvarList() const;
		
		/** @returns Value representation of the expression. */
		Cn value() const;
		
		/** @returns true if the expression is a value, false otherwise. */
		bool isValue() const;
		
		/**
		 *	Converts a @p tag to an object type.
		 */
		static enum Object::ObjectType whatType(const QString& tag);
		
		/**
		 *	Returns whether @p s is MathML or not. Very simple.
		 */
		static bool isMathML(const QString& s) { return !s.isEmpty() && s[0]=='<'; }
		
	private:
		Object* branch(const QDomElement& elem);
		
		class ExpressionPrivate;
		QSharedDataPointer<ExpressionPrivate> d;
};

#endif
