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
#include "analitza/value.h"
#include <analitza/expressiontype.h>
#include <analitza/variable.h>
#include <analitza/analitzautils.h>

#include <KDebug>
#include <KLocale>

using std::sin;
using std::cos;
using std::atan;
using std::fabs;

using Analitza::ExpressionType;
using Analitza::Expression;
using Analitza::Variables;
using Analitza::Cn;

///Functions where the x is bounding. like x->sin(x)
struct FunctionY : public FunctionImpl
{
	explicit FunctionY(const Expression &e, Variables* v, const QString& bvar="x") : FunctionImpl(e, v ,0,0), vx(new Cn)
	{
		Analitza::Ci* vi=func.refExpression()->parameters().first();
		vi->value()=vx;
		
		if(func.isCorrect()) {
			Expression deriv = func.derivative(bvar);
			if(func.isCorrect())
				m_deriv = new Expression(deriv);
			func.flushErrors();
		}
	}
	
	FunctionY(const FunctionY &fy) : FunctionImpl(fy), vx(new Cn)
	{
		Analitza::Ci* vi=func.refExpression()->parameters().first();
		vi->value()=vx;
	}
	virtual ~FunctionY() { delete vx; }
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
    QLineF derivative(const QPointF& p);
	virtual FunctionImpl* copy() { return new FunctionY(*this); }
	static QStringList supportedBVars() { return QStringList("x"); }
    static ExpressionType expectedType() { return ExpressionType(ExpressionType::Lambda).addParameter(ExpressionType(ExpressionType::Value)).addParameter(ExpressionType(ExpressionType::Value)); }
	
	QStringList boundings() const { return supportedBVars(); }
	void calculateValues(double, double);
	static QStringList examples() { return QStringList("x->x**sin x"); }
	
	Analitza::Cn* vx;
};

///Functions where the y is bounding. like y->sin(y). FunctionY mirrored
struct FunctionX : public FunctionY
{
	explicit FunctionX(const Expression &e, Variables* v) : FunctionY(e, v, "y") {}
	FunctionX(const FunctionX &fx) : FunctionY(fx) {}
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
    QLineF derivative(const QPointF& p);
	virtual FunctionImpl* copy() { return new FunctionX(*this); }
	
	static QStringList supportedBVars() { return QStringList("y"); }
	static ExpressionType expectedType() { return FunctionY::expectedType(); }
	QStringList boundings() const { return supportedBVars(); }
	static QStringList examples() { return QStringList("y->sin y"); }
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
	
	bool jumping=true;
	for(double x=l_lim; x<r_lim-step; x+=step) {
		vx->setValue(x);
		Cn y = func.calculateLambda().toReal();
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
	vx->setValue(dp.x());
	Expression r=func.calculateLambda();
	
	if(!r.isReal())
		m_err += i18n("We can only draw Real results.");
	
	dp.setX(r.toReal().value());
	QString pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QPair<QPointF, QString> FunctionY::calc(const QPointF& p)
{
	QPointF dp=p;
	vx->setValue(dp.x());
	Expression r=func.calculateLambda();
	
	if(!r.isReal())
		m_err += i18n("We can only draw Real results.");
	
	dp.setY(r.toReal().value());
	QString pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QLineF FunctionY::derivative(const QPointF& p)
{
    Analitza::Analyzer df(func.variables());
    df.setExpression(func.derivative("x"));
    df.expression().parameters().first()->value() = vx;

    if(!df.isCorrect()) {
        kDebug() << "Derivative error: " <<  df.errors();
        return QLineF();
    }

    vx->setValue(p.x());

    return slopeToLine(df.calculateLambda().toReal().value());
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

QLineF FunctionX::derivative(const QPointF& p)
{
	QPointF p1(p.y(), p.x());
	QLineF ret=FunctionY::derivative(p1);
	return mirrorXY(ret);
}
