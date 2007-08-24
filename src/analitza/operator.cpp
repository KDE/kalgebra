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

char* Operator::m_words[nOfOps] = {
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
	"arccot", "arcoth",
	"arccosh", "arccsc", "arccsch",
	"arcsec", "arcsech", "arcsinh", "arctanh",
	"exp", "ln", "log",
	"conjugate", "arg", "real", "imaginary",
	"sum", "product", "diff", "function",
};

QString Operator::toString() const
{
	Q_ASSERT(m_optype<nOfOps);
	return QString("%1").arg(m_words[m_optype]);
}

QString Operator::toMathML() const
{
	Q_ASSERT(m_optype<nOfOps);
	return QString("<%1 />").arg(m_words[m_optype]);
}


enum Operator::OperatorType Operator::toOperatorType(const QString &e)
{
	//qDebug(), "lol";
	enum OperatorType ret=none;
	
	if(e=="plus") ret=plus;
	else if(e=="times") ret=times;
	else if(e=="min") ret=min;
	else if(e=="max") ret=max;
	else if(e=="and") ret=_and;
	else if(e=="or" ) ret=_or;
	else if(e=="xor") ret=_xor;
	else if(e=="gcd") ret=gcd;
	else if(e=="lcm") ret=lcm;
	else if(e=="max") ret=max;
	else if(e=="min") ret=min;
	else if(e=="quotient") ret=quotient;
	else if(e=="power") ret=power;
	else if(e=="minus") ret=minus;
	else if(e=="divide" ) ret=divide;
	else if(e=="rem") ret=rem;
	else if(e=="factorof") ret=factorof;
	else if(e=="lt") ret=lt;
	else if(e=="gt") ret=gt;
	else if(e=="eq") ret=eq;
	else if(e=="neq") ret=neq;
	else if(e=="leq") ret=leq;
	else if(e=="geq") ret=geq;
	else if(e=="implies") ret=implies;
	else if(e=="approx") ret=approx;
	else if(e=="root") ret=root;
	else if(e=="factorial") ret=factorial;
	else if(e=="abs") ret=abs;
	else if(e=="sin") ret=sin;
	else if(e=="cos") ret=cos;
	else if(e=="tan") ret=tan;
	else if(e=="sec") ret=sec;
	else if(e=="csc") ret=csc;
	else if(e=="cot") ret=cot;
	else if(e=="sinh") ret=sinh;
	else if(e=="cosh") ret=cosh;
	else if(e=="tanh") ret=tanh;
	else if(e=="sech") ret=sech;
	else if(e=="csch") ret=csch;
	else if(e=="coth") ret=coth;
	else if(e=="arcsin") ret=arcsin;
	else if(e=="arccos") ret=arccos;
	else if(e=="arctan") ret=arctan;
	else if(e=="arccos") ret=arccos;
	else if(e=="arccot") ret=arccot;
	else if(e=="arccoth") ret=arccoth;
	else if(e=="arccosh") ret=arccosh;
	else if(e=="arccsc" ) ret=arccsc;
	else if(e=="arccsch") ret=arccsch;
	else if(e=="arcsec" ) ret=arcsec;
	else if(e=="arcsech") ret=arcsech;
	else if(e=="arcsinh") ret=arcsinh;
	else if(e=="arctanh") ret=arctanh;
	else if(e=="exp") ret=exp;
	else if(e=="ln") ret=ln;
	else if(e=="log") ret=log;
	else if(e=="not") ret=_not;
	else if(e=="factorial") ret=factorial;
	else if(e=="minus") ret=minus;
	else if(e=="abs") ret=abs;
	else if(e=="conjugate") ret=conjugate;
	else if(e=="arg") ret=arg;
	else if(e=="real") ret=real;
	else if(e=="imaginary") ret=imaginary;
	else if(e=="floor") ret=floor;
	else if(e=="ceiling") ret=ceiling;
	else if(e=="sum") ret=sum;
	else if(e=="product") ret=product;
	else if(e=="diff") ret=diff;
	
	return ret;
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
		case times:
		case min:
		case max:
		case _and:
		case _or:
		case _xor:
		case gcd:
		case lcm:
		case function:
			return -1;
		case quotient:
		case power:
		case minus:
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
		case arccoth:
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
		case conjugate:
		case arg:
		case real:
		case imaginary:
		case floor:
		case ceiling:
		case sum:
		case product:
		case diff:
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
		case plus:
			return 2;
		case times:
			return 3;
		case divide:
			return 4;
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
			return true;
		default:
			return false;
	}
}


