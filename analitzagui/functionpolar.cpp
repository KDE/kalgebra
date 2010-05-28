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
#include <analitza/variable.h>
#include "analitza/vector.h"
#include <analitza/expressiontype.h>

#include <KLocale>

using std::acos;
using std::atan;
using std::sqrt;

using Analitza::Expression;
using Analitza::ExpressionType;
using Analitza::Variables;
using Analitza::Cn;

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
        const double len=6.*der;
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

struct FunctionPolar : public FunctionImpl
{
	FunctionPolar(const Expression &e, Variables* v);
	FunctionPolar(const FunctionPolar &fp);
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	function::Axe axeType() const { return function::Polar; }
    QLineF derivative(const QPointF& p);
	virtual FunctionImpl* copy() { return new FunctionPolar(*this); }
    static QStringList examples() { return QStringList("q->3*sin(7*q)"); } //afiestas's favourite plot
	
	inline QPointF fromPolar(double r, double th) { return QPointF(r*std::cos(th), r*std::sin(th)); }
	QRect m_last_viewport;
	QStringList boundings() const { return supportedBVars(); }
	static QStringList supportedBVars() { return QStringList("q"); }
	static ExpressionType expectedType() { return ExpressionType(ExpressionType::Lambda).addParameter(ExpressionType(ExpressionType::Value)).addParameter(ExpressionType(ExpressionType::Value)); }
	
	Cn* m_th;
};

REGISTER_FUNCTION(FunctionPolar)
static const double pi=acos(-1.);

FunctionPolar::FunctionPolar(const Expression &e, Variables* v)
	: FunctionImpl(e, v, 0, 2*M_PI)
	, m_th(new Cn)
{
		Analitza::Ci* vi=func.refExpression()->parameters().first();
		vi->value()=m_th;
}

FunctionPolar::FunctionPolar(const FunctionPolar &fp)
	: FunctionImpl(fp)
	, m_th(new Cn)
{
		Analitza::Ci* vi=func.refExpression()->parameters().first();
		vi->value()=m_th;
}

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
	
	double inv_res= double((ulimit-dlimit)/resolution());
	double final=ulimit-inv_res;
	for(double th=dlimit; th<final; th+=inv_res) {
		m_th->setValue(th);
		double r = func.calculateLambda().toReal().value();
		
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
	
	QPointF dist;
	m_th->setValue(th);
	do {
		m_th->setValue(th);
		r = func.calculateLambda().toReal().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		m_th->setValue(th+2.*pi);
		r = func.calculateLambda().toReal().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d2 = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		th += 2.*pi;
	} while(d>d2);
	th -= 2.*pi;
	
	m_th->setValue(th);
	Expression res=func.calculateLambda();
	
	if(!res.isReal())
		m_err += i18n("We can only draw Real results.");
	r = res.toReal().value();
	dp = fromPolar(r, th);
	
	pos = QString("r=%1 th=%2").arg(r,3,'f',2).arg(th,3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QLineF FunctionPolar::derivative(const QPointF& point)
{
    QString rt = func.expression().lambdaBody().toString();
    rt.replace("q", "t");

    QString comp1str = rt + "*cos(t)";
    QString comp2str = rt + "*sin(t)";

    QString polart;
    polart = "t->vector{" + comp1str + ", " + comp2str + "}";

    Analitza::Analyzer newfunc(func.variables());
    newfunc.setExpression(Analitza::Expression(polart, false));
    newfunc.refExpression()->parameters()[0]->value() = m_th;

    if(newfunc.isCorrect() && newfunc.expression().lambdaBody().isVector())
    {
        Analitza::Analyzer f(newfunc.variables());
        f.setExpression(Analitza::Expression("t->" + newfunc.expression().lambdaBody().elementAt(0).toString() + "+" + QString::number(-1.0*point.x()), false));
        f.refExpression()->parameters()[0]->value() = m_th;

        Analitza::Analyzer df(newfunc.variables());
        df.setExpression(f.derivative("t"));
        df.refExpression()->parameters()[0]->value() = m_th;

        qDebug() << f.expression().toString();

//TODO
//    Analitza::Analyzer g(func.variables());
//    g.setExpression(Analitza::Expression("t->" + func.expression().lambdaBody().elementAt(1).toString() + "+" + QString::number(-1.0*point.y()), false));
//    g.refExpression()->parameters()[0]->value() = vx;
//
//    Analitza::Analyzer dg(func.variables());
//    dg.setExpression(g.derivative("t"));
//    dg.refExpression()->parameters()[0]->value() = vx;

        const int MAX_I = 256;
        const double E = 0.0001;
        double t0 = 1.0;
        double t = t0;
        double error = 1000.0;
        int i = 0;

        while (true)
        {
            m_th->setValue(t0);

            double r = f.calculateLambda().toReal().value();
            double d = df.calculateLambda().toReal().value();

            i++;
            t = t0 - r/d;

            if ((error < E) || (i > MAX_I))
                break;

            error = fabs(t - t0);
            t0 = t;
        }

//TODO
//    t0 = 1.0;
//    t = t0;
//    error = 1000.0;
//    i = 0;
//
//    while (true)
//    {
//        m_th->setValue(t0);
//
//        double r = g.calculateLambda().toReal().value();
//        double d = dg.calculateLambda().toReal().value();
//
//        i++;
//        t = t0 - r/d;
//
//        if ((error < E) || (i > MAX_I))
//            break;
//
//        error = fabs(t - t0);
//        t0 = t;
//    }

        Analitza::Analyzer dfunc(newfunc.variables());
        dfunc.setExpression(newfunc.derivative("t"));
        dfunc.refExpression()->parameters()[0]->value() = m_th;

        m_th->setValue(t);

        Analitza::Vector* v = static_cast<Analitza::Vector*>(dfunc.calculateLambda().tree());
        Analitza::Cn *comp1 = static_cast<Cn*>(v->at(0));
        Analitza::Cn *comp2 = static_cast<Cn*>(v->at(1));

        double m = comp2->value()/comp1->value();

        return slopeToLine(m);
    }
    else
        return QLineF();
}
