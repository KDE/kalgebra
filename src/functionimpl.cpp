/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
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

#include "functionimpl.h"
#include "variables.h"
#include "expression.h"
#include "functionfactory.h"

#include <KLocale>
#include <KDebug>
#include <container.h>

using std::acos;
using std::atan;
using std::fabs;
using std::cos;
using std::sin;
using std::sqrt;

FunctionImpl::FunctionImpl(const Expression& newFunc, Variables* v)
	: points(), func(v), m_deriv(0)
{
	func.setExpression(newFunc);
	if(func.isCorrect()) {
		Expression deriv = func.derivative();
		if(func.isCorrect())
			m_deriv = new Expression(deriv);
	}
	
	func.flushErrors();
}

FunctionImpl::FunctionImpl(const FunctionImpl& fi)
	: points(), func(fi.func.variables()), m_deriv(0)
{
// 	Q_ASSERT(fi.isCorrect());
	func.setExpression(fi.func.expression());
	if(fi.m_deriv)
		m_deriv = new Expression(*fi.m_deriv);
}

FunctionImpl::~FunctionImpl()
{
	points.clear();
	
	delete m_deriv;
}

bool isSimilar(const double &a, const double &b, const double& diff=0.0001)
{
	return a<b+diff && a>b-diff;
}

bool FunctionImpl::addValue(const QPointF& p)
{
	int count=points.count();
	if(count<2) {
		points.append(p);
		return false;
	}
	
	bool appended;
	double slope1=(points[count-1].x()-p.x())/(points[count-1].y()-p.y());
	double slope2=(points[count-2].x()-p.x())/(points[count-2].y()-p.y());
	if(isSimilar(slope1, slope2) || (p.y()==points[count-1].y() && p.y()==points[count-2].y())) {
		points.last()=p;
		appended=false;
	} else {
		points.append(p);
		appended=true;
	}
	return appended;
}

double calcExp(const Expression& exp, Variables* v, double defaultValue)
{
	Expression r;
	if(exp.isCorrect())
	{
		Analitza d(v);
		d.setExpression(exp);
		r=d.calculate();
	}
	
	if(r.isCorrect() && r.isReal())
		return r.toReal().value();
	else
		return defaultValue;
}

double FunctionImpl::downlimit(double defaultValue) const
{
	return calcExp(func.expression().downlimit(), func.variables(), defaultValue);
}

double FunctionImpl::uplimit(double defaultValue) const
{
	return calcExp(func.expression().uplimit(), func.variables(), defaultValue);
}
