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


#ifndef ANALITZA_H
#define ANALITZA_H

#include <QStringList>

#include "expression.h"
#include "analitzaexport.h"

class Object;
class Variables;
class Container;

//FIXME: Explain that better and with examples.
/**
 *	This is the base Math class. Analitza will be the one that will calculate things.
 *	@author Aleix Pol <aleixpol@gmail.com>
 */

class ANALITZA_EXPORT Analitza
{
	//FIXME: Remove all friends. Most solved with variables getter.
	friend class VarEdit;
	friend class VariableView;
	friend class function;
	friend class Calculate3D;
	friend class ExpressionEdit;
	
	public:
		/** Constructor. Creates an empty Analitza module. */
		Analitza();
		
		/** Copy constructor. Creates a copy of the @p a Analitza instance. Inherits its Variable structure. */
		Analitza(const Analitza& a);
		
		/** Destructor. */
		~Analitza();
		
		/** Sets another Variable module so that it is used in further calculations. */
		void setVariables(Variables* v);
		
		/** Sets an expression to calculate. */
		void setExpression(const Expression &e);
		
		/** Returns the expression in use. */
		const Expression* expression() { return &m_exp; }
		
		/** Returns the expression in use. */
		const Expression & expression() const { return m_exp; }
		
		/** Calculates the expression and returns a value alone. */
		Expression calculate();
		
		/** Evaluates an expression, like calculate() but returns a tree. */
		Expression evaluate();
		
		/** Evaluates the derivative of an expression expression, like expression() but sorrounded with a diff(). */
		Expression derivative(/*QStringList vars*/);
		
		/** Evaluates the derivative of an expression expression. */
		double derivative(const QList< QPair<QString, double > >& values );
		
		/** Returns whether there has been a problem in the last calculation. */
		inline bool isCorrect() const { return m_err.isEmpty() && m_exp.isCorrect(); }
		
		/** Returns whether @p var is a function. */
		bool isFunction(const Ci& var) const;
		
		/** Returns a list of expression's bounded variable names. */
		QStringList bvarList() const;
		
		/** Empties the error list. */
		void flushErrors() { m_err.clear(); }
		
		/** simplifies the expression. */
		void simplify(); //FIXME: Should return an Expression
		
		/** Returns whether there is any @p var variable in the @p o tree. @p bvars tells the already defined variables (which won't return true). */
		static bool hasVars(const Object* o, const QString &var=QString(), const QStringList& bvars=QStringList());
		
		/** @returns Return an error list. */
		QStringList errors() const { return m_exp.error() + m_err; }
		
		/** @returns Returns a way to query variables. */
		const Variables *variables() const { return m_vars; }
		
		/** @returns Returns a way to query variables. */
		Variables *variables() { return m_vars; }
		
		/** Adds a variable entry */
		void insertVariable(const QString& name, const Expression& value);
		
		/** Adds a variable entry */
		void insertVariable(const QString& name, const Object* value);
		
	protected:
		Expression m_exp;
		Variables *m_vars;
		QStringList m_err;
	private:
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
		
		Object* derivative(const QString &var, const Object*);
		Object* derivative(const QString &var, const Container*);
		Object* removeDependencies(Object* o) const;
		void levelOut(Container *c, Container *ob, QList<Object*>::iterator &it);
		
		static bool hasTheVar(const QStringList& vars, const Object* o);
		static bool hasTheVar(const QStringList& vars, const Container* c);
};

#endif
