/*************************************************************************************
 *  Copyright (C) 2007-2010 by Aleix Pol <aleixpol@kde.org>                          *
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


#ifndef ANALYZER_H
#define ANALYZER_H

#include <QStringList>

#include "expression.h"
#include "analitzaexport.h"
#include "expressiontype.h"
#include "builtinmethods.h"
#include <QStack>

namespace Analitza
{

class Apply;
class BoundingIterator;
class BuiltinMethods;
class Object;
class Variables;
class Container;
class Operator;
class Ci;

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
		
		/** Calculates the expression and returns a value alone. */
		Expression calculate();
		
		/**
		 * Calculates the expression and returns a value alone.
		 * The parameters need to be set by passing a stack instance
		 */
		Expression calculateLambda();
		
		/** Evaluates an expression, like calculate() but returns a tree. */
		Expression evaluate();
		
		/** Evaluates the derivative of an expression expression, like expression() but sorrounded with a diff(). */
		Expression derivative(const QString& var);
		
		/** Evaluates the derivative of an expression expression. */
		double derivative(const QVector<Object*>& values );
		
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
		
		void setStack(const QVector<Object*>& stack) { m_runStack = stack; }
		
		BuiltinMethods* builtinMethods();
		
		/** Makes it possible to easily enter a bunch of code to execute it */
		void importScript(QTextStream* stream);
	private:
		typedef Object* (Analyzer::*funcContainer)(const Container*);
		static funcContainer operateContainer[];
		
		Expression m_exp;
		Variables *m_vars;
		QStringList m_err;
		QVector<Object*> m_runStack;
		int m_runStackTop;
		BuiltinMethods m_builtin;
		
		const bool m_varsOwned;
		bool m_hasdeps;
		ExpressionType m_currentType;
		
		Object* calc(const Object* e);
		Object* operate(const Container*);
		Object* operate(const Apply*);
		Object* eval(const Object* e, bool vars, const QSet<QString>& unscoped);
		
		Object* sum(const Apply& c);
		Object* product(const Apply& c);
		Object* exists(const Apply& c);
		Object* forall(const Apply& c);
		Object* func(const Apply& c);
		Object* calcDiff(const Apply* c);
		Object* calcMap(const Apply* c);
		Object* calcFilter(const Apply* c);
		
		Object* calcPiecewise(const Container* c);
		Object* calcDeclare(const Container* c);
		Object* calcMath(const Container* c);
		Object* calcLambda(const Container* c);
		Object* calcCallFunction(Analitza::Container* function, const QVector<Analitza::Object* >& args, const Analitza::Object* op);
		
		Object* simp(Object* root);
		Object* simpScalar(Apply* c);
		Object* simpPolynomials(Apply* c);
		Object* simpSum(Apply* c);
		Object* simpApply(Apply* c);
		Object* simpPiecewise(Container* c);
		
		Object* derivative(const QString &var, const Object*);
		void levelOut(Apply *c, Apply *ob, QVector<Object*>::iterator &it);
		Object* boundedOperation(const Apply & n, const Operator & t, Object* initial);
		
		BoundingIterator* initializeBVars(const Apply* n, int base);
		BoundingIterator* initBVarsContainer(const Apply* n, int base, Object* domain);
		BoundingIterator* initBVarsRange(const Apply* n, int base, Object* dlimit, Object* ulimit);
		
		template <class T, class Tit>
		void iterateAndSimp(T* v);
		
		Object* variableValue(Ci* var);
		
		template <class T, class Tit>
		void alphaConversion(T* o, int min);
		void alphaConversion(Apply* a, int min);
		void alphaConversion(Container* a, int min);
		Object* applyAlpha(Analitza::Object* o, int min);
};

}
#endif
