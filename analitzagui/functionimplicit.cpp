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

#include <QVector2D>
#include <qmath.h>
#include <KDebug>
#include <KLocale>

#include "analitza/value.h"
#include "analitza/vector.h"
#include "analitza/container.h"
#include "analitza/expressiontype.h"
#include "analitza/variable.h"
#include "analitza/variables.h"
#include "analitza/value.h"
#include "functionimpl.h"
#include "functionfactory.h"

using Analitza::Expression;
using Analitza::ExpressionType;
using Analitza::Variables;
using Analitza::Vector;
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
        const double len=7.*der;
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

//BEGIN Box class

class Box : public QRectF
{
    public:
        explicit Box();
        Box(const Box &box);
        Box(const QRect &rect);
        Box(const QRectF &rect);
        Box(qreal x, qreal y, qreal width, qreal height);

        // no usar we sw usar la convencion de la plataforma topleftbox ...
        Box nwBox() const;
        Box neBox() const;
        Box seBox() const;
        Box swBox() const;

        qreal topLeftValue;
        qreal topRightValue;
        qreal bottomRightValue;
        qreal bottomLeftValue;

        void printValues();
};

Box::Box()
    : QRectF()
{

}

Box::Box(const QRect &rect)
    :  QRectF(rect)
{

}

Box::Box(const QRectF &rect)
    : QRectF(rect)
{

}

Box::Box(const Box &box)
    : QRectF(box)
{
}

Box::Box(qreal x, qreal y, qreal width, qreal height)
    : QRectF(x, y, width, height)
{

}

Box Box::nwBox() const
{
    return Box(x(), y(), width()*0.5, height()*0.5);
}

Box Box::neBox() const
{
    return Box(x() + width()*0.5, y(), width()*0.5, height()*0.5);
}

Box Box::seBox() const
{
    return Box(x() + width()*0.5, y() + height()*0.5, width()*0.5, height()*0.5);
}

Box Box::swBox() const
{
    return Box(x(), y() + height()*0.5, width()*0.5, height()*0.5);
}

void Box::printValues()
{
    qDebug() << "TL -> " << topLeftValue;
    qDebug() << "TR -> " << topRightValue;
    qDebug() << "BR -> " << bottomRightValue;
    qDebug() << "BL -> " << bottomLeftValue;
}

//END Box class

//NOTE This version of FunctionImplicit implements the quadtree approach
struct FunctionImplicit : public FunctionImpl
{
    explicit FunctionImplicit(const Expression &e, Variables* v)
        : FunctionImpl(e, v, 0,2*M_PI)
        , dx(new Analitza::Variables)
        , dy(new Analitza::Variables)
    {
        initImplicitFunction();
    }

    FunctionImplicit(const FunctionImplicit &fx) : FunctionImpl(fx)
        , dx(new Analitza::Variables)
        , dy(new Analitza::Variables)
    {
        initImplicitFunction();
    }

    void updatePoints(const QRect& viewport);
    QPair<QPointF, QString> calc(const QPointF& dp);
    QLineF derivative(const QPointF& p);
    virtual FunctionImpl* copy() { return new FunctionImplicit(*this); }

    static QStringList supportedBVars()
    {
        QStringList st;
        st.append("x");
        st.append("y");
        return st;
    }

    static ExpressionType expectedType()
    {
        return ExpressionType(ExpressionType::Lambda).addParameter(ExpressionType(ExpressionType::Value)).addParameter(ExpressionType(ExpressionType::Value)).addParameter(ExpressionType(ExpressionType::Value));
    }

    QStringList boundings() const { return supportedBVars(); }

    void initImplicitFunction();

    qreal evalImplicitFunction(qreal x, qreal y);
    qreal evalPartialDerivativeX(qreal x, qreal y);
    qreal evalPartialDerivativeY(qreal x, qreal y);

    Analitza::Analyzer dx; // partial derivative respect to var x
    Analitza::Analyzer dy; // partial derivative respect to var y

    Analitza::Cn *vx; // var x
    Analitza::Cn *vy; // var y

    QPointF initialPoint;
    static QStringList examples() { return QStringList("x^3-y^2+2") << "y^2*(y^2-10)-x^2*(x^2-9)"; }
    
    QPointF last_calc;
	QVector<Analitza::Object*> m_runStack;

    // small size means better quality but decrease the performance of updating
    static const qreal MIN_SIZE_OF_BOX;
    static const qreal MIN_SIZE_OF_BOX_FOR_CHECK_SIGNS;

    public:
        bool isBoxEmpty(const Box &box);
        void subdivideSpace(const Box &root);
        bool allDisconnected() { return true; } // true when we work implicit curves

    private:
        void addPath(const QPointF &a, const QPointF &b)
        {
//            addValue(a);
//            addValue(b);
            points.append(a);
            points.append(b);
//            static int i = 0;
//            m_jumps.append(2*i);
//            i++;
        }
};

