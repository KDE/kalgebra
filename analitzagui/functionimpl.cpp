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
#include "analitza/variables.h"
#include "analitza/expression.h"
#include "functionfactory.h"
#include <cmath>

#include <KDebug>

using std::acos;
using std::atan;
using std::fabs;
using std::cos;
using std::sin;
using std::sqrt;

using Analitza::Expression;
using Analitza::Variables;

FunctionImpl::FunctionImpl(const Expression& newFunc, Variables* v, double defDl, double defUl)
	: points(), func(v), m_deriv(0), m_res(0), mUplimit(defUl), mDownlimit(defDl)
{
	func.setExpression(newFunc);
	func.simplify();
	
	func.flushErrors();
}

FunctionImpl::FunctionImpl(const FunctionImpl& fi)
	: points(), func(fi.func.variables()), m_deriv(0), m_res(fi.m_res)
	, mUplimit(fi.mUplimit), mDownlimit(fi.mDownlimit)
{
// 	Q_ASSERT(fi.isCorrect());
	func.setExpression(fi.func.expression());
	if(fi.m_deriv)
		m_deriv = new Expression(*fi.m_deriv);
}

FunctionImpl::~FunctionImpl()
{
	delete m_deriv;
}

bool FunctionImpl::isSimilar(double a, double b, double diff)
{
	return fabs(a-b)<diff;
}

bool FunctionImpl::addValue(const QPointF& p)
{
	int count=points.count();
	if(count<2) {
		points.append(p);
		return false;
	}
	
	//TODO: Think of some optimization if needed
	bool appended;
	double slope1=(points[count-1].y()-points[count-2].y())/(points[count-1].x()-points[count-2].x());
	double slope2=(points[count-1].y()-p.y())/(points[count-1].x()-p.x());
	if(isSimilar(slope1, slope2) || (p.y()==points[count-1].y() && p.y()==points[count-2].y())) {
// 		qDebug() << "join" << points[count-2] << points[count-1] << p;
// 		qDebug() << "because" << slope1 << slope2 << isSimilar(slope1, slope2);
		points.last()=p;
		appended=false;
	} else {
		points.append(p);
		appended=true;
	}
	return appended;
}

void FunctionImpl::setResolution(uint res)
{
	Q_ASSERT(res>2);
	
	if(res!=m_res) {
		points.clear();
		m_jumps.clear();
	}
	m_res=res;
}

double FunctionImpl::uplimit() const
{
	return mUplimit;
}

double FunctionImpl::downlimit() const
{
	return mDownlimit;
}

void FunctionImpl::setLimits(double d, double u)
{
	Q_ASSERT(u>=d);
	mUplimit=u;
	mDownlimit=d;
}
