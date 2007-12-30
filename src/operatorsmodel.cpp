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
#include "variables.h"
#include <KLocale>
#include <KApplication>
#include <QFont>

OperatorsModel::OperatorsModel(QObject *parent) : QAbstractTableModel(parent), m_vars(0)
{
}

QVariant OperatorsModel::data(const QModelIndex & index, int role) const
{
	QVariant ret;
	if(role==Qt::DisplayRole) {
		if(index.row()<Operator::nOfOps-2) {
			Operator oper((Operator::OperatorType) (index.row()+1));
			switch(index.column()) {
				case 0:
					ret=oper.toString();
					break;
				case 1:
					ret=description(oper);
					break;
				case 2:
					ret=example(oper);
					break;
			}
		} else if(m_vars) {
			int var=index.row()-Operator::nOfOps+2;
			QString key=m_vars->keys()[var];
			switch(index.column()) {
				case 0:
					ret=key;
					break;
				case 1:
					ret=m_vars->value(key)->toString();
					break;
			}
		}
	} else if(role==Qt::FontRole && index.column()==1) {
		QFont f = KApplication::font();
		f.setItalic(true);
		ret=f;
	}
	return ret;
}

QVariant OperatorsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant ret;
	if(role==Qt::DisplayRole && orientation==Qt::Horizontal) {
		switch(section) {
			case 0:
				ret=i18nc("@title:column", "Name");
				break;
			case 1:
				ret=i18nc("@title:column", "Description");
				break;
			case 2:
				ret=i18nc("@title:column", "Parameters");
				break;
		}
	}
	return ret;
}

int OperatorsModel::rowCount(const QModelIndex &) const
{
	int count=Operator::nOfOps;
	if(m_vars)
		count+=m_vars->count();
	return count-2;
}

int OperatorsModel::columnCount(const QModelIndex &) const
{
	return 3;
}

void OperatorsModel::updateInformation()
{
	reset();
}

/*void OperatorsModel::addEntry(int i, const QString &name, const QString &value, const QString &ex)
{
	QFont f = KApplication::font();
	f.setItalic(true);
	QStandardItem *descr = new QStandardItem(value);
	descr->setFont(f);
	
	setData(index(i-1, 0), name);
	setItem(i-1, 1, descr);
	setData(index(i-1, 2), ex);
}*/

QString OperatorsModel::example(Operator oper)
{
	QString funcname=oper.toString();
	QString bounds;
	if(oper.isBounded()) {
		bounds="x->from..to";
	}
	
	QString sample = i18n("%1(", funcname);
	if(!bounds.isEmpty()) {
		sample += i18n("%1, ", bounds);
	}
	
	if(oper.nparams()<0) {
		return i18n("%1... parameters, ...)", sample);
	} else {
		for(int i=0; i<oper.nparams(); ++i) {
			sample += i18n("par%1", i+1);
			if(i<oper.nparams()-1)
				sample += ", ";
		}
		return sample+')';
	}
}

