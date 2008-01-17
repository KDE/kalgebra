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

#include "value.h"
#include "operator.h"

#include <QDomElement>

#include <KLocale>

Cn::Cn(Object const *o)
	: Object(Object::value), m_value(0.), m_boolean(false)
{
	if(o->type()==Operator::value){
		Cn *v = (Cn*) o;
		m_value = v->value();
		m_boolean = v->isBoolean();
		setCorrect(v->isCorrect());
	} else {
		setCorrect(false);
	}
}

QString Cn::toString() const
{
	if(isBoolean()) {
		if(isTrue())
			return "true";
		else
			return "false";
	} else
		return QString("%1").arg(m_value, 0, 'g', 12);
}

QString Cn::toMathML() const
{
	if(isBoolean()) {
		if(isTrue())
			return "<cn type='constant'>true</cn>";
		else
			return "<cn type='constant'>false</cn>";
	} else
		return QString("<cn>%1</cn>").arg(m_value, 0, 'g', 12);
}

QString Cn::toHtml() const
{
	if(isBoolean()) {
		if(isTrue())
			return i18nc("html representation of a number", "<span class='const'>true</span>");
		else
			return i18nc("html representation of a number", "<span class='const'>false</span>");
	} else
		return i18nc("html representation of a number", "<span class='num'>%1</span>", m_value);
}

/*enum Cn::ValueFormat Cn::whatValueFormat(const QDomElement& val)
{
	enum ValueFormat ret= none;
	QString tag = val.tagName();
	if(tag == "cn"){ // a is a number
		if(val.attribute("type","real") == "real")		ret = real;
		else if(val.attribute("type") == "integer")		ret = integer;
		else if(val.attribute("type") == "e-notation")		ret = real;
		else if(val.attribute("type") == "rational")		ret = real;
		else if(val.attribute("type") == "complex-cartesian")	ret = none;
		else if(val.attribute("type") == "complex-polar")	ret = none;
		else if(val.attribute("type") == "constant") {
			if(val.text() == "&pi;")		ret=real;
			else if (val.text() == "&ee;" || val.text() == "&ExponentialE;")	ret = real;
			else if (val.text() == "&true;")	ret = boolean;
			else if (val.text() == "&false;")	ret = boolean;
			else if (val.text() == "&gamma;")	ret = real;
			//else if (val.text() == "&ImagniaryI;")	; //TODO: Not implemented 
			else if (val.text() == "&infin;")	ret=nan;
			else if (val.text() == "&NaN;")		ret=nan;
		}
	} else if(tag=="true")		ret = boolean;
	else if(tag=="false")		ret = boolean;
	else if(tag=="pi")		ret = real;
	else if(tag=="exponentiale")	ret = real;
	else if(tag=="eulergamma")	ret = real;
	
	return ret;
}*/

void Cn::setValue(const QDomElement& val)
{
// 	this->m_vformat=whatValueFormat(val);
	bool wrong;
	QString tag = val.tagName();
	m_boolean=false;
	
	if(tag == "cn"){ // a is a number
		if(val.attribute("type", "integer") == "real") {
			m_value= val.text().trimmed().toDouble(&wrong); //TODO: Base on double not implemented
		} else if(val.attribute("type", "integer") == "integer"){
			int base = val.attribute("base", "10").toInt(NULL, 10);
			m_value= val.text().trimmed().toInt(&wrong, base);
		}
#if 0
		else if(val.attribute("type") == "e-notation")	{ /*TODO: Not implemented */ }
		else if(val.attribute("type") == "rational")		{ /*TODO: Not implemented */ }
		else if(val.attribute("type") == "complex-cartesian")	{ /*TODO: Not implemented */ }
		else if(val.attribute("type") == "complex-polar")	{ /*TODO: Not implemented */ }
#endif
		else if(val.attribute("type") == "constant"){
			if(val.text() == "&pi;")			{ m_value = pi().m_value; }
			else if (val.text() == "&ee;" || val.text() == "&ExponentialE;"){ m_value = e().m_value; }
			else if (val.text() == "&true;")	{ m_value=1.; m_boolean=true; }
			else if (val.text() == "&false;")	{ m_value=0.; m_boolean=true; }
			else if (val.text() == "&gamma;")	{ m_value = 0.5772156649; }
#if 0
			else if (val.text() == "&ImagniaryI;")	; //TODO: Not implemented 
			else if (val.text() == "&infin;")	; //TODO: Not implemented  }
			else if (val.text() == "&NaN;")		; //TODO: Not implemented  }*/
#endif
		}
	}
}

