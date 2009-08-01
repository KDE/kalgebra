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
#include "variables.h"

#include <KLocale>

struct FunctionPolar : public FunctionImpl
{
	FunctionPolar(const Expression &e, Variables* v) : FunctionImpl(e, v) {}
	FunctionPolar(const FunctionPolar &fp) : FunctionImpl(fp) {}
	
	void updatePoints(const QRect& viewport, unsigned int resolution);
	QPair<QPointF, QString> calc(const QPointF& dp);
	function::Axe axeType() const { return function::Polar; }
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionPolar(*this); }
	
	inline QPointF fromPolar(double r, double th) { return QPointF(r*std::cos(th), r*std::sin(th)); }
	QRect m_last_viewport;
	static QStringList supportedBVars() { return QStringList("q"); }
};

REGISTER_FUNCTION(FunctionPolar)
static const double pi=acos(-1.);

void FunctionPolar::updatePoints(const QRect& viewport, unsigned int max_res)
{
	Q_UNUSED(viewport);
	Q_ASSERT(func.expression().isCorrect());
	if(int(max_res)==points.capacity())
		return;
	unsigned int resolution=max_res;
	
	double ulimit=uplimit(2*pi);
	double dlimit=downlimit(0);
	
	if(ulimit<=dlimit) {
		m_err += i18n("Cannot have downlimit ≥ uplimit");
		return;
	}
	
	points.clear();
	points.reserve(max_res);
	
	func.variables()->modify("q", 0.);
	Cn *varth = (Cn*) func.variables()->value("q");
	
	double inv_res= double((ulimit-dlimit)/resolution);
	double final=ulimit-inv_res;
	for(double th=dlimit; th<final; th+=inv_res) {
		varth->setValue(th);
		double r = func.calculate().value().value();
		
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
	
	const Expression& e = func.expression();
	Expression ulimitexp = e.uplimit(), dlimitexp=e.downlimit();
	Cn ulimit, dlimit;
	if(ulimitexp.isCorrect()) {
		Analitza u(func.variables());
		u.setExpression(ulimitexp);
		ulimitexp=u.calculate();
	}
	if(!ulimitexp.isCorrect())
		ulimit = 2.*pi;
	
	if(dlimitexp.isCorrect()) {
		Analitza d(func.variables());
		d.setExpression(dlimitexp);
		dlimitexp=d.calculate();
	}
	if(!dlimitexp.isCorrect())
		dlimit = 0.;
	
	if(th<dlimit.value()) th=dlimit.value();
	if(th>ulimit.value()) th=ulimit.value();
	
	QPointF dist;
	do {
		func.variables()->modify("q", th);
		r = func.calculate().value().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		func.variables()->modify("q", th+2.*pi);
		r = func.calculate().value().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d2 = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		th += 2.*pi;
	} while(d>d2);
	
	func.variables()->modify("q", th);
	r = func.calculate().value().value();
	dp = fromPolar(r, th);
	pos = QString("r=%1 th=%2").arg(r,3,'f',2).arg(th,3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QLineF FunctionPolar::derivative(const QPointF& p) const
{
	Q_UNUSED(p);
	return QLineF();
}
