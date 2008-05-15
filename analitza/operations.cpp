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

#include <math.h>

#include "operations.h"
#include <KLocale>
#include <cmath>


#include "value.h"
#include "container.h"
#include "expression.h"

using namespace std;

Cn* Operations::reduceRealReal(enum Operator::OperatorType op, Cn *oper, const Cn *oper1, bool &correct)
{
	int residu;
	double a=oper->value(), b=oper1->value(), c;
	Cn::ValueFormat format=oper->format();
	
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
			if(floor(b)!=0.)
				a = remainder(a, b);
			else
				correct=false;
			break;
		case Operator::quotient:
			a = floor(a / b);
			format=Cn::Integer;
			break;
		case Operator::factorof:
			if(floor(b)!=0.)
				a = (((int)a % (int)b)==0) ? 1.0 : 0.0;
			else {
				a = 0.;
				correct=false;
			}
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
		case Operator::gcd: //code by michael cane aka kiko :)
			while (b > 0.) {
				residu = (int) floor(a) % (int) floor(b);
				a = b;
				b = residu;
			}
			format=Cn::Integer;
			break;
		case Operator::lcm: //code by michael cane aka kiko :)
			c=a*b;
			while (b > 0.) {
				residu = (int) floor(a) % (int) floor(b);
				a = b;
				b = residu;
			}
			a=(int)c/(int)a;
			format=Cn::Integer;
			break;
		case Operator::root:
			a = b==2.0 ? sqrt(a) : pow(a, 1.0/b);
			break;
		default:
			correct=false;
			break;
	}
	oper->setValue(a);
	oper->setFormat(format);
	delete oper1;
	return oper;
}

Cn* Operations::reduceUnaryReal(enum Operator::OperatorType op, Cn *val, bool &correct)
{
	double a=val->value();
	Cn::ValueFormat format=val->format();
	
	switch(op) {
		case Operator::minus:
			a = -a;
			break;
		case Operator::factorial: {
			int b = static_cast<int>(floor(a));
			for(a=1.; b>1.; b--) {
				a*=b;
			}
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
			a=acos(a);
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
// 		case Operator::arccsc:
// 		case Operator::arccsch:
// 		case Operator::arcsec:
// 		case Operator::arcsech:
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
			correct=false;
			break;
	}
	
	val->setValue(a);
	val->setFormat(format);
	return val;
}

Object * Operations::reduce(Operator::OperatorType op, Object * val1, Object * val2, bool &correct)
{
	Object::ValueType t1=val1->valueType(), t2=val2->valueType();
	correct=true;
	
	if(t1==Object::Real && t2==Object::Real) return reduceRealReal(op, (Cn*) val1, (Cn*) val2, correct);
	if(t1==Object::Real && t2==Object::Vector) return reduceRealVector(op, (Cn*) val1, (Container*) val2, correct);
	if(t1==Object::Vector && t2==Object::Real) return reduceVectorReal(op, (Container*) val1, (Cn*) val2, correct);
	if(t1==Object::Vector && t2==Object::Vector) return reduceVectorVector(op, (Container*) val1, (Container*) val2, correct);
	Q_ASSERT(0);
	return 0;
}

Object * Operations::reduceUnary(Operator::OperatorType op, Object * val, bool &correct)
{
	correct=true;
	switch(val->valueType()) {
		case Object::Real:
			return reduceUnaryReal(op, (Cn*) val, correct);
		case Object::Vector:
			return reduceUnaryVector(op, (Container*) val, correct);
		case Object::Null:
			break;
	}
	
	Q_ASSERT(0);
	return 0;
}

Object * Operations::reduceRealVector(Operator::OperatorType op, Cn * oper, Container * v1, bool& correct)
{
	for(Container::iterator it=v1->m_params.begin(); it!=v1->m_params.end(); ++it)
	{
		*it=reduce(op, new Cn(oper), *it, correct);
	}
	
	delete oper;
	return v1;
}

Object * Operations::reduceVectorReal(Operator::OperatorType op, Container * v1, Cn * oper, bool &correct)
{
	for(Container::iterator it=v1->m_params.begin(); it!=v1->m_params.end(); ++it)
	{
		*it=reduce(op, *it, new Cn(oper), correct);
	}
	delete oper;
	return v1;
}

Object * Operations::reduceVectorVector(Operator::OperatorType op, Container * v1, Container * v2, bool &correct)
{
	Q_ASSERT(v1->m_params.count()==v2->m_params.count());
	if(op==Operator::scalarproduct)
		op=Operator::times;
	Container::iterator it2=v2->m_params.begin();
	for(Container::iterator it1=v1->m_params.begin(); it1!=v1->m_params.end(); ++it1, ++it2)
	{
		*it1=reduce(op, *it1, *it2, correct);
		v2->m_params.erase(it2);
	}
	delete v2;
	return v1;
}

Object * Operations::reduceUnaryVector(Operator::OperatorType op, Container * c, bool &correct)
{
	Object *ret=0;
	switch(op) {
		case Operator::card:
			ret=new Cn(double(c->m_params.count()));
			break;
		default:
			correct=false;
			ret=new Cn(0.);
			break;
	}
	delete c;
	return ret;
}
