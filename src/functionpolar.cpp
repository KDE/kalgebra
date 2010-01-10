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
#include "functionfactory.h"
#include "value.h"

#include <KLocale>

using std::acos;
using std::atan;
using std::sqrt;

using Analitza::Expression;
using Analitza::Variables;
using Analitza::Cn;

struct FunctionPolar : public FunctionImpl
{
	FunctionPolar(const Expression &e, Variables* v) : FunctionImpl(e, v, 0, 2*M_PI) {}
	FunctionPolar(const FunctionPolar &fp) : FunctionImpl(fp) {}
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	function::Axe axeType() const { return function::Polar; }
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionPolar(*this); }
	
	inline QPointF fromPolar(double r, double th) { return QPointF(r*std::cos(th), r*std::sin(th)); }
	QRect m_last_viewport;
	QStringList boundings() const { return supportedBVars(); }
	static QStringList supportedBVars() { return QStringList("q"); }
};

REGISTER_FUNCTION(FunctionPolar)
static const double pi=acos(-1.);

void FunctionPolar::updatePoints(const QRect& viewport)
{
	Q_UNUSED(viewport);
	Q_ASSERT(func.expression().isCorrect());
	if(int(resolution())==points.capacity())
		return;
	
	double ulimit=uplimit();
	double dlimit=downlimit();
	
	points.clear();
	points.reserve(resolution());
	
	Cn *varth=func.insertValueVariable("q", 0.);
	
	double inv_res= double((ulimit-dlimit)/resolution());
	double final=ulimit-inv_res;
	for(double th=dlimit; th<final; th+=inv_res) {
		varth->setValue(th);
		double r = func.calculate().toReal().value();
		
		addValue(fromPolar(r, th));
	}
}

QPair<QPointF, QString> FunctionPolar::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	if(p==QPointF(0., 0.))
		return QPair<QPointF, QString>(dp, i18n("center"));
	double th=atan(p.y()/ p.x()), r=1., d, d2;
	if(p.x()<0.)	th += pi;
	else if(th<0.)	th += 2.*pi;
	
	th=qMax(th, downlimit());
	th=qMin(th, uplimit());
	
	Cn* tth=func.insertValueVariable("q", th);
	QPointF dist;
	do {
		tth->setValue(th);
		r = func.calculate().toReal().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		tth->setValue(th+2.*pi);
		r = func.calculate().toReal().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d2 = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		th += 2.*pi;
	} while(d>d2);
	th -= 2.*pi;
	
	tth->setValue(th);
	Expression res=func.calculate();
	
	if(!res.isReal())
		m_err += i18n("We can only draw Real results.");
	r = res.toReal().value();
	dp = fromPolar(r, th);
	
	pos = QString("r=%1 th=%2").arg(r,3,'f',2).arg(th,3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QLineF FunctionPolar::derivative(const QPointF& p) const
{
	Q_UNUSED(p);
	return QLineF();
}
