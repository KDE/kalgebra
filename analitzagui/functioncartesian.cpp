/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
 *  Copyright (C) 2010 by Percy Camilo T. Aucahuasi <percy.camilo.ta@gmail.com>      *
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
#include "functionutils.h"

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
	explicit FunctionY(const Expression &e, Variables* v, const QString& bvar=QChar('x')) : FunctionImpl(e, v ,0,0), vx(new Cn)
	{
		m_runStack.append(vx);
		func.setStack(m_runStack);
		
		if(func.isCorrect()) {
			Expression deriv = func.derivative(bvar);
			if(func.isCorrect())
				m_deriv = new Expression(deriv);
			func.flushErrors();
		}
	}
	
	FunctionY(const FunctionY &fy) : FunctionImpl(fy), vx(new Cn)
	{
		m_runStack.append(vx);
		func.setStack(m_runStack);
	}
	virtual ~FunctionY() { delete vx; }
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p);
	virtual FunctionImpl* copy() { return new FunctionY(*this); }
	static QStringList supportedBVars() { return QStringList("x"); }
	static ExpressionType expectedType() { return ExpressionType(ExpressionType::Lambda).addParameter(ExpressionType(ExpressionType::Value)).addParameter(ExpressionType(ExpressionType::Value)); }
	virtual QString iconName() const { return "newfunction"; }
	
	QStringList boundings() const { return supportedBVars(); }
	void calculateValues(double, double);
	static QStringList examples() { return QStringList("x->x**sin x"); }
	void optimizeJump();
	
	Analitza::Cn* vx;
	QVector<Analitza::Object*> m_runStack;
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
			} else if(points.count()>3 && FunctionUtils::traverse(points[points.count()-3].y(), prevY, y.value())) {
				optimizeJump();
				m_jumps.append(points.count()-1);
				jumping=true;
			}
		}
	}
// 	qDebug() << "juuuumps" << m_jumps << resolution();
}

void FunctionY::optimizeJump()
{
	QPointF before = points.at(points.count()-2), after=points.last();
	qreal x1=before.x(), x2=after.x();
	qreal y1=before.y(), y2=after.y();
	int iterations=5;
	
// 	qDebug() << "+++++++++" << before << after;
	for(; iterations>0; --iterations) {
		qreal dist = x2-x1;
		qreal x=x1+dist/2;
		
		vx->setValue(x);
		qreal y = func.calculateLambda().toReal().value();
		
		if(fabs(y1-y)<fabs(y2-y)) {
			before.setX(x);
			before.setY(y);
			x1=x;
			y1=y;
		} else {
			after.setX(x);
			after.setY(y);
			x2=x;
			y2=y;
		}
	}
// 	qDebug() << "---------" << before << after;
	points[points.count()-2]=before;
	points.last()=after;
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
	vx->setValue(dp.y());
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
	Analitza::Analyzer a(func.variables());
	double ret;
	
	if(m_deriv) {
		vx->setValue(p.x());
		a.setExpression(*m_deriv);
		
		a.setStack(m_runStack);
		if(a.isCorrect())
			ret = a.calculateLambda().toReal().value();
		
		if(!a.isCorrect()) {
			kDebug() << "Derivative error: " <<  a.errors();
			return QLineF();
		}
	} else {
		QVector<Analitza::Object*> vars;
		vars.append(new Cn(p.x()));
		a.setExpression(func.expression());
		ret=a.derivative(vars);
		qDeleteAll(vars);
	}
	
	return FunctionUtils::slopeToLine(ret);
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
	return FunctionUtils::mirrorXY(ret);
}
