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

#include <cmath>
#include <QString>
#include <QStringList>

#include "value.h"
#include "operator.h"
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
		
		/** Returns an expression. */
		Expression* expression() { return &m_exp; }
		
		/** Calculates the expression and returns a value alone. */
		Cn calculate();
		
		/** Evaluates an expression, like calculate() but returns a tree. */
		Expression evaluate();
		
		/** Evaluates the derivative of an expression expression, like expression() but sorrounded with a diff(). */
		Expression derivative(/*QStringList vars*/);
		
		/** Returns whether there has been a problem in the last calculation. */
		bool isCorrect() const { return m_exp.isCorrect() && m_err.isEmpty(); }
		
		/** Returns whether @p var is a function. */
		bool isFunction(const Ci& var) const;
		
		/** Returns a list of expression's bounded variable names. */
		QStringList bvarList() const;
		
		/** Empties the error list. */
		void flushErrors() { m_err = QStringList(); }
		
		/** simplifies the expression. */
		void simplify(); //FIXME: Should return an Expression
		
		/** Returns whether there is any @p var variable in the @p o tree. @p bvars tells the already defined variables (which won't return true). */
		static bool hasVars(const Object* o, const QString &var=QString(), const QStringList& bvars=QStringList());
		
		/** @returns Return an error list. */
		QStringList errors() const { return m_exp.error() + m_err; }
		
		/** @returns Returns a way to query variables. */
		const Variables *variables() const { return m_vars; }
		
		/** Adds a variable entry */
		void insertVariable(const QString& name, const Expression& value);
		
		/** Adds a variable entry */
		void insertVariable(const QString& name, const Object* value);
// 	protected: //FIXME:Shame on me
		Expression m_exp;
		Variables *m_vars;
		QStringList m_err;
	private:
		Cn calc(const Object* e);
		Cn operate(const Container*);
		Object* eval(const Object* e, bool vars);
		
		Cn sum(const Container& c);
		Cn product(const Container& c);
		Cn func(const Container& c);
		
		Object* simp(Object* root);
		Object* simpScalar(Container* c);
		Object* simpPolynomials(Container* c);
		Object* simpSum(Container* c);
		Object* simpPiecewise(Container* c);
		
		Object* derivative(const QString &var, const Object*);
		Object* derivative(const QString &var, const Container*);
		void reduce(enum Operator::OperatorType op, Cn *ret, Cn oper, bool unary);
		Object* removeDependencies(Object* o) const;
		void levelOut(Container *c, Container *ob, QList<Object*>::iterator &it);
		
		static bool hasTheVar(const QStringList& vars, const Object* o);
		static bool hasTheVar(const QStringList& vars, const Container* c);
};

#endif