//const qreal FunctionImplicit::MIN_SIZE_OF_BOX = 0.005; 0.005 good value for high quality
const qreal FunctionImplicit::MIN_SIZE_OF_BOX = 0.025;
//const qreal FunctionImplicit::MIN_SIZE_OF_BOX_FOR_CHECK_SIGNS = 1.5; 1.0 is ok for high quality
const qreal FunctionImplicit::MIN_SIZE_OF_BOX_FOR_CHECK_SIGNS = 1.15;

REGISTER_FUNCTION(FunctionImplicit)

void FunctionImplicit::initImplicitFunction()
{
    vx = new Analitza::Cn;
    vy = new Analitza::Cn;
	m_runStack.append(vx);
	m_runStack.append(vy);

    func.setStack(m_runStack);

    dx.setExpression(func.derivative("x"));
    dx.setStack(m_runStack);

    dy.setExpression(func.derivative("y"));
    dy.setStack(m_runStack);
}

qreal FunctionImplicit::evalImplicitFunction(qreal x, qreal y)
{
    vx->setValue(x);
    vy->setValue(y);

    return func.calculateLambda().toReal().value();
}

qreal FunctionImplicit::evalPartialDerivativeX(qreal x, qreal y)
{
    vx->setValue(x);
    vy->setValue(y);

    return dx.calculateLambda().toReal().value();
}

qreal FunctionImplicit::evalPartialDerivativeY(qreal x, qreal y)
{
    vx->setValue(x);
    vy->setValue(y);

    return dy.calculateLambda().toReal().value();
}

//Begin Quadtree

bool FunctionImplicit::isBoxEmpty(const Box &cell)
{
    Box box(cell);

    vx->setValue(box.topLeft().x());
    vy->setValue(box.topLeft().y());
    box.topLeftValue = func.calculateLambda().toReal().value();

    vx->setValue(box.topRight().x());
    vy->setValue(box.topRight().y());
    box.topRightValue = func.calculateLambda().toReal().value();

    vx->setValue(box.bottomRight().x());
    vy->setValue(box.bottomRight().y());
    box.bottomRightValue = func.calculateLambda().toReal().value();

    vx->setValue(box.bottomLeft().x());
    vy->setValue(box.bottomLeft().y());
    box.bottomLeftValue = func.calculateLambda().toReal().value();

    qreal topSignShift = box.topLeftValue*box.topRightValue;
    qreal rightSignShift = box.topRightValue*box.bottomRightValue;
    qreal bottomSignShift = box.bottomRightValue*box.bottomLeftValue;
    qreal leftSignShift = box.bottomLeftValue*box.topLeftValue;

    // if there is sign shift on any vertice then a segment
    // of the curve is in the box
    if ((topSignShift < 0) || (rightSignShift < 0) || (bottomSignShift < 0) || (leftSignShift < 0))
    {
        if (box.width() < MIN_SIZE_OF_BOX)
        {
            // | (t->b)
            if ((topSignShift < 0) && (rightSignShift > 0) && (bottomSignShift < 0) && (leftSignShift > 0))
                addPath(QPointF(box.center().x(), box.top()),
                    QPointF(box.center().x(), box.bottom()));

            // - (r->l)
            if ((topSignShift > 0) && (rightSignShift < 0) && (bottomSignShift > 0) && (leftSignShift < 0))
                addPath(QPointF(box.left(), box.center().y()),
                    QPointF(box.right(), box.center().y()));

            // \ (t->r)
            if ((topSignShift < 0) && (rightSignShift < 0) && (bottomSignShift > 0) && (leftSignShift > 0))
                addPath(QPointF(box.center().x(), box.top()),
                    QPointF(box.right(), box.center().y()));
            // / (r->b)
            if ((topSignShift > 0) && (rightSignShift < 0) && (bottomSignShift < 0) && (leftSignShift > 0))
                addPath(QPointF(box.right(), box.center().y()),
                    QPointF(box.center().x(), box.bottom()));

            // \ (b->r)
            if ((topSignShift > 0) && (rightSignShift < 0) && (bottomSignShift < 0) && (leftSignShift > 0))
                addPath(QPointF(box.center().x(), box.bottom()),
                    QPointF(box.right(), box.center().y()));

            // / (r->t)
            if ((topSignShift < 0) && (rightSignShift < 0) && (bottomSignShift > 0) && (leftSignShift > 0))
                addPath(QPointF(box.right(), box.center().y()),
                    QPointF(box.center().x(), box.top()));

            // - t
            if ((topSignShift < 0) && (rightSignShift > 0) && (bottomSignShift > 0) && (leftSignShift > 0))
                addPath(box.topLeft(), box.topRight());

            // - b
            if ((topSignShift > 0) && (rightSignShift > 0) && (bottomSignShift < 0) && (leftSignShift > 0))
                addPath(box.bottomLeft(), box.bottomRight());

            // | r
            if ((topSignShift > 0) && (rightSignShift > 0) && (bottomSignShift < 0) && (leftSignShift > 0))
                addPath(box.topRight(), box.bottomRight());

            // | l
            if ((topSignShift > 0) && (rightSignShift > 0) && (bottomSignShift > 0) && (leftSignShift < 0))
                addPath(box.topLeft(), box.bottomLeft());
        }

        return false;
    }

    return true;
}

