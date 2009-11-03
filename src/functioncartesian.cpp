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

#include <KDebug>
#include <KLocale>

using std::sin;
using std::cos;
using std::atan;
using std::fabs;

///Functions where the x is bounding. like x->sin(x)
struct FunctionY : public FunctionImpl
{
	explicit FunctionY(const Expression &e, Variables* v) : FunctionImpl(e, v ,0,0) {}
	FunctionY(const FunctionY &fy) : FunctionImpl(fy) {}
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionY(*this); }
	static QStringList supportedBVars() { return QStringList("x"); }
	
	QStringList boundings() const { return supportedBVars(); }
	
	void calculateValues(double, double);
};

///Functions where the y is bounding. like y->sin(y). FunctionY mirrored
struct FunctionX : public FunctionY
{
	explicit FunctionX(const Expression &e, Variables* v) : FunctionY(e, v) {}
	FunctionX(const FunctionX &fx) : FunctionY(fx) {}
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionX(*this); }
	
	static QStringList supportedBVars() { return QStringList("y"); }
	QStringList boundings() const { return supportedBVars(); }
};

REGISTER_FUNCTION(FunctionY)
REGISTER_FUNCTION(FunctionX)

namespace
{
	/// The @p p1 and @p p2 parameters are the last 2 values found
	/// @p next is the next value found
	///	@returns whether we've found the gap
	
	bool traverse(double p1, double p2, double next)
	{
		static const double delta=3;
		double diff=p2-p1, diff2=next-p2;
		bool ret=false;
		
		if(diff>0 && diff2<-delta)
			ret=true;
		else if(diff<0 && diff2>delta)
			ret=true;
		
		return ret;
	}

	QLineF slopeToLine(const double &der)
	{
		double arcder = atan(der);
		const double len=3.*der;
		QPointF from, to;
		from.setX(len*cos(arcder));
		from.setY(len*sin(arcder));

		to.setX(-len*cos(arcder));
		to.setY(-len*sin(arcder));
		return QLineF(from, to);
	}

	QLineF mirrorXY(const QLineF& l)
	{
		return QLineF(l.y1(), l.x1(), l.y2(), l.x2());
	}
}

void FunctionY::calculateValues(double l_lim, double r_lim)
{
	m_jumps.clear();
	points.clear();
	points.reserve(resolution());
	
	double step= double((-l_lim+r_lim)/resolution());
	
	Cn *vx = func.insertValueVariable(boundings().first(), 0.);
	
	bool jumping=true;
	for(double x=l_lim; x<r_lim-step; x+=step) {
		vx->setValue(x);
		Cn y = func.calculate().toReal();
		QPointF p(x, y.value());
		bool ch=addValue(p);
		
		bool jj=jumping;
		jumping=false;
		if(ch && !jj) {
// 			if(!m_jumps.isEmpty()) qDebug() << "popopo" << m_jumps.last() << points.count();
			double prevY=points[points.count()-2].y();
			if(y.format()!=Cn::Real && prevY!=y.value()) {
				m_jumps.append(points.count()-1);
				jumping=true;
			} else if(points.count()>3 && traverse(points[points.count()-3].y(), prevY, y.value())) {
				m_jumps.append(points.count()-1);
				jumping=true;
			}
		}
	}
// 	qDebug() << "juuuumps" << m_jumps << resolution();
}

void FunctionY::updatePoints(const QRect& viewport)
{
	double l_lim=viewport.left()-.1, r_lim=viewport.right()+.1;
	
	if(!points.isEmpty()
			&& isSimilar(points.first().x(), l_lim)
			&& isSimilar(points.last().x(), r_lim)) {
		return;
	}
	
	calculateValues(l_lim, r_lim);
// 	qDebug() << "end." << m_jumps;
}

QPair<QPointF, QString> FunctionX::calc(const QPointF& p)
{
	QPointF dp=p;
	func.insertValueVariable(boundings().first(), dp.y());
	if(!func.calculate().isReal())
		m_err += i18n("We can only draw Real results.");
	
	dp.setX(func.calculate().toReal().value());
	QString pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QPair<QPointF, QString> FunctionY::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	func.insertValueVariable(boundings().first(), dp.x());
	
	if(!func.calculate().isReal())
		m_err += i18n("We can only draw Real results.");
	
	dp.setY(func.calculate().toReal().value());
	pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QLineF FunctionY::derivative(const QPointF& p) const
{
	Analitza a(func.variables());
	double ret;
	
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.insertValueVariable(boundings().first(), p.x());
		
		if(a.isCorrect())
			ret = a.calculate().toReal().value();
		
		if(!a.isCorrect()) {
			kDebug() << "Derivative error: " <<  a.errors();
			return QLineF();
		}
	} else {
		QList<QPair<QString, double> > vars;
		vars.append(QPair<QString, double>(boundings().first(), p.x()));
		a.setExpression(func.expression());
		ret=a.derivative(vars);
	}
	
	return slopeToLine(ret);
}

void FunctionX::updatePoints(const QRect& viewport)
{
	double l_lim=viewport.bottom()-.1, r_lim=viewport.top()+.1;
	calculateValues(l_lim, r_lim);
	for(int i=0; i<points.size(); i++) {
		QPointF p=points[i];
		points[i]=QPointF(p.y(), p.x());
	}
}

QLineF FunctionX::derivative(const QPointF& p) const
{
	QPointF p1(p.y(), p.x());
	QLineF ret=FunctionY::derivative(p1);
	return mirrorXY(ret);
}