QString Cn::reduce(enum Operator::OperatorType op, const Cn &oper)
{
	int residu;
	double a=m_value, b=oper.value(), c;
	bool boolean=false;
	
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
				a = static_cast<int>(floor(a)) % static_cast<int>(floor(b));
			else
				return i18n("Cannot calculate the <em>remainder</em> of 0.");
			break;
		case Operator::quotient:
			a = floor(a / b);
			break;
		case Operator::factorof:
			if(floor(b)!=0.)
				a = (((int)a % (int)b)==0) ? 1.0 : 0.0;
			else {
				a = 0.;
				return i18n("Cannot calculate the <em>factor</em> of 0.");
			}
			boolean = true;
			break;
		case Operator::min:
			a= a < b? a : b;
			break;
		case Operator::max:
			a= a > b? a : b;
			break;
		case Operator::gt:
			a= a > b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::lt:
			a= a < b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::eq:
			a= a == b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::approx:
			a= fabs(a-b)<0.001? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::neq:
			a= a != b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::geq:
			a= a >= b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::leq:
			a= a <= b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::_and:
			a= a && b? 1.0 : 0.0;
			boolean=true;
			break;
		case Operator::_not:
			a=!a;
			boolean = true;
			break;
		case Operator::_or:
			a= a || b? 1.0 : 0.0;
			boolean = true;
			break;
		case Operator::_xor:
			a= (a || b) && !(a&&b)? 1.0 : 0.0;
			boolean = true;
			break;
		case Operator::implies:
			a= (a && !b)? 0.0 : 1.0;
			boolean = true;
			break;
		case Operator::gcd: //code by michael cane aka kiko :)
			while (b > 0.) {
				residu = (int) floor(a) % (int) floor(b);
				a = b;
				b = residu;
			}
			break;
		case Operator::lcm: //code by michael cane aka kiko :)
			c=a*b;
			while (b > 0.) {
				residu = (int) floor(a) % (int) floor(b);
				a = b;
				b = residu;
			}
			a=(int)c/(int)a;
			break;
		case Operator::root:
			a = b==2.0 ? sqrt(a) : pow(a, 1.0/b);
			break;
		default:
			return i18n("The operator <em>%1</em> has not been implemented", QString(Operator::m_words[op]));
			break;
	}
	m_value=a;
	m_boolean=boolean;
	return QString();
}

QString Cn::reduceUnary(enum Operator::OperatorType op)
{
	double a=m_value;
	QString error;
	
	switch(op) {
		case Operator::minus:
			a = -a;
			break;
		case Operator::factorial: {
			int b = static_cast<int>(floor(a));
			for(a=1.; b>1.; b--) {
				a*=b;
			}
		}
			break;
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
		// 	case Operator::arccsc:
		// 	case Operator::arccsch:
		// 	case Operator::arcsec:
		// 	case Operator::arcsech:
		// 	case Operator::arcsinh:
		// 	case Operator::arctanh:
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
			break;
		case Operator::ceiling:
			a=ceil(a);
			break;
		default:
			error=i18n("Unary operator '%1' not suported", Operator(op).toString());
			break;
	}
	
	m_value=a;
	
	return error;
}

Cn Cn::pi() { return Cn(3.1415926535897932384626433); }
Cn Cn::e() { return Cn(2.718281828); }
Cn Cn::euler() { return Cn(0.5772156649); }

