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

//BEGIN simplexes: para buscar un punto muy cercano a la curva (o un punto de ella)

// Esta clase representa un vertice que forma parte de un simplex, contiene
// las coordenadas del punto y el valor que toma la funcion implicita
// en esas coordenadas
class Vertex : public QPointF
{
    public:
        // enum Order {First = 0, Middle, Last}; // lo reemplace por un
        // tipo unsigned short int
        // Reemplace este  tipo por un boolean m_upperSide
        // enum Sign {Positive = 0, Negative, None};
        // Suponiendo que k = 0 en 1) T Simplex seq f(x,y) = k

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

        // setSign no es necesario pues el signo dependde del valor
        void setValue(qreal value)
        {
            m_value = value;

            if (value > 0.)
                m_isUpperSide = true;
            else
                // value = 0 tambien? ... es improbable que el valor sea 0
                m_isUpperSide = false;
        }

        void setOrder(unsigned short int order) { m_order = order; }

    private:
        // valor resultado de evaluar la funcion f(x, y) en x() e y() del vertice
        qreal m_value;
        // arriva de k, si es false esta abajo .. si k = 0 si es false
        // esto significa q es negativo
        bool m_isUpperSide;
        // orden en el que se coloco el vertice 1 2 3 (mas reciente a mas antiguo)
        unsigned short int m_order;
};

// Representa un simplex en el espacio 2D, es decir un triangulo equlatero
class Simplex
{
    public:
        // Min : estaba buscando un minimo
        // Undef significa que ya esta¡ en la curva y que no busca ni maximos ni minimos
        enum Target {Undef = 0, Min, Max};

        Simplex();
        // El constructor define el orden de los vertices ... a partir de estos
        // vertices saco quien es el mayor o el menor vertice
        // notar que la arista se saca atraves del calculo de la distancia
        // entre cualquiera de 2 vertices
        Simplex(const Vertex & vertex1, const Vertex & vertex2, const Vertex & vertex3, Target target = Undef);

        // estos 3 vertices representan el orden el primero es 1 y asi...
        Vertex vertex1() const;
        Vertex vertex2() const;
        Vertex vertex3() const;

        // estos 3 vertices significan quien es el mayor de los vertices (el
        // que toma mayor valor al evaluar fx)
        // zeta > beta > alfa
        Vertex vertexAlfa() const;
        Vertex vertexBeta() const;
        Vertex vertexZeta() const;

        qreal edge() const;
        QPointF centerOfGravity() const;
        // Este es el vertice de coordenadas menores, lo ncesito porque cuando
        // se forma un simplex de acuerdo a la formula los siguiente 2 vertices
        // simpre estan a la derecha del vertice incial y
        // siempre estan mas arriva del vertice inicial
        Vertex lowestVertex() const;

        Target target() const; // lo que estaba buscando el simplex

        // para el parche de la primera modificacion del simplex (recorrer curva)
        bool operator == (const Simplex &simplex);
    private:
        // los primero 3 vertices son vertex1, vertex2 y vertex3 los siguientes
        // 3 son vertexAlfa, vertexBeta y vertexZeta
        QVector<Vertex> m_vertexList;
        qreal m_edge;

        Target m_target; // lo que estaba buscando el simplex ... arranca como Undef
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
    //NOTE calculo de la arista, formula = distancia entrecualquiera de 2
    // vertices agarro el vertex 1 y 2
    // lo estoy haciendo lo mas explicito posible, las
    // optimizaciones las dejo para despues ;)
    Vertex vector = vertex2 - vertex1;
    qreal norm = sqrt(vector.x()*vector.x() + vector.y()*vector.y());
    //qreal norm = vector.x()*vector.x() + vector.y()*vector.y();
    m_edge = norm;

    //NOTE Configuraciones de los vertices

    m_vertexList.append(vertex1);
    m_vertexList.append(vertex2);
    m_vertexList.append(vertex3);

    // Actualizamos el orden de los vertices ... recuerda el constructor
    // defines el orden
    m_vertexList[0].setOrder(1);
    m_vertexList[1].setOrder(2);
    m_vertexList[2].setOrder(3);

    // Agregamos los 3 ultimos vertices a ordenandolos conforme a su valor,
    // el de manor valor es alfa y el de mayor es zeta
    m_vertexList.append(vertex1);
    m_vertexList.append(vertex2);
    m_vertexList.append(vertex3);