QString OperatorsModel::description(Operator o)
{
	QString s;
	switch(o.operatorType()) {
		case Operator::plus:
			s = i18n("Addition");
			break;
		case Operator::times:
			s = i18n("Multiplication");
			break;
		case Operator::divide:
			s = i18n("Division");
			break;
		case Operator::minus:
			s = i18n("Subtraction");
			break;
		case Operator::power:
			s = i18n("Power");
			break;
		case Operator::rem:
			s = i18n("Remainder");
			break;
		case Operator::quotient:
			s = i18n("Quotient");
			break;
		case Operator::factorof:
			s = i18n("The factor of");
			break;
		case Operator::factorial:
			s = i18n("Factorial. factorial(n)=n!");
			break;
		case Operator::sin:
			s = i18n("Function to calculate the sine of a given angle");
			break;
		case Operator::cos:
			s = i18n("Function to calculate the cosine of a given angle");
			break;
		case Operator::tan:
			s = i18n("Function to calculate the tangent of a given angle");
			break;
		case Operator::sec:
			s = i18n("Secant");
			break;
		case Operator::csc:
			s = i18n("Cosecant");
			break;
		case Operator::cot:
			s = i18n("Cotangent");
			break;
		case Operator::sinh:
			s = i18n("Hyperbolic sine");
			break;
		case Operator::cosh:
			s = i18n("Hyperbolic cosine");
			break;
		case Operator::tanh:
			s = i18n("Hyperbolic tangent");
			break;
		case Operator::sech:
			s = i18n("Hyperbolic secant");
			break;
		case Operator::csch:
			s = i18n("Hyperbolic cosecant");
			break;
		case Operator::coth:
			s = i18n("Hyperbolic cotangent");
			break;
		case Operator::arcsin:
			s = i18n("Arc sine");
			break;
		case Operator::arccos:
			s = i18n("Arc cosine");
			break;
		case Operator::arctan:
			s = i18n("Arc tangent");
			break;
		case Operator::arccot:
			s = i18n("Arc cotangent");
			break;
		case Operator::arccoth:
			s = i18n("Hyperbolic arc cotangent");
			break;
		case Operator::arctanh:
			s = i18n("Hyperbolic arc tangent");
			break;
		case Operator::sum:
			s = i18n("Summatory");
			break;
		case Operator::product:
			s = i18n("Productory");
			break;
		case Operator::diff:
			s = i18n("Differentiation");
			break;
		case Operator::arcsinh:
			s = i18n("Hyperbolic arc sine");
			break;
		case Operator::arccosh:
			s = i18n("Hyperbolic arc cosine");
			break;
		case Operator::arccsc:
			s = i18n("Arc cosecant");
			break;
		case Operator::arccsch:
			s = i18n("Hyperbolic arc cosecant");
			break;
		case Operator::arcsec:
			s = i18n("Arc secant");
			break;
		case Operator::arcsech:
			s = i18n("Hyperbolic arc secant");
			break;
		case Operator::exp:
			s = i18n("Exponent (e^x)");
			break;
		case Operator::ln:
			s = i18n("Base-e logarithm");
			break;
		case Operator::log:
			s = i18n("Base-10 logarithm");
			break;
		case Operator::abs:
			s = i18n("Absolute value. abs(n)=|n|");
			break;
		case Operator::conjugate:
			s = i18n("Conjugate");
			break;
		case Operator::arg:
			s = "---";//i18n("Arg?");
			break;
		case Operator::real:
			s = i18n("Real");
			break;
		case Operator::imaginary:
			s = i18n("Imaginary");
			break;
		case Operator::floor:
			s = i18n("Floor value. floor(n)=⌊n⌋");
			break;
		case Operator::ceiling:
			s = i18n("Ceil value. ceil(n)=⌈n⌉");
			break;
		case Operator::min:
			s = i18n("Minimum");
			break;
		case Operator::max:
			s = i18n("Maximum");
			break;
		case Operator::gt:
			s = i18n("Greater than. gt(a,b)=a>b");
			break;
		case Operator::lt:
			s = i18n("Less than. lt(a,b)=a<b");
			break;
		case Operator::eq:
			s = i18n("Equal. eq(a,b) = a=b");
			break;
		case Operator::approx:
			s = i18n("Approximation approx(a)=a±n");
			break;
		case Operator::neq:
			s = i18n("Not equal. neq(a,b)=a≠b");
			break;
		case Operator::geq:
			s = i18n("Greater or equal. geq(a,b)=a≥b");
			break;
		case Operator::leq:
			s = i18n("Less or equal. leq(a,b)=a≤b");
			break;
		case Operator::_and:
			s = i18n("Boolean and");
			break;
		case Operator::_not:
			s = i18n("Boolean not");
			break;
		case Operator::_or:
			s = i18n("Boolean or");
			break;
		case Operator::_xor:
			s = i18n("Boolean xor");
			break;
		case Operator::implies:
			s = i18n("Boolean implication");
			break;
		case Operator::gcd:
			s = i18n("Greatest common divisor");
			break;
		case Operator::lcm:
			s = i18n("Least common multiple");
			break;
		case Operator::root:
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


