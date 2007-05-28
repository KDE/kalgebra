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

#include "operatorsmodel.h"
#include "operator.h"
#include <KLocale>
#include <KApplication>

OperatorsModel::OperatorsModel(int num, QObject *parent) : QStandardItemModel(num, 3, parent), m_count(Object::nOfOps)
{
	setHeaderData(0, Qt::Horizontal, i18n("Name"));
	setHeaderData(1, Qt::Horizontal, i18n("Description"));
	setHeaderData(2, Qt::Horizontal, i18n("Parameters"));
	
	for (int i=1; i<m_count; ++i) {
		addEntry(i, Operator::m_words[i], description((Object::OperatorType) i), example((Object::OperatorType) i));
	}
}


void OperatorsModel::addEntry(int i, const QString &name, const QString &value, const QString &ex)
{
	QFont f = KApplication::font();
	f.setItalic(true);
	QStandardItem *descr = new QStandardItem(value);
	descr->setFont(f);
		
	setData(index(i-1, 0), name);
	setItem(i-1, 1, descr);
	setData(index(i-1, 2), ex);
}

QString OperatorsModel::example(Object::OperatorType o)
{
	int op = Operator::nparams(o);
	QString funcname=Operator::m_words[o];
	if(op == -1) {
		return QString("%1(..., parameters, ...)").arg(funcname);
	} else {
		QString sample = QString("%1(").arg(funcname);
		for(int i=0; i<op; ++i) {
			sample += QString("par%1").arg(i+1);
			if(i<op-1)
				sample += ", ";
		}
		return sample+')';
	}
}

QString OperatorsModel::description(Object::OperatorType o)
{
	QString s;
	switch(o) {
		case Object::plus:
			s = i18n("Addition");
			break;
		case Object::times:
			s = i18n("Multiplication");
			break;
		case Object::divide:
			s = i18n("Division");
			break;
		case Object::minus:
			s = i18n("Subtraction");
			break;
		case Object::power:
			s = i18n("Power");
			break;
		case Object::rem:
			s = i18n("Reminder");
			break;
		case Object::quotient:
			s = i18n("Quotient");
			break;
		case Object::factorof:
			s = i18n("The factor of");
			break;
		case Object::factorial:
			s = i18n("Factorial. factorial(n)=n!");
			break;
		case Object::sin:
			s = i18n("Sine");
			break;
		case Object::cos:
			s = i18n("Cosine");
			break;
		case Object::tan:
			s = i18n("Tangent");
			break;
		case Object::sec:
			s = i18n("Secant");
			break;
		case Object::csc:
			s = i18n("Cosecant");
			break;
		case Object::cot:
			s = i18n("Cotangent");
			break;
		case Object::sinh:
			s = i18n("Hyperbolic sinus");
			break;
		case Object::cosh:
			s = i18n("Hyperbolic sinus");
			break;
		case Object::tanh:
			s = i18n("Hyperbolic tangent");
			break;
		case Object::sech:
			s = i18n("Hyperbolic secant");
			break;
		case Object::csch:
			s = i18n("Hyperbolic cosecant");
			break;
		case Object::coth:
			s = i18n("Hyperbolic cotangent");
			break;
		case Object::arcsin:
			s = i18n("Arc sine");
			break;
		case Object::arccos:
			s = i18n("Arc cosine");
			break;
		case Object::arctan:
			s = i18n("Arc tangent");
			break;
		case Object::arccot:
			s = i18n("Arc cotangent");
			break;
		case Object::arccoth:
			s = i18n("Hyperbolic arc cotangent");
			break;
		case Object::arctanh:
			s = i18n("Hyperbolic arc tangent");
			break;
		case Object::sum:
			s = i18n("Summatory");
			break;
		case Object::product:
			s = i18n("Productory");
			break;
		case Object::diff:
			s = i18n("Differentiation");
			break;
		case Object::arcsinh:
			s = i18n("Hyperbolic arc sine");
			break;
		case Object::arccosh:
			s = i18n("Hyperbolic arc cosine");
			break;
		case Object::arccsc:
			s = i18n("Arc cosecant");
			break;
		case Object::arccsch:
			s = i18n("Hyperbolic arc cosecant");
			break;
		case Object::arcsec:
			s = i18n("Arc secant");
			break;
		case Object::arcsech:
			s = i18n("Hyperbolic arc secant");
			break;
		case Object::exp:
			s = i18n("Exponent (e^x)");
			break;
		case Object::ln:
			s = i18n("Base-e logarithm");
			break;
		case Object::log:
			s = i18n("Base-10 logarithm");
			break;
		case Object::abs:
			s = i18n("Absolute value. abs(n)=|n|");
			break;
		case Object::conjugate:
			s = i18n("Conjugate");
			break;
		case Object::arg:
			s = "---";//i18n("Arg?");
			break;
		case Object::real:
			s = i18n("Real");
			break;
		case Object::imaginary:
			s = i18n("Imaginary");
			break;
		case Object::floor:
			s = i18n("Floor value. floor(n)=⌊n⌋");
			break;
		case Object::ceiling:
			s = i18n("Ceil value. abs(n)=⌈n⌉");
			break;
		case Object::min:
			s = i18n("Minimum");
			break;
		case Object::max:
			s = i18n("Maximum");
			break;
		case Object::gt:
			s = i18n("Greater than. gt(a,b)=a>b");
			break;
		case Object::lt:
			s = i18n("Less than. lt(a,b)=a<b");
			break;
		case Object::eq:
			s = i18n("Equal. eq(a,b) = a=b");
			break;
		case Object::approx:
			s = i18n("Approximation approx(a)=a±n");
			break;
		case Object::neq:
			s = i18n("Greater than. neq(a,b)=a≠b");
			break;
		case Object::geq:
			s = i18n("Greater or equal. gt(a,b)=a>b");
			break;
		case Object::leq:
			s = i18n("Less or equal. gt(a,b)=a>b");
			break;
		case Object::_and:
			s = i18n("Boolean and");
			break;
		case Object::_not:
			s = i18n("Boolean not");
			break;
		case Object::_or:
			s = i18n("Boolean or");
			break;
		case Object::_xor:
			s = i18n("Boolean xor");
			break;
		case Object::implies:
			s = i18n("Boolean implication");
			break;
		case Object::gcd:
			s = i18n("Greatest common divisor");
			break;
		case Object::lcm:
			s = i18n("Least common multiple");
			break;
		case Object::root:
			s = i18n("Root");
			break;
		default:
			break;
	}
	return s;
}


/*QString OperatorsModel::operToString(const Operator& op) const
{
	QStandardItem *it;
	
	for(int i=0; i<KEYWORDNUM; i++) {
		it=item(i,2);
		if(it!=NULL && it->data(Qt::EditRole).toInt()==op.operatorType()) {
			return item(i,0)->data(Qt::EditRole).toString();
}
}
	return QString();
}*/

