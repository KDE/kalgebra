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

#include "operations.h"

#include <math.h>

#include <cmath>
#include <kdemacros.h>

#include "value.h"
#include "vector.h"
#include "expression.h"
#include "list.h"
#include "expressiontypechecker.h"
#include "customobject.h"
#include "localize.h"

using namespace std;
using namespace Analitza;

bool Operations::isNull(Operator::OperatorType opt, Object* ret)
{
	return ret->type()==Object::value &&
		((opt==Operator::_and && static_cast<Cn*>(ret)->value()==0.) || (opt==Operator::_or && static_cast<Cn*>(ret)->value()==1.));
}

Cn* Operations::reduceRealReal(enum Operator::OperatorType op, Cn *oper, const Cn *oper1, QString** correct)
{
	int residu;
	double a=oper->value(), b=oper1->value();
	Cn::ValueFormat format=Cn::ValueFormat(oper->format() | oper1->format());
	
	switch(op) {
		case Operator::plus:
			a += b;
			break;
		case Operator::times:
			a *= b;
			break;
		case Operator::divide:
			a /=b;
			break;
		case Operator::minus:
			a -= b;
			break;
		case Operator::power:
			a = pow(a, b);
			break;
		case Operator::rem:
			format=Cn::Integer;
			if(KDE_ISLIKELY(floor(b)!=0.))
				a = remainder(a, b);
			else
				*correct=new QString(i18n("Cannot calculate the remainder on 0."));
			break;
		case Operator::quotient:
			a = floor(a / b);
			format=Cn::Integer;
			break;
		case Operator::factorof:
			if(KDE_ISLIKELY(floor(b)!=0.))
				a = ((int(floor(a)) % int(floor(b)))==0) ? 1.0 : 0.0;
			else
				*correct=new QString(i18n("Cannot calculate the factor on 0."));
			
			format=Cn::Boolean;
			break;
		case Operator::min:
			a= a < b? a : b;
			break;
		case Operator::max:
			a= a > b? a : b;
			break;
		case Operator::gt:
			a= a > b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::lt:
			a= a < b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::eq:
			a= a == b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::approx:
			a= fabs(a-b)<0.001? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::neq:
			a= a != b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::geq:
			a= a >= b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::leq:
			a= a <= b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::_and:
			a= a && b? 1.0 : 0.0;
			format=Cn::Boolean;
			break;
		case Operator::_or:
			a= a || b? 1.0 : 0.0;
			format = Cn::Boolean;
			break;
		case Operator::_xor:
			a= (a || b) && !(a&&b)? 1.0 : 0.0;
			format = Cn::Boolean;
			break;
		case Operator::implies:
			a= (a || !b)? 0.0 : 1.0;
			format = Cn::Boolean;
			break;
		case Operator::gcd:  {
			//code by michael cane aka kiko :)
			int ia=floor(a), ib=floor(b);
			while (ib > 0) {
				residu = ia % ib;
				ia = ib;
				ib = residu;
			}
			a=ia; b=ib;
			format=Cn::Integer;
		}	break;
		case Operator::lcm:
			//code by michael cane aka kiko :)
			if(KDE_ISUNLIKELY(floor(a)==0. || floor(b)==0.))
				*correct=new QString(i18n("Cannot calculate the lcm of 0."));
			else {
				int ia=floor(a), ib=floor(b);
				int ic=ia*ib;
				while (ib > 0) {
					residu = ia % ib;
					ia = ib;
					ib = residu;
				}
				ia=ic/ia;
				a=ia; b=ib;
			}
			format=Cn::Integer;
			break;
		case Operator::root:
			a = b==2.0 ? sqrt(a) : pow(a, 1.0/b);
			break;
		default:
			break;
	}
	oper->setValue(a);
	oper->setFormat(format);
	delete oper1;
	return oper;
}