    // Actualizamos el orden de los 3 ultimos vertices corerspondientes al
    //valorvertices ... recuerda el constructor defines el orden
    m_vertexList[3].setOrder(1);
    m_vertexList[4].setOrder(2);
    m_vertexList[5].setOrder(3);

    // ahora los vamos a ordenar de acuerdo a su valor ... nota solo
    // los 3 ultimos por eso le sumo 3 a los indices
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
    //TODO
    /// Existe una manera mas inteligente de encontrar el punto mas
    /// adecuado que el baricentro del simplex
    /// si tomamos en cuenta que la curva sale por una arista podriamos
    /// encontrar la interseccion de la arsta con la curva (newton-raphson)
    /// y de esta manera usar ese punto para agregarlo, luego
    /// el punto central jalarlo un poka al lado
    /// del vertice mas alejado para que bezier se muestre en toda su dimencion
    /// ver Render by Bezier Curve 1 y 2 (mis apuntes)

    return (vertex1() + vertex2() + vertex3())/3.;
}

Vertex Simplex::lowestVertex() const
{
    QVector<Vertex> vertexArray;
    vertexArray.append(vertex1());
    vertexArray.append(vertex2());
    vertexArray.append(vertex3());

    for (short int i = 0; i < vertexArray.size(); ++i)
        for (short int j = 0; j < vertexArray.size(); ++j)
            if ((vertexArray[i].x() <= vertexArray[j].x()) &&
                (vertexArray[i].y() <= vertexArray[j].y()))
            {
                Vertex tmp = vertexArray[i];
                vertexArray.replace(i, vertexArray[j]);
                vertexArray.replace(j, tmp);
            }

    return vertexArray[0];
}

// lo que estaba buscando el simplex
Simplex::Target Simplex::target() const
{
    return m_target;
}

bool Simplex::operator == (const Simplex &simplex)
{
    return (vertex3() == simplex.vertex3());
}

// Para N = 2 (un simplex en el espacio bidimiencional)
const double P_CONST_COEF = 0.9659258262890682; // (N + sqrt(N + 1.) - 1.)/(sqrt(2.)*N);
const double Q_CONST_COEF = 0.2588190451025207; // (sqrt(N + 1.) - 1.)/(sqrt(2.)*N);

//END simplexes

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
    
    Analitza::Expression partialDerivative(const Analitza::Expression& exp, const QString& var);
    QStringList boundings() const { return supportedBVars(); }

    // meths form implict alg
    void initImplicitFunction();

    qreal evalImplicitFunction(qreal x, qreal y);
    qreal evalPartialDerivativeX(qreal x, qreal y);
    qreal evalPartialDerivativeY(qreal x, qreal y);

    QPointF findBestPointOnCurve(const QPointF &refPoint = QPointF(22.0, -52.0));

    // attrs form implict alg
    Analitza::Cn *vx; // var x
    Analitza::Cn *vy; // var y

    Analitza::Analyzer dx; // partial derivative respect to var x
    Analitza::Analyzer dy; // partial derivative respect to var x

    // arreglo de todos los simplex calculados, necesarios para buscar un
    // punto en la curva
    QVector<Simplex> simplexes;

    QPointF initialPoint;
    QPointF last_calc;
};

REGISTER_FUNCTION(FunctionImplicit)

// input : implict function wich means a lambda in the form (x, y)->f(x,y)
// return a lambda exp: the partial derivative relative to @var
Analitza::Expression FunctionImplicit::partialDerivative(const Analitza::Expression &exp, const QString& var)
{
    return func.derivative(var);
}



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

