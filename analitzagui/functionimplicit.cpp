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

//BEGIN Vertex

// Vertex for he simplex
class Vertex : public QPointF
{
    public:
        explicit Vertex()
            : QPointF()
        {}

        Vertex(const QPointF point, qreal value = 0.)
            : QPointF(point)
        {
            setValue(value);
        }

        Vertex(qreal x, qreal y, qreal value = 0.)
            : QPointF(x, y)
        {
            setValue(value);
        }

        qreal value() const { return m_value; }
        bool isUpperSide() const { return m_isUpperSide; }
        unsigned short int order() const { return m_order; }

        void setValue(qreal value)
        {
            m_value = value;

            if (value > 0.)
                m_isUpperSide = true;
            else
                // the value never will be zero (in theory)
                m_isUpperSide = false;
        }

        void setOrder(unsigned short int order) { m_order = order; }

    private:
        //NOTE We get this value when eval the implicit function
        qreal m_value;
        bool m_isUpperSide; // if the vertex is upper the curve
        unsigned short int m_order; // order of the vertex: 1 (new) and 3 (older)
};

//END Vertex

//BEGIN Simplex

// A simplex in 2D space

const double P_CONST_COEF = 0.9659258262890682; // (N + sqrt(N + 1.) - 1.)/(sqrt(2.)*N);
const double Q_CONST_COEF = 0.2588190451025207; // (sqrt(N + 1.) - 1.)/(sqrt(2.)*N);

class Simplex
{
    public:
        // Min means that simplex is tracking a minimum
        // Undef means that the simplex is over the curve
        enum Target {Undef = 0, Min, Max};

        Simplex();
        // The constructor define the order of the vertexes
        Simplex(const Vertex & vertex1, const Vertex & vertex2, const Vertex & vertex3, Target target = Undef);

        Vertex vertex1() const; // new vertex
        Vertex vertex2() const;
        Vertex vertex3() const; // old vertex

        // Wich vertex contains the greater value
        // zeta > beta > alfa
        Vertex vertexAlfa() const;
        Vertex vertexBeta() const;
        Vertex vertexZeta() const;

        qreal edge() const;
        QPointF centerOfGravity() const;

        Target target() const;

    private:
        // This vector contains six vertexes: vertex1, vertex2,
        // vertex3, vertexAlfa, vertexBeta and vertexZeta
        QVector<Vertex> m_vertexList;
        qreal m_edge; // edge of the vertex

        Target m_target; // default Undef
};

const qreal SQRT_3 = sqrt(3);

Simplex::Simplex()
    : m_edge(1.0)
    , m_target(Undef)
{
}

Simplex::Simplex(const Vertex & vertex1, const Vertex & vertex2, const Vertex & vertex3, Target target)
    : m_target(target)
{
    Vertex vector = vertex2 - vertex1;
    qreal norm = sqrt(vector.x()*vector.x() + vector.y()*vector.y());
    m_edge = norm;

    m_vertexList.append(vertex1);
    m_vertexList.append(vertex2);
    m_vertexList.append(vertex3);

    // update the order ... see the constructor
    m_vertexList[0].setOrder(1);
    m_vertexList[1].setOrder(2);
    m_vertexList[2].setOrder(3);

    m_vertexList.append(vertex1);
    m_vertexList.append(vertex2);
    m_vertexList.append(vertex3);

    m_vertexList[3].setOrder(1);
    m_vertexList[4].setOrder(2);
    m_vertexList[5].setOrder(3);

    for (short int i = 0; i < 3; ++i)
        for (short int j = 0; j < 3; ++j)
            if (m_vertexList[i + 3].value() < m_vertexList[j + 3].value())
            {
                Vertex tmp;
                tmp.setX(m_vertexList[i + 3].x());
                tmp.setY(m_vertexList[i + 3].y());
                tmp.setValue(m_vertexList[i + 3].value());
                tmp.setOrder(m_vertexList[i + 3].order());

                m_vertexList[i + 3].setX(m_vertexList[j + 3].x());
                m_vertexList[i + 3].setY(m_vertexList[j + 3].y());
                m_vertexList[i + 3].setValue(m_vertexList[j + 3].value());
                m_vertexList[i + 3].setOrder(m_vertexList[j + 3].order());

                m_vertexList[j + 3].setX(tmp.x());
                m_vertexList[j + 3].setY(tmp.y());
                m_vertexList[j + 3].setValue(tmp.value());
                m_vertexList[j + 3].setOrder(tmp.order());
            }
}