Cn* Operations::reduceUnaryReal(enum Operator::OperatorType op, Cn *val, QString** correct)
{
	double a=val->value();
	Cn::ValueFormat format=val->format();
	
	switch(op) {
		case Operator::minus:
			a = -a;
			break;
		case Operator::factorial: {
			//Use gamma from math.h?
			uint res=1;
			for(; a>1.; a--) {
				res*=floor(a);
			}
			a=res;
			format=Cn::Integer;
		}	break;
		case Operator::sin:
			a=sin(a);
			break;
		case Operator::cos:
			a=cos(a);
			break;
		case Operator::tan:
			a=tan(a);
			break;
		case Operator::sec:
			a=1./cos(a);
			break;
		case Operator::csc:
			a=1./sin(a);
			break;
		case Operator::cot:
			a=1./tan(a);
			break;
		case Operator::sinh:
			a=sinh(a);
			break;
		case Operator::cosh:
			a=cosh(a);
			break;
		case Operator::tanh:
			a=tanh(a);
			break;
		case Operator::sech:
			a=1.0/cosh(a);
			break;
		case Operator::csch:
			a=1.0/sinh(a);
			break;
		case Operator::coth:
			a=cosh(a)/sinh(a);
			break;
		case Operator::arcsin:
			a=asin(a);
			break;
		case Operator::arccos:
			a=acos(a);
			break;
		case Operator::arctan:
			a=atan(a);
			break;
		case Operator::arccot:
			a=log(a+pow(a*a+1., 0.5));
			break;
		case Operator::arcsinh:
			a=0.5*(log(1.+1./a)-log(1.-1./a));
			break;
		case Operator::arccosh:
			a=log(a+sqrt(a-1.)*sqrt(a+1.));
			break;
		case Operator::arccsc:
			a=1/asin(a);
			break;
		case Operator::arccsch:
			a=1/(0.5*(log(1.+1./a)-log(1.-1./a)));
			break;
		case Operator::arcsec:
			a=1/(acos(a));
			break;
		case Operator::arcsech:
			a=1/(log(a+sqrt(a-1.)*sqrt(a+1.)));
			break;
		case Operator::arctanh:
			a=atanh(a);
			break;
		case Operator::exp:
			a=exp(a);
			break;
		case Operator::ln:
			a=log(a);
			break;
		case Operator::log:
			a=log10(a);
			break;
		case Operator::abs:
			a= a>=0. ? a : -a;
			break;
		//case Object::conjugate:
		//case Object::arg:
		//case Object::real:
		//case Object::imaginary:
		case Operator::floor:
			a=floor(a);
			format=Cn::Integer;
			break;
		case Operator::ceiling:
			a=ceil(a);
			format=Cn::Integer;
			break;
		case Operator::_not:
			a=!a;
			format=Cn::Boolean;
			break;
		default:
			*correct=new QString(i18n("Could not calculate a value %1", Operator(op).toString()));
			break;
	}
	
	val->setValue(a);
	val->setFormat(format);
	return val;
}

Object * Operations::reduceRealVector(Operator::OperatorType op, Cn * oper, Vector * v1, QString** correct)
{
	switch(op) {
		case Operator::selector: {
			int select=oper->intValue();
			Object* ret=0;
			if(select<1 || (select-1) >= v1->size()) {
				*correct=new QString(i18n("Invalid index for a container"));
				ret=new Cn(0.);
			} else {
				ret=v1->at(select-1);
				v1->setAt(select-1, 0);
			}
			delete oper;
			delete v1;
			return ret;
		}	break;
		default:
			for(Vector::iterator it=v1->begin(); it!=v1->end(); ++it)
			{
				*it=reduce(op, new Cn(*oper), *it, correct);
			}
			
			delete oper;
			return v1;
	}
}

Object * Operations::reduceVectorReal(Operator::OperatorType op, Vector * v1, Cn * oper, QString** correct)
{
	for(Vector::iterator it=v1->begin(); it!=v1->end(); ++it)
	{
		*it=reduce(op, *it, new Cn(*oper), correct);
	}
	delete oper;
	return v1;
}

Object * Operations::reduceVectorVector(Operator::OperatorType op, Vector * v1, Vector * v2, QString** correct)
{
	if(v1->size()!=v2->size()) {
		*correct=new QString(i18n("Cannot operate on different sized vectors."));
		return new Cn(0.);
	}
	
	if(op==Operator::scalarproduct)
		op=Operator::times;
	Vector::iterator it2=v2->begin();
	for(Vector::iterator it1=v1->begin(); it1!=v1->end(); ++it1)
	{
		*it1=reduce(op, *it1, *it2, correct);
		
		it2=v2->erase(it2);
	}
	delete v2;
	return v1;
}

