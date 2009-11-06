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

#include <KLocale>
#include <cmath>
#include <kdemacros.h>

#include "value.h"
#include "vector.h"
#include "expression.h"
#include "list.h"

using namespace std;
using namespace Analitza;

Cn* Operations::reduceRealReal(enum Operator::OperatorType op, Cn *oper, const Cn *oper1, QString &correct)
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
			if(KDE_ISLIKELY(floor(b)!=0.))
				a = remainder(a, b);
			else
				correct=i18n("Cannot calculate the remainder on 0.");
			break;
		case Operator::quotient:
			a = floor(a / b);
			format=Cn::Integer;
			break;
		case Operator::factorof:
			if(KDE_ISLIKELY(floor(b)!=0.))
				a = ((int(floor(a)) % int(floor(b)))==0) ? 1.0 : 0.0;
			else
				correct=i18n("Cannot calculate the factor on 0.");
			
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
				correct=i18n("Cannot calculate the lcm of 0.");
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

Cn* Operations::reduceUnaryReal(enum Operator::OperatorType op, Cn *val, QString &correct)
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
			correct=i18n("Could not calculate a value %1", Operator(op).toString());
			break;
	}
	
	val->setValue(a);
	val->setFormat(format);
	return val;
}

Object * Operations::reduce(Operator::OperatorType op, Object * val1, Object * val2, QString &correct)
{
	Object::ObjectType t1=val1->type(), t2=val2->type();
	
	if(t1==Object::value && t2==Object::value) return reduceRealReal(op, (Cn*) val1, (Cn*) val2, correct);
	if(t1==Object::value && t2==Object::vector) return reduceRealVector(op, (Cn*) val1, (Vector*) val2, correct);
	if(t1==Object::vector && t2==Object::value) return reduceVectorReal(op, (Vector*) val1, (Cn*) val2, correct);
	if(t1==Object::vector && t2==Object::vector) return reduceVectorVector(op, (Vector*) val1, (Vector*) val2, correct);
	if(t1==Object::list && t2==Object::list) return reduceListList(op, (List*) val1, (List*) val2, correct);
	if(t1==Object::value && t2==Object::list) return reduceRealList(op, (Cn*) val1, (List*) val2, correct);
	
	correct = i18n("Could not reduce '%1' and '%2'.", val1->toString(), val2->toString());
	return 0;
}

Object * Operations::reduceUnary(Operator::OperatorType op, Object * val, QString &correct)
{
	switch(val->type()) {
		case Object::value:
			return reduceUnaryReal(op, (Cn*) val, correct);
		case Object::vector:
			return reduceUnaryVector(op, (Vector*) val, correct);
		case Object::list:
			return reduceUnaryList(op, (List*) val, correct);
		case Object::none:
		case Object::variable:
		case Object::oper:
		case Object::container:
			//This should never happen
			break;
	}
	
	Q_ASSERT(false && "using reduceUnary in a wrong way");
	return 0;
}

Object * Operations::reduceRealVector(Operator::OperatorType op, Cn * oper, Vector * v1, QString& correct)
{
	switch(op) {
		case Operator::selector: {
			int select=oper->intValue();
			Object* ret=0;
			if(select<1 || (select-1) >= v1->size()) {
				correct=i18n("Invalid index for a container");
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

Object * Operations::reduceVectorReal(Operator::OperatorType op, Vector * v1, Cn * oper, QString &correct)
{
	for(Vector::iterator it=v1->begin(); it!=v1->end(); ++it)
	{
		*it=reduce(op, *it, new Cn(*oper), correct);
	}
	delete oper;
	return v1;
}

Object * Operations::reduceVectorVector(Operator::OperatorType op, Vector * v1, Vector * v2, QString &correct)
{
	if(v1->size()!=v2->size()) {
		correct=i18n("Cannot operate on different sized vectors.");
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

Object * Operations::reduceUnaryVector(Operator::OperatorType op, Vector * c, QString &correct)
{
	Object *ret=0;
	switch(op) {
		case Operator::card:
			ret=new Cn(c->size());
			break;
		default:
			correct=i18n("Could not calculate a vector's %1", Operator(op).toString());
			ret=new Cn(0.);
			break;
	}
	delete c;
	return ret;
}

Object* Operations::reduceListList(Operator::OperatorType op, List* l1, List* l2, QString &correct)
{
	Object* ret=0;
	switch(op) {
		case Operator::_union: {
			List::iterator itEnd=l2->end();
			for(List::iterator it=l2->begin(); it!=itEnd;) {
				l1->appendBranch(*it);
				it=l2->erase(it);
			}
			
// 			delete l2;
			ret=l1;
		}	break;
		default:
			correct=i18n("Could not calculate a list's %1", Operator(op).toString());
			ret=new Cn(0.);
			break;
	}
	return ret;
}

Object* Operations::reduceUnaryList(Operator::OperatorType op, List* l, QString &correct)
{
	Object *ret=0;
	switch(op) {
		case Operator::card:
			ret=new Cn(l->size());
			break;
		default:
			correct=i18n("Could not calculate a list's %1", Operator(op).toString());
			ret=new Cn(0.);
			break;
	}
	delete l;
	return ret;
}

Object* Operations::reduceRealList(Operator::OperatorType op, Cn* oper, List* v1, QString& correct)
{
	switch(op) {
		case Operator::selector: {
			int select=oper->intValue();
			Object* ret=0;
			if(select<1 || (select-1) >= v1->size()) {
				correct=i18n("Invalid index for a container");
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
