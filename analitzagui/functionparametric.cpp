/*************************************************************************************
 *  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
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

#include <KDebug>
#include <KLocale>

using Analitza::Expression;
using Analitza::ExpressionType;
using Analitza::Variables;
using Analitza::Object;
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

struct FunctionParametric : public FunctionImpl
{
	explicit FunctionParametric(const Expression &e, Variables* v) : FunctionImpl(e, v, 0,2*M_PI), vx(new Cn)
	{
		m_runStack.append(vx);
		func.setStack(m_runStack);
		
		if(func.isCorrect()) {
			Expression deriv = func.derivative("t");
			if(func.isCorrect())
				m_deriv = new Expression(deriv);
			func.flushErrors();
		}
	}
	
	FunctionParametric(const FunctionParametric &fx) : FunctionImpl(fx), vx(new Cn)
	{
		m_runStack.append(vx);
		func.setStack(m_runStack);
	}
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p);
	virtual FunctionImpl* copy() { return new FunctionParametric(*this); }
	virtual QString iconName() const { return "newparametric"; }
	
	static QStringList supportedBVars() { return QStringList("t"); }
	static ExpressionType expectedType() { return ExpressionType(ExpressionType::Lambda).addParameter(ExpressionType(ExpressionType::Value)).addParameter(ExpressionType(ExpressionType::Vector, ExpressionType(ExpressionType::Value), 2)); }
	QStringList boundings() const { return supportedBVars(); }
	static QStringList examples() { return QStringList("t->vector {t,t**2}"); }
	Cn* vx;
	
	QVector<Analitza::Object*> m_runStack;
};
REGISTER_FUNCTION(FunctionParametric)

void FunctionParametric::updatePoints(const QRect& viewport)
{
	Q_UNUSED(viewport);
	Q_ASSERT(func.expression().isCorrect());
	//if(int(resolution())==points.capacity())
	//	return;
	
	double ulimit=uplimit();
	double dlimit=downlimit();
	
	points.clear();
	m_jumps.clear();
	//points.reserve(resolution());

	
// 	double inv_res= double((ulimit-dlimit)/resolution());
	double inv_res= 0.01; 
// 	double final=ulimit-inv_res;
	
		//by percy
	QRectF vp(viewport);
	
	vp.setTop(viewport.top() - 2);
	vp.setBottom(viewport.bottom() + 2);
	vp.setLeft(viewport.left() + 2);
	vp.setRight(viewport.right() - 2);
	
	QPointF curp;
	
	vx->setValue(dlimit);
	Expression res;
	
	int i = 0;
	bool jlock = false;
	
	for(double t=dlimit; t<ulimit; t+=inv_res, ++i) {
		vx->setValue(t);
		res=func.calculateLambda();
		
		Cn x=res.elementAt(0).toReal();
		Cn y=res.elementAt(1).toReal();
		
		curp = QPointF(x.value(), y.value());
		
		if (vp.contains(curp))
		{
			addValue(curp);
			jlock = false;
		}
		else if (!jlock)
		{
			m_jumps.append(i);
			jlock = true;
		}
		
		// 		objectWalker(vo);
		Q_ASSERT(res.isVector());
	}
}

QPair<QPointF, QString> FunctionParametric::calc(const QPointF& point)
{
	vx->setValue(0.);
	Expression res=func.calculateLambda();

    Q_ASSERT(func.isCorrect() && func.expression().lambdaBody().isVector());
    {
        Analitza::Analyzer f(func.variables());
        f.setExpression(Analitza::Expression("t->" + func.expression().lambdaBody().elementAt(0).toString() + "+" + QString::number(-1.0*point.x()), false));
        f.setStack(m_runStack);

        Analitza::Analyzer df(func.variables());
        df.setExpression(f.derivative("t"));
        df.setStack(m_runStack);

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
            vx->setValue(t0);

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
//        vx->setValue(t0);
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

        vx->setValue(t);

        Analitza::Cn comp1 = res.elementAt(0).toReal();
        Analitza::Cn comp2 = res.elementAt(1).toReal();

        return QPair<QPointF, QString>(QPointF(comp1.value(), comp2.value()), QString());
    }
}

QLineF FunctionParametric::derivative(const QPointF& point)
{
    Q_ASSERT(func.isCorrect() && func.expression().lambdaBody().isVector());
    {
        Analitza::Analyzer f(func.variables());
        f.setExpression(Analitza::Expression("t->" + func.expression().lambdaBody().elementAt(0).toString() + "+" + QString::number(-1.0*point.x()), false));
        f.setStack(m_runStack);

        Analitza::Analyzer df(func.variables());
        df.setExpression(f.derivative("t"));
        df.setStack(m_runStack);

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
            vx->setValue(t0);

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
//        vx->setValue(t0);
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

        Analitza::Analyzer dfunc(func.variables());
        dfunc.setExpression(func.derivative("t"));
        dfunc.setStack(m_runStack);

        vx->setValue(t);

		Expression exp=dfunc.calculateLambda();
        Analitza::Cn comp1 = exp.elementAt(0).toReal();
        Analitza::Cn comp2 = exp.elementAt(1).toReal();

        double m = comp2.value()/comp1.value();

        return slopeToLine(m);
    }
}