Cn* Operations::reduceUnaryVector(Operator::OperatorType op, Vector * c, QString** correct)
{
	Cn *ret=0;
	switch(op) {
		case Operator::card:
			ret=new Cn(c->size());
			break;
		default:
			//Should be dealt by typechecker. not necessary
			*correct=new QString(i18n("Could not calculate a vector's %1", Operator(op).toString()));
			ret=new Cn(0.);
			break;
	}
	delete c;
	return ret;
}

Object* Operations::reduceListList(Operator::OperatorType op, List* l1, List* l2, QString** correct)
{
	Object* ret=0;
	switch(op) {
		case Operator::_union: {
			List::iterator itEnd=l2->end();
			for(List::iterator it=l2->begin(); it!=itEnd;) {
				l1->appendBranch(*it);
				it=l2->erase(it);
			}
			
			ret=l1;
		}	break;
		default:
			//Should be dealt by typechecker. not necessary
			*correct=new QString(i18n("Could not calculate a list's %1", Operator(op).toString()));
			delete l1;
			ret=new Cn(0.);
			break;
	}
	delete l2;
	return ret;
}

Cn* Operations::reduceUnaryList(Operator::OperatorType op, List* l, QString** correct)
{
	Cn *ret=0;
	switch(op) {
		case Operator::card:
			ret=new Cn(l->size());
			break;
		default:
			*correct=new QString(i18n("Could not calculate a list's %1", Operator(op).toString()));
			ret=new Cn(0.);
			break;
	}
	delete l;
	return ret;
}

Object* Operations::reduceRealList(Operator::OperatorType op, Cn* oper, List* v1, QString** correct)
{
	switch(op) {
		case Operator::selector: {
			int select=oper->intValue();
			Object* ret=0;
			if(select<1 || (select-1) >= v1->size()) {
				*correct=new QString(i18n("Invalid index for a container"));
				ret=new Cn(0.);
			} else {
				ret=v1->at(select-1);
				v1->setAt(select-1, 0);
			}
			delete oper;
			delete v1;
			return ret;
		}	break;
		default:
			break;
	}
	return 0;
}

ExpressionType TypeTriplet(const ExpressionType& a,const ExpressionType& b,const ExpressionType& c) { return ExpressionType(ExpressionType::Lambda).addParameter(a).addParameter(b).addParameter(c); }

