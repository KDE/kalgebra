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

#include "operator.h"
#include "expressionwriter.h"

char Operator::m_words[nOfOps][14] = {
	"onone",
	"plus", "times", "minus", "divide", "quotient",
	"power", "root", "factorial",
	"and", "or", "xor", "not",
	"gcd", "lcm", "rem", "factorof",
	"max", "min",
	"lt", "gt", "eq", "neq", "leq", "geq", "implies",
	"approx", "abs", "floor", "ceiling",
	"sin", "cos", "tan",
	"sec", "csc", "cot",
	"sinh", "cosh", "tanh",
	"sech", "csch", "coth",
	"arcsin", "arccos", "arctan",
	"arccot", //"arccoth",
	"arccosh", "arccsc", "arccsch",
	"arcsec", "arcsech", "arcsinh", "arctanh",
	"exp", "ln", "log",// "real",
// 	"conjugate", "arg", "imaginary",
	"sum", "product", "diff",
	//Vector operations
	"card", "scalarproduct", "selector",
	"function"
};

QString Operator::visit(ExpressionWriter* e) const
{
	return e->accept(this);
}

QString Operator::name() const
{
	Q_ASSERT(m_optype<nOfOps);
	return QString(m_words[m_optype]);
}

enum Operator::OperatorType Operator::toOperatorType(const QString &e)
{
	//qDebug(), "lol";
	for(int i=none; i<nOfOps; i++)
	{
		if(m_words[i]==e)
			return (OperatorType) i;
	}
	
	return none;
}

Operator::OperatorType Operator::multiplicityOperator(const Operator::OperatorType& t)
{
	OperatorType r;
	switch(t) {
		case minus:
		case plus:
			r=times;
			break;
		case times:
			r = power;
			break;
		default:
			r=none;
	}
	return r;
}


int Operator::nparams(Operator::OperatorType t)
{
	switch(t) {
		case plus:
		case minus:
		case times:
		case min:
		case max:
		case _and:
		case _or:
		case _xor:
		case gcd:
		case lcm:
		case function:
		case scalarproduct:
			return -1;
		case quotient:
		case power:
		case divide:
		case rem:
		case factorof:
		case lt:
		case gt:
		case eq:
		case neq:
		case leq:
		case geq:
		case implies:
		case approx:
		case root:
		case selector: //FIXME: Should move to -1 when matrix appear
			return 2;
		case abs:
		case sin:
		case cos:
		case tan:
		case sec:
		case csc:
		case cot:
		case sinh:
		case cosh:
		case tanh:
		case sech:
		case csch:
		case coth:
		case arcsin:
		case arccos:
		case arctan:
		case arccot:
// 		case arccoth:
		case arccosh:
		case arccsc:
		case arccsch:
		case arcsec:
		case arcsech:
		case arcsinh:
		case arctanh:
		case exp:
		case ln:
		case log:
		case _not:
		case factorial:
// 		case real:
// 		case conjugate:
// 		case arg:
// 		case imaginary:
		case floor:
		case ceiling:
		case sum:
		case product:
		case diff:
		case card:
			return 1;
		case nOfOps:
		case none:
			break;
	}
	return 0;
}


Operator::Operator(Object const *o) : Object(o->type())
{
	if(type() == Object::oper) {
		Operator *c = (Operator*) o;
		m_optype = c->operatorType();
	} else
		setType(Object::none);
}

unsigned int Operator::operatorWeight(OperatorType op)
{
	switch(op) {
		case lt:
		case gt:
		case eq:
		case neq:
		case leq:
		case geq:
			return 1;
		case minus:
			return 2;
		case plus:
			return 3;
		case times:
			return 4;
		case divide:
			return 5;
		case _and:
		case _or:
		case _xor:
			return 6;
		case power:
			return 7;
		default:
			return 1000;
	}
	return 666;
}

bool Operator::isBounded() const
{
	switch(m_optype) {
		case sum:
		case product:
		case function:
		case diff:
			return true;
		default:
			return false;
	}
}

bool Operator::isTrigonometric(enum OperatorType t)
{
	switch(t) {
		case sin:
		case cos:
		case tan:
		case sec:
		case csc:
		case cot:
		case sinh:
		case cosh:
		case tanh:
		case sech:
		case csch:
		case coth:
		case arcsin:
		case arccos:
		case arctan:
		case arccot:
// 		case arccoth:
		case arccosh:
		case arccsc:
		case arccsch:
		case arcsec:
		case arcsech:
		case arcsinh:
		case arctanh:
			return true;
		default:
			return false;
	}
}


