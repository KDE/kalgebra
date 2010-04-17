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


#ifndef ANALIZER_H
#define ANALIZER_H

#include <QStringList>

#include "expression.h"
#include "analitzaexport.h"
#include "expressiontype.h"

namespace Analitza
{
class BoundingIterator;
class Object;
class Variables;
class Container;
class Operator;
class Ci;

class BoundingIterator
{
	public:
		virtual ~BoundingIterator() {}
		virtual bool hasNext()=0;
};

//FIXME: Explain that better and with examples.
/**
 *	This is the base Math class. Analyzer will be the one that will calculate things.
 *	@author Aleix Pol <aleixpol@kde.org>  
 */

class ANALITZA_EXPORT Analyzer
{
	public:
		/** Constructor. Creates an empty Analyzer module with a Variables module. */
		Analyzer();
		
		/** Constructor. Creates an empty Analyzer module. 
			@param v: Sets a custom variables module. This module will _not_ be deleted along with Analyzer
		*/
		Analyzer(Variables* v);
		
		/** Copy constructor. Creates a copy of the @p a Analyzer instance. Inherits its Variable structure. */
		Analyzer(const Analyzer& a);
		
		/** Destructor. */
		~Analyzer();
		
		/** Sets an expression to calculate. */
		void setExpression(const Expression &e);
		
		/** Returns the expression in display. */
		const Expression& expression() const { return m_exp; }
		
		/** Returns the expression in display. Use carefully */
		Expression* refExpression() { return &m_exp; }
		
		/** Calculates the expression and returns a value alone. */
		Expression calculate();
		
		/** Calculates the expression and returns a value alone. The parameters need to be set using ::refExpression()->parameters()*/
		Expression calculateLambda();
		
		/** Evaluates an expression, like calculate() but returns a tree. */
		Expression evaluate();
		
		/** Evaluates the derivative of an expression expression, like expression() but sorrounded with a diff(). */
		Expression derivative(/*QStringList vars*/);
		
		/** Evaluates the derivative of an expression expression. */
		double derivative(const QList< QPair<QString, double > >& values );
		
		/** Returns whether there has been a problem in the last calculation. */
		bool isCorrect() const { return m_err.isEmpty() && m_exp.isCorrect(); }
		
		/** Empties the error list. */
		void flushErrors() { m_err.clear(); }
		
		/** simplifies the expression. */
		void simplify(); //FIXME: Should return an Expression
		
		/** @returns Return an error list. */
		QStringList errors() const { return m_exp.error() + m_err; }
		
		/** @returns Returns a way to query variables. */
		Variables *variables() const { return m_vars; }
		
		/**
			Adds a variable entry. It is the proper way to do it because tracks some possible errors.
			May change the error in case we're trying to represent something wrong.
			@returns Returns if it was actually inserted.
		*/
		bool insertVariable(const QString& name, const Expression& value);
		
		/**
			Adds a variable entry. It is the proper way to do it because tracks some possible errors.
			May change the error in case we're trying to represent something wrong.
			@returns Returns if it was actually inserted.
		*/
		bool insertVariable(const QString& name, const Object* value);
		
		/**
			Adds a variable entry named @p name with @p value value.
			@returns Returns the added object
		*/
		Cn* insertValueVariable(const QString& name, double value);
		
		/** Returns whether the current expression has all data it needs to be calculated.*/
		bool hasDependencies() const { return m_hasdeps; }
		
		/** This method is useful if you want to work programatically on functions with undefined variables.
			@returns the same expression set but with explicit dependencies.
			
			e.g. x+2 would return x->x+2
		*/
		Expression dependenciesToLambda() const;
		
		/** This method lets you retrieve the current type in use.
		 @returns the type of the current expression.
		 */
		ExpressionType type() const { return m_currentType; }
	private:
		Expression m_exp;
		Variables *m_vars;
		QStringList m_err;
		
		const bool m_varsOwned;
		bool m_hasdeps;
		ExpressionType m_currentType;
		
		Object::ScopeInformation varsScope() const;
		
		Object* calc(const Object* e);
		Object* operate(const Container*);
		Object* eval(const Object* e, bool vars, const QSet<QString>& unscoped);
		
		Object* sum(const Container& c);
		Object* product(const Container& c);
		Object* func(const Container& c);
		
		Object* calcPiecewise(const Container* c);
		Object* calcDeclare(const Container* c);
		
		Object* simp(Object* root);
		Object* simpScalar(Container* c);
		Object* simpPolynomials(Container* c);
		Object* simpSum(Container* c);
		Object* simpPiecewise(Container* c);
		Object* simpApply(Container* c);
		
		Object* derivative(const QString &var, const Object*);
		Object* derivative(const QString &var, const Container*);
		void levelOut(Container *c, Container *ob, QList<Object*>::iterator &it);
		Object* boundedOperation(const Container & n, const Operator & t, Object* initial);
		
		BoundingIterator* initializeBVars(const Container* n);
};

}
#endif