//TODO: test that there's one output per input
QList<ExpressionType> Operations::infer(Operator::OperatorType op)
{
	QList<ExpressionType> ret;
	
	switch(op) {
		case Operator::plus:
		case Operator::minus:
			ret << TypeTriplet(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			ret << TypeTriplet(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			break;
		case Operator::divide:
			ret << TypeTriplet(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			ret << TypeTriplet(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Value),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			ret << TypeTriplet(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			break;
		case Operator::times:
			ret << TypeTriplet(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			ret << TypeTriplet(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Value),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			ret << TypeTriplet(ExpressionType(ExpressionType::Value),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			ret << TypeTriplet(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			break;
		case Operator::eq:
		case Operator::neq:
			ret << TypeTriplet(ExpressionType(ExpressionType::Any, 1),
							   ExpressionType(ExpressionType::Any, 1),
							   ExpressionType(ExpressionType::Bool));
			break;
		case Operator::scalarproduct:
			ret << TypeTriplet(
								ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
								ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1),
								ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), -1));
			break;
		case Operator::power:
		case Operator::rem:
		case Operator::quotient:
		case Operator::factorof:
		case Operator::min:
		case Operator::max:
		case Operator::gcd:
		case Operator::lcm:
		case Operator::root:
			ret << TypeTriplet(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			break;
		case Operator::gt:
		case Operator::lt:
		case Operator::approx:
		case Operator::geq:
		case Operator::leq:
			ret << TypeTriplet(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Bool));
			break;
		case Operator::_and:
		case Operator::_or:
		case Operator::_xor:
		case Operator::implies:
			ret << TypeTriplet(ExpressionType(ExpressionType::Bool), ExpressionType(ExpressionType::Bool), ExpressionType(ExpressionType::Bool));
			break;
		case Operator::selector:
			ret << TypeTriplet(ExpressionType(ExpressionType::Value),
							   ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Any, 1), -1),
							   ExpressionType(ExpressionType::Any, 1));
			ret << TypeTriplet(ExpressionType(ExpressionType::Value),
							   ExpressionType(ExpressionType::List, ExpressionType(ExpressionType::Any, 1)),
							   ExpressionType(ExpressionType::Any, 1));
			break;
		case Operator::_union:
			ret << TypeTriplet(ExpressionType(ExpressionType::List, ExpressionType(ExpressionType::Any, 1)),
							   ExpressionType(ExpressionType::List, ExpressionType(ExpressionType::Any, 1)),
							   ExpressionType(ExpressionType::List, ExpressionType(ExpressionType::Any, 1)));
			break;
		default:
			break;
	}
	return ret;
}

#define TypePair(a,b) ExpressionType(ExpressionType::Lambda).addParameter(a).addParameter(b)

QList<ExpressionType> Operations::inferUnary(Operator::OperatorType op)
{
	QList<ExpressionType> ret;
	switch(op) {
		case Operator::minus:
			ret << TypePair(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			ret << TypePair(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Any, 1), -1), ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Any, 1), -1));
			break;
		case Operator::card:
			ret << TypePair(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Any, 1), -1), ExpressionType(ExpressionType::Value));
			ret << TypePair(ExpressionType(ExpressionType::List, ExpressionType(ExpressionType::Any, 1)), ExpressionType(ExpressionType::Value));
			break;
		case Operator::sum:
		case Operator::product:
			ret << TypePair(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			ret << TypePair(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Any, 1), -1), ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Any, 1), -1));
			break;
		case Operator::factorial:
		case Operator::sin:
		case Operator::cos:
		case Operator::tan:
		case Operator::sec:
		case Operator::csc:
		case Operator::cot:
		case Operator::sinh:
		case Operator::cosh:
		case Operator::tanh:
		case Operator::sech:
		case Operator::csch:
		case Operator::coth:
		case Operator::arcsin:
		case Operator::arccos:
		case Operator::arctan:
		case Operator::arccot:
		case Operator::arcsinh:
		case Operator::arccosh:
		case Operator::arccsc:
		case Operator::arccsch:
		case Operator::arcsec:
		case Operator::arcsech:
		case Operator::arctanh:
		case Operator::exp:
		case Operator::ln:
		case Operator::log:
		case Operator::abs:
		//case Object::conjugate:
		//case Object::arg:
		//case Object::real:
		//case Object::imaginary:
		case Operator::floor:
		case Operator::ceiling:
			ret << TypePair(ExpressionType(ExpressionType::Value), ExpressionType(ExpressionType::Value));
			break;
		case Operator::_not:
		case Operator::exists:
		case Operator::forall:
			ret << TypePair(ExpressionType(ExpressionType::Bool), ExpressionType(ExpressionType::Bool));
			break;
		default:
			break;
	}
	return ret;
}

Object* Operations::reduceCustomCustom(Operator::OperatorType op, CustomObject* v1, CustomObject* v2, QString** )
{
	switch(op) {
		case Operator::neq:
			return new Cn(v1->value()!=v2->value());
		case Operator::eq:
			return new Cn(v1->value()==v2->value());
		default:
			break;
	}
	
	Q_ASSERT(false && "not implemented, please report");
	return 0;
}

Operations::BinaryOp Operations::opsBinary[Object::custom+1][Object::custom+1] = {
	{0,0,0,0,0,0,0,0,0},
	{0, (Operations::BinaryOp) reduceRealReal, 0, (Operations::BinaryOp) reduceRealVector, (Operations::BinaryOp) reduceRealList,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0, (Operations::BinaryOp) reduceVectorReal, 0, (Operations::BinaryOp) reduceVectorVector, 0,0,0,0},
	{0, 0, 0,0, (Operations::BinaryOp) reduceListList, 0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,(Operations::BinaryOp) reduceCustomCustom}
};

Object * Operations::reduce(Operator::OperatorType op, Object * val1, Object * val2, QString** correct)
{
	Object::ObjectType t1=val1->type(), t2=val2->type();
	
	BinaryOp f=opsBinary[t1][t2];
	Q_ASSERT(f);
	return f(op, val1, val2, correct);
}

Operations::UnaryOp Operations::opsUnary[] = {
	0,
	(Operations::UnaryOp) Operations::reduceUnaryReal,
	0, //variable
	(Operations::UnaryOp) Operations::reduceUnaryVector,
	(Operations::UnaryOp) Operations::reduceUnaryList,
	0,0,0
};

Object * Operations::reduceUnary(Operator::OperatorType op, Object * val, QString** correct)
{
	UnaryOp f=opsUnary[val->type()];
	
	Q_ASSERT(f && "using reduceUnary in a wrong way");
	return f(op, val, correct);
}