Vertex Simplex::vertex1() const { return m_vertexList[0]; }
Vertex Simplex::vertex2() const { return m_vertexList[1]; }
Vertex Simplex::vertex3() const { return m_vertexList[2]; }

Vertex Simplex::vertexAlfa() const { return m_vertexList[3]; }
Vertex Simplex::vertexBeta() const { return m_vertexList[4]; }
Vertex Simplex::vertexZeta() const { return m_vertexList[5]; }

qreal Simplex::edge() const { return m_edge; }

QPointF Simplex::centerOfGravity() const
{
    return (vertex1() + vertex2() + vertex3())/3.;
}

Simplex::Target Simplex::target() const
{
    return m_target;
}

//END Simplex

//NOTE This version of FunctionImplicit implements a
// an algorithm of the type tangential, it draw follows the
// tangent
struct FunctionImplicit : public FunctionImpl
{
    explicit FunctionImplicit(const Expression &e, Variables* v)
        : FunctionImpl(e, v, 0,2*M_PI)
        , dx(new Analitza::Variables)
        , dy(new Analitza::Variables)
    {
        Analitza::Ci* vi=func.refExpression()->parameters().first();
        vi->value()=vx;

        initImplicitFunction();
    }

    FunctionImplicit(const FunctionImplicit &fx) : FunctionImpl(fx)
        , dx(new Analitza::Variables)
        , dy(new Analitza::Variables)
    {
        Analitza::Ci* vi=func.refExpression()->parameters().first();
        vi->value()=vx;

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

    QPointF findBestPointOnCurve(const QPointF &refPoint = QPointF(22.0, -52.0));

    Analitza::Cn *vx; // var x
    Analitza::Cn *vy; // var y

    Analitza::Analyzer dx; // partial derivative respect to var x
    Analitza::Analyzer dy; // partial derivative respect to var x

    // Used for search a close point on curve
    QVector<Simplex> simplexes;

    QPointF initialPoint;
    static QStringList examples() { return QStringList("(2*x+y)*(x^2+y^2)^4+2*y*(5*x^4+10*x^2*y^2-3*y^4)-2*x+y"); }
    QPointF last_calc;
};

REGISTER_FUNCTION(FunctionImplicit)

void FunctionImplicit::initImplicitFunction()
{
    vx = new Analitza::Cn;
    vy = new Analitza::Cn;

    func.refExpression()->parameters()[0]->value() = vx;
    func.refExpression()->parameters()[1]->value() = vy;

    dx.setExpression(func.derivative("x"));
    dx.refExpression()->parameters()[0]->value() = vx;
    dx.refExpression()->parameters()[1]->value() = vy;

    dy.setExpression(func.derivative("y"));
    dy.refExpression()->parameters()[0]->value() = vx;
    dy.refExpression()->parameters()[1]->value() = vy;

    initialPoint = findBestPointOnCurve();
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

QPointF FunctionImplicit::findBestPointOnCurve(const QPointF &refPoint)
{
    simplexes.clear();

    double m_simplexEdge = 0.5;

    // clac the first simplex, next simplexes will be generated by reflection
    double relativePosX = refPoint.x();
    double relativePosY = refPoint.y();

    double p = m_simplexEdge*P_CONST_COEF;
    double q = m_simplexEdge*Q_CONST_COEF;

    Vertex vertex1(relativePosX, relativePosY,
                   evalImplicitFunction(relativePosX, relativePosY));

    Vertex vertex2(relativePosX + p, relativePosY + q,
                   evalImplicitFunction(relativePosX + p, relativePosY + q));

    Vertex vertex3(relativePosX + q, relativePosY + p,
                   evalImplicitFunction(relativePosX + q, relativePosY + p));

    simplexes.append(Simplex(vertex1, vertex2, vertex3));

    int max_iter = 0;

    while (max_iter < 2048)
    {
        max_iter++;

        bool isUpperSide = ((simplexes.last().vertex1().isUpperSide() == true) &&
                            (simplexes.last().vertex2().isUpperSide() == true) &&
                            (simplexes.last().vertex3().isUpperSide() == true));

        bool isUnderSide = ((simplexes.last().vertex1().isUpperSide() == false) &&
                            (simplexes.last().vertex2().isUpperSide() == false) &&
                            (simplexes.last().vertex3().isUpperSide() == false));

        if (isUpperSide || isUnderSide)
        {
            bool findMax;

            if (isUpperSide) // then we have to search a minimum
                findMax = false;

            if (isUnderSide) // then we have to search a maximun
                findMax = true;

            // This vertex will be generated by reflection
            Vertex newVertex;

            if (findMax)
                newVertex = (simplexes.last().vertexBeta() +
                             simplexes.last().vertexZeta()) -
                             simplexes.last().vertexAlfa();
            else
                newVertex = (simplexes.last().vertexAlfa() +
                             simplexes.last().vertexBeta()) -
                             simplexes.last().vertexZeta();

            //update the value of the vertex
            newVertex.setValue(evalImplicitFunction(newVertex.x(), newVertex.y()));

            Vertex vertex1 = newVertex;
            Vertex vertex2;
            Vertex vertex3;

            if (findMax)
            {
                vertex2 = simplexes.last().vertexZeta().order() > simplexes.last().vertexBeta().order()? simplexes.last().vertexBeta():simplexes.last().vertexZeta();
                vertex3 = simplexes.last().vertexZeta().order() > simplexes.last().vertexBeta().order()? simplexes.last().vertexZeta():simplexes.last().vertexBeta();
            }
            else
            {
                vertex2 = simplexes.last().vertexAlfa().order() > simplexes.last().vertexBeta().order()? simplexes.last().vertexBeta():simplexes.last().vertexAlfa();
                vertex3 = simplexes.last().vertexAlfa().order() > simplexes.last().vertexBeta().order()? simplexes.last().vertexAlfa():simplexes.last().vertexBeta();
            }

            bool checkRule2 = false;

            if (findMax)
            {
                if (simplexes.last().vertexAlfa().order() == 1)
                    checkRule2 = true;
            }
            else
            {
                if (simplexes.last().vertexZeta().order() == 1)
                    checkRule2 = true;
            }

            if (checkRule2)
            {
                newVertex = (simplexes.last().vertexAlfa() + simplexes.last().vertexZeta()) - simplexes.last().vertexBeta();
                newVertex.setValue(evalImplicitFunction(newVertex.x(), newVertex.y()));

                vertex1 = newVertex;
                vertex2 = simplexes.last().vertexZeta().order() > simplexes.last().vertexAlfa().order()? simplexes.last().vertexAlfa():simplexes.last().vertexZeta();
                vertex3 = simplexes.last().vertexZeta().order() > simplexes.last().vertexAlfa().order()? simplexes.last().vertexZeta():simplexes.last().vertexAlfa();
            }

            Simplex::Target target = Simplex::Undef;

            if (findMax)
                target = Simplex::Max; // simplex was seraching a maximun
            else
                target = Simplex::Min; // simplex was seraching a maximun

            simplexes.append(Simplex(vertex1, vertex2, vertex3, target));
        }
        else
            break;
    }

    return QPointF(simplexes.last().centerOfGravity().x(), simplexes.last().centerOfGravity().y());
}

void FunctionImplicit::updatePoints(const QRect& viewport)
{
    Q_UNUSED(viewport);
    Q_ASSERT(func.expression().isCorrect());

    if(int(resolution())==points.capacity())
        return;

    //double ulimit = uplimit();
    //double dlimit = downlimit();

    points.clear();
    points.reserve(resolution());

    double e = 0.01;
    int iter = 0;
    double k = 1.;

    // initial point
    double xi = initialPoint.x();
    double yi = initialPoint.y();

    double h = 0.1; // best resolution vals < 0.1

    QVector2D oldT(0., 0.);

    int mmmcont = 0;

    for (int i = 0; i < 1024; i++)
    {
        double fx = evalPartialDerivativeX(xi, yi);
        double fy = evalPartialDerivativeY(xi, yi);

        QVector2D T = QVector2D(-fy, fx);
        T.normalize();
        T *= h;

        double cond = oldT.x()*T.x() + oldT.y()*T.y();

        if (cond < 0)
        {
            // change the direction of the tangent for draw
            // curves with quadruple point
            k *= -k;

            mmmcont++;

            //WARNING check this hardcode num
            // in general check another method for singular points
            if (mmmcont > 69)
                T *= -1.0;
        }
        else
        // change the direction of the tangent
            k = 1.;

        T *= k;

        QPointF p0(xi + T.x(), yi + T.y());
        QPointF point = p0;

        oldT = T;

        if (fabs(T.x()) > fabs(T.y())) // m < 1
        {
            double error = 500.0f;
            iter = 0;
            double yant = p0.y();
            double y = 0.0f;

            while (true)
            {
                double fn_val = evalImplicitFunction(p0.x(), yant);
                double dy_val = evalPartialDerivativeY(p0.x(), yant);

                iter++;

                y = yant - fn_val/dy_val;

                if ((error < e) || (iter > 256)) // 256 const max iter num
                    break;

                error = fabs(y - yant);
                yant = y;
            }

            point = QPointF(p0.x(), y);
        }
        else
        {
            double error = 500.0f;
            iter = 0;
            double xant = p0.x();
            double x = 0.0f;

            while (true)
            {
                double fn_val = evalImplicitFunction(xant, p0.y());
                double dx_val = evalPartialDerivativeX(xant, p0.y());

                iter++;

                x = xant - fn_val/dx_val;

                if ((error < e) || (iter > 256)) // 256 const max iter num
                    break;

                error = fabs(x - xant);
                xant = x;
            }

            point = QPointF(x, p0.y());
        }

        addValue(point);

        xi = point.x();
        yi = point.y();
    }
}

QPair<QPointF, QString> FunctionImplicit::calc(const QPointF& point)
{
    // Check for x

    QString expLiteral = func.expression().lambdaBody().toString();
    expLiteral.replace("y", QString::number(point.y()));
    expLiteral.prepend("x->");

    Analitza::Analyzer f(func.variables());
    f.setExpression(Analitza::Expression(expLiteral, false));
    f.refExpression()->parameters()[0]->value() = vx;

    Analitza::Analyzer df(func.variables());
    df.setExpression(f.derivative("x"));
    df.refExpression()->parameters()[0]->value() = vx;

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
        f.refExpression()->parameters()[0]->value() = vy;

        Analitza::Analyzer df(func.variables());
        df.setExpression(f.derivative("y"));
        df.refExpression()->parameters()[0]->value() = vy;

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

        if (!has_root_y)
            return QPair<QPointF, QString>(last_calc, QString(""));
        else
        {
            last_calc = QPointF(point.x(), y);

            return QPair<QPointF, QString>(last_calc, QString(""));
        }
    }
    else
    {
        last_calc = QPointF(x, point.y());

        return QPair<QPointF, QString>(last_calc, QString(""));
    }
}

QLineF FunctionImplicit::derivative(const QPointF& p)
{
    double fx = evalPartialDerivativeX(p.x(), p.y());
    double fy = evalPartialDerivativeY(p.x(), p.y());

    return slopeToLine(-fx/fy);
}
