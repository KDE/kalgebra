/***************************************************************************
 *   Copyright (C) 2006 by Aleix Pol                                       *
 *   aleixpol@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef OBJECT_H
#define OBJECT_H

/**
	@author Aleix Pol <aleixpol@gmail.com>
*/

#include <QList>
#include <QtCore>

#define i18n QString

// OperatorsModel m_words;
class Object
{
public:
	enum ObjectType { none=0, value, variable, oper, container };
	enum ContainerType { cnone=0, math, apply, declare, lambda, bvar, uplimit, downlimit };
	enum OperatorType {
		onone=0,
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
		arccot, arcoth,
		arccosh, arccsc, arccsch,
		arcsec, arcsech, arcsinh, arctanh,
		exp, ln, log,
		conjugate, arg, real, imaginary,
		sum, product, diff, function //FIXME: <- ????
	};
	
	virtual ~Object(){}
	
	void setType(enum ObjectType t) { m_type=t; }
	enum ObjectType type() const { return m_type; }
	bool isCorrect() const { return m_correct && m_type!=none; }
	void setCorrect(bool c) { m_correct=c; }
	bool isContainer() { return m_type==container; }
	virtual QString toString() const { return "object"; };
	virtual QString toMathML() const { return "object"; };
	static enum ObjectType whatType(const QString& tag);
protected:
	Object(enum ObjectType t) : m_type(t), m_correct(true) {}
private:
	enum ObjectType m_type;
	bool m_correct;
	QString err;
};

class Ci : public Object
{
	public:
		Ci(const Object *);
		Ci(QString b=QString()) : Object(variable), m_name(b) {}
		void setName(const QString& n) { m_name=n; }
		QString name() const { return m_name; }
		
		bool operator==(const Ci& var) { return var.m_name==m_name; }
		bool isFunction() const { return m_function; }
		void setFunction(bool f) { m_function=f; }
		QString toString() const { return m_name; }
		QString toMathML() const { return QString("<ci>%1</ci>").arg(m_name);}
	private:
		QString m_name;
		bool m_function;
};

#endif