void FunctionImplicit::initImplicitFunction()
{
    vx = new Analitza::Cn;
    vy = new Analitza::Cn;

    func.refExpression()->parameters()[0]->value() = vx;
    func.refExpression()->parameters()[1]->value() = vy;

    // dx
    //Analitza::Analyzer dx(new Analitza::Variables);
    dx.setExpression(partialDerivative(func.expression(), "x"));

    dx.refExpression()->parameters()[0]->value() = vx;
    dx.refExpression()->parameters()[1]->value() = vy;

    // dy
    //Analitza::Analyzer dy(new Analitza::Variables);
    dy.setExpression(partialDerivative(func.expression(), "y"));

    dy.refExpression()->parameters()[0]->value() = vx;
    dy.refExpression()->parameters()[1]->value() = vy;

    // initial point
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

    // si no tenemos simplex, calculamos el primer simplex, los demas los
    // generamos por reflexion
    // Genero el primer simplex
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
        /// *** ///
        /// Modificacion 2 del simplex seq
        /// Primero buscamos la que el simplex contenga un punto de la funcion,
        /// esto se basa en
        /// comparar los valoras con el cero de f(x,y) ... se basa en valores
        /// *** ///

        // Primero verificamos si el primer simplex esta fuera de la
        // curva (osea si no contiene contiene el punto)
        // lo de abajo nos indica que todos los valores del simplex
        // son positivos en ese caso
        // buscamos un minimo (cero), caso contrario buscamos un maximo (cero)
        //  ... cambando el signo en
        // el ordenamiento del simplex al momento de contruirlo

        // Bandera para saber si todo el simplex esta en el area positiva

        bool isUpperSide = ((simplexes.last().vertex1().isUpperSide() == true) &&
                            (simplexes.last().vertex2().isUpperSide() == true) &&
                            (simplexes.last().vertex3().isUpperSide() == true));

        // Bandera para saber si todo el simplex esta en el area negativa
        bool isUnderSide = ((simplexes.last().vertex1().isUpperSide() == false) &&
                            (simplexes.last().vertex2().isUpperSide() == false) &&
                            (simplexes.last().vertex3().isUpperSide() == false));

        // Si el simplex esta fuera de la curva (en el lado positivo o
        // negativo entonces ejecutar modificacion 2)
        if (isUpperSide || isUnderSide)
        {
            bool findMax;

            if (isUpperSide) // si esta en la parte + entonces buscamos el minimo
                findMax = false;

            if (isUnderSide) // si esta en la parte - entonces buscamos el maximo
                findMax = true;

            //
            //qDebug() << "Simplex ->" << simplexes.last().vertex1().value() << simplexes.last().vertex2().value() << simplexes.last().vertex3().value();
            //qDebug() << "Con v1  ->" << simplexes.last().vertex1().x() <<  " - " << simplexes.last().vertex1().y();
            //qDebug() << "Con v2  ->" << simplexes.last().vertex2().x() <<  " - " << simplexes.last().vertex2().y();
            //qDebug() << "Con v3  ->" << simplexes.last().vertex3().x() <<  " - " << simplexes.last().vertex3().y();
            //qDebug() << "------------------------------------------------------------------";
            //

            // REGLA 1 del simplex seq original
            // Este es el vertice E en el papel "Hallar nuevo simplex"
            Vertex newVertex;

            if (findMax)
                newVertex = (simplexes.last().vertexBeta() +
                             simplexes.last().vertexZeta()) -
                             simplexes.last().vertexAlfa();
            else
                newVertex = (simplexes.last().vertexAlfa() +
                             simplexes.last().vertexBeta()) -
                             simplexes.last().vertexZeta();

            //NOTE CAMBIAR SETEAR EL VALOR INMEDIATAMENTE
            newVertex.setValue(evalImplicitFunction(newVertex.x(), newVertex.y()));

            //NOTE Es necesario constrir los vertices de orden menor a mayor
            // para construir luego el simplex
            // Configuro los vertices del NUEVO simplex 1 2 3 del menos
            // antiguo al mas antiguio
            // Ver "1 Todo el trabajo a nivel de simplex para ver como se
            // convierten los vertices del simplex por iteracion"
            Vertex vertex1 = newVertex;
            Vertex vertex2;
            Vertex vertex3;

            // son diferentes asignacione en caso de querer hallar el maximo l el minimo
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

            // REGLA 2
            // El vertice mas reciente no puede eliminarse, ver libro
            bool checkRule2 = false;

            // son diferentes comparaciones en caso de querer hallar el
            // maximo l el minimo
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

            if (checkRule2) // Si se cumple la regla 2 del simplex secuencial
            {
                // Aqui da lo mimso porque beta siempre es el que se va para la segunda condicion
                newVertex = (simplexes.last().vertexAlfa() + simplexes.last().vertexZeta()) - simplexes.last().vertexBeta();
                newVertex.setValue(evalImplicitFunction(newVertex.x(), newVertex.y()));

                // Configuro los vertices del NUEVO simplex 1 2 3 del
                // menos antiguo al mas antiguio
                // Ver "1 Todo el trabajo a nivel de simplex para ver como
                // se convierten los vertices del simplex por iteracion"
                vertex1 = newVertex;
                vertex2 = simplexes.last().vertexZeta().order() > simplexes.last().vertexAlfa().order()? simplexes.last().vertexAlfa():simplexes.last().vertexZeta();
                vertex3 = simplexes.last().vertexZeta().order() > simplexes.last().vertexAlfa().order()? simplexes.last().vertexZeta():simplexes.last().vertexAlfa();
            }

            Simplex::Target target = Simplex::Undef;

            if (findMax)
                target = Simplex::Max; // el simplex estaba buscando un maximo
            else
                target = Simplex::Min; // el simplex estaba buscando un minimo

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

    double ulimit=uplimit();
    double dlimit=downlimit();

    points.clear();
    points.reserve(resolution());

    //qDebug() << evalImplicitFunction(exp0, QPointF(3, 5));
    //qDebug() << evalImplicitFunction(partialDerivative(func.expression(), 'x'), QPointF(7, 2), va, vb);

    // aqui empieza el algoritmo (tipo de algoritmo = tangencial)
    // completar con implementacion de kmplot (que es de tipo malla)

    double e = 0.01;
    int iter = 0;
    double k = 1.;

    // initial point
    double xi = initialPoint.x();
    double yi = initialPoint.y();

    double h = 0.1; // best resolution vals < 0.1

    QVector2D oldT(0., 0.);

    int iters = static_cast<int>(fabs(qMax(initialPoint.x(), initialPoint.y())))*64;

    //if (iters < 64)
        iters = 1024;

    int mmmcont = 0;

    for (int i = 0; i < iters; i++) //NOTE puede bajarse a 256 para mejorar el performance
    {
        // Calculo la tangente
        double fx = evalPartialDerivativeX(xi, yi);
        double fy = evalPartialDerivativeY(xi, yi);

        QVector2D T = QVector2D(-fy, fx);
        T.normalize();
        T *= h;

        double cond = oldT.x()*T.x() + oldT.y()*T.y();

//        qDebug() << "old tang " << oldT;
//        qDebug() << "cur tang " << T;

        if (cond < 0)
        {
//            qDebug() << "HEREEEEEEEEEEEEEEEEEEEE";
            // Camino facil pero incorrecto no sigue la tangente pero dibuja
            // la lemniscata
            //T = Vector2D(fy, fx);
            k *= -k; // comentado el 2010 en el meetin de kdeedu
            //break;

            // descomentado para el kdeedumeeting pinta test2 :)
            // si lo comento pinta el tes1 ... =O
            //T *= -1.0;

            mmmcont++;

            // NO no
            if (mmmcont > 69)
                T *= -1.0;

            //qDebug() << "una vuelta " << mmmcont << " en iter " << i;
            //out << "New T  >" << T.x() << " | " << T.y() << endl;
            //qDebug() << "New T  > " << T;
        }
        else
        // Importante poner la condicion de else
        // debido a que NO se debe asumir que las iteraciones
        // siempre caeran dentro del caso if
        // Otra solucion seria poner la declaracion de k = 1.
        // a la entrada del buclue principal ... tomar esta
        // ultima consideracion para la implementacion final
        {
            k = 1.;
        }

        T *= k;

        QPointF p0(xi + T.x(), yi + T.y());
        QPointF point = p0;

        oldT = T;

        //END

        if (fabs(T.x()) > fabs(T.y())) // m < 1
        {
            //qDebug() << "m < 1 [caso 1]";

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
        //if (fabs(T.x()) < fabs(T.y())) // m > 1 // no ingresa al bucle ni al anterior
        {
            //qDebug() << "m > 1 [caso 2]";

            double error = 500.0f;
            iter = 0;
            double xant = p0.x();
            double x = 0.0f;

            while (true)
            {
                double fn_val = evalImplicitFunction(xant, p0.y());
                double dx_val = evalPartialDerivativeX(xant, p0.y());

                iter++;

                //qDebug() << fn_val << " " << dx_val << iter;

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
    qreal eval = evalImplicitFunction(p.x(), p.y());
    QLineF ret;

    qreal fx = evalPartialDerivativeX(p.x(), p.y());
    qreal fy = evalPartialDerivativeY(p.x(), p.y());

    QVector2D T = QVector2D(-fy, fx);
    T.normalize();


/*
    QPointF from(p.x() - 5*T.x(), p.y() - 5*T.y());
    QPointF to(p.x() + 5*T.x(), p.y() + 5*T.y());
*/

    return slopeToLine(T.y()/T.x());
//    return QLineF();

//    qreal range = 0.05;
//
//    if ((-range < eval) && (eval < range))
//        ret = QLineF(from, to);

//    return ret;
}