//TODO must return a quadtree
void FunctionImplicit::subdivideSpace(const Box &root)
{
    QVector<Box> boxes;
    boxes.append(root.nwBox());
    boxes.append(root.neBox());
    boxes.append(root.seBox());
    boxes.append(root.swBox());

    for (int i = 0; i < boxes.size(); i++)
    {
        bool empty_box = isBoxEmpty(boxes.at(i));

        if ((empty_box) && (boxes.at(i).width() > MIN_SIZE_OF_BOX_FOR_CHECK_SIGNS))
          empty_box = false;

        if (empty_box)
        {
        }
        else
        {
            if (boxes.at(i).width() < MIN_SIZE_OF_BOX)
            {
//                points.append(boxes.at(i).center());
            }
            else
                subdivideSpace(boxes.at(i));
        }
    }
}

//End Quadtree

void FunctionImplicit::updatePoints(const QRect& viewport)
{
//    Q_UNUSED(viewport);
    Q_ASSERT(func.expression().isCorrect());

    if(int(resolution())==points.capacity())
        return;

    //double ulimit = uplimit();
    //double dlimit = downlimit();

	points.clear();
	points.reserve(resolution());

//    tree<Box>::iterator top;
//    top = quadtree.begin();
//    currentroot = quadtree.insert(top, Box(viewport.x(), viewport.y(), viewport.width(), viewport.height()));

//    points.clear();
	Box root(viewport); //TODO translate points before?

	if (viewport.width() < 23)
		root = Box(-15,-18,34,33);
//		root = Box(-15,-18,34,33); // default fixed viewport settings

    subdivideSpace(root);

    if (points.size() <= 2)
    {
        m_err << i18nc("This function can't be represented as a curve. To draw implicit curve, the function has to satisfy the implicit function theorem.", "Implicit function undefined in the plane");
    }
}

QPair<QPointF, QString> FunctionImplicit::calc(const QPointF& point)
{
    // Check for x
    QVector<Analitza::Object*> vxStack; vxStack.append(vx);
    QVector<Analitza::Object*> vyStack; vyStack.append(vy);

    QString expLiteral = func.expression().lambdaBody().toString();
    expLiteral.replace("y", QString::number(point.y()));
    expLiteral.prepend("x->");

    Analitza::Analyzer f(func.variables());
    f.setExpression(Analitza::Expression(expLiteral, false));
    f.setStack(vxStack);

    Analitza::Analyzer df(func.variables());
    df.setExpression(f.derivative("x"));
    df.setStack(vxStack);

    const int MAX_I = 256;
    const double E = 0.0001;
    double x0 = point.x();
    double x = x0;
    double error = 1000.0;
    int i = 0;
    bool has_root_x = true;

    while (true)
    {
        vx->setValue(x0);

        double r = f.calculateLambda().toReal().value();
        double d = df.calculateLambda().toReal().value();

        i++;
        x = x0 - r/d;

        if (error < E) break;
        if (i > MAX_I)
        {
            has_root_x = false;
            break;
        }

        error = fabs(x - x0);
        x0 = x;
    }

    // Check for y
    if (!has_root_x)
    {
        expLiteral = func.expression().lambdaBody().toString();
        expLiteral.replace("x", QString::number(point.x()));
        expLiteral.prepend("y->");

        Analitza::Analyzer f(func.variables());
        f.setExpression(Analitza::Expression(expLiteral, false));
        f.setStack(vyStack);

        Analitza::Analyzer df(func.variables());
        df.setExpression(f.derivative("y"));
        df.setStack(vyStack);

        double y0 = point.y();
        double y = y0;
        error = 1000.0;
        i = 0;
        bool has_root_y = true;

        while (true)
        {
            vy->setValue(y0);

            double r = f.calculateLambda().toReal().value();
            double d = df.calculateLambda().toReal().value();

            i++;
            y = y0 - r/d;

            if (error < E) break;
            if (i > MAX_I)
            {
                has_root_y = false;
                break;
            }

            error = fabs(y - y0);
            y0 = y;
        }

		if (has_root_y)
			last_calc = QPointF(point.x(), y);
		return QPair<QPointF, QString>(last_calc, QString());
    }
    else
    {
        last_calc = QPointF(x, point.y());

        return QPair<QPointF, QString>(last_calc, QString());
    }
}

QLineF FunctionImplicit::derivative(const QPointF& p)
{
    double fx = evalPartialDerivativeX(p.x(), p.y());
    double fy = evalPartialDerivativeY(p.x(), p.y());

    return slopeToLine(-fx/fy);
}
