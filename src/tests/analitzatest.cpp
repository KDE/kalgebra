/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include "analitzatest.h"
#include "analitza.h"
#include <qtest_kde.h>
#include <cmath>

#include "container.h"
#include "variables.h"

using namespace std;

QTEST_KDEMAIN_CORE( AnalitzaTest )

AnalitzaTest::AnalitzaTest(QObject *parent)
 : QObject(parent)
{}

AnalitzaTest::~AnalitzaTest()
{}

void AnalitzaTest::initTestCase()
{
	a=new Analitza;
}

void AnalitzaTest::cleanupTestCase()
{
	delete a;
}

void AnalitzaTest::testTrivialCalculate_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<double>("result");

	QTest::newRow("a value") << "2" << 2.;
	QTest::newRow("simple addition") << "2+2" << 4.;
	QTest::newRow("simple power") << "2**99" << pow(2., 99.);
	QTest::newRow("simple multiplication") << "3*3" << 9.;
	QTest::newRow("sinus") << "sin(3*3)" << sin(9.);
	QTest::newRow("declare") << "x:=3" << 3.;
	QTest::newRow("sum") << "sum(x->1..99, x)" << 4950.;
	
	QTest::newRow("product") << "product(n->1..5, n)" << 120.;
	QTest::newRow("factorial") << "factorial(5)" << 120.;
	
	QTest::newRow("simple piecewise") << "piecewise { eq(pi,0)? 3, eq(pi, pi)?33 }" << 33.;
	QTest::newRow("simple piecewise with otherwise") << "piecewise { eq(pi,0)? 3, ?33 }" << 33.;
}

void AnalitzaTest::testTrivialCalculate()
{
	QFETCH(QString, expression);
	QFETCH(double, result);
	Expression e(expression, false);
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	QCOMPARE(a->isCorrect(), true);
	QCOMPARE(a->calculate().value(), result);
}

void AnalitzaTest::testTrivialEvaluate_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");

	QTest::newRow("simple value") << "2" << "2";
	QTest::newRow("simple addition") << "2+2" << "4";
	QTest::newRow("simple addition with var") << "2+x" << "x+2";
	QTest::newRow("minus irreductibility") << "-x" << "-x";
	QTest::newRow("minus") << "x-x-x" << "-x";
	QTest::newRow("minus2") << "x-y-y-y-x" << "-y";
	QTest::newRow("minus3") << "x-x-x-x-x-x" << "-4*x";
	QTest::newRow("minus4") << "x-(-x)-x-x" << "0";
	QTest::newRow("addition") << "x+x" << "2*x";
	QTest::newRow("simple polynomial") << "x+x+x**2+x**2" << "2*x+2*x^2";
	QTest::newRow("simplification of unary minus in times") << "x*(-x)" << "-x^2";
	QTest::newRow("strange") << "0*x-1*1" << "-1";
	QTest::newRow("strange2") << "x-x" << "0";
	QTest::newRow("old leak") << "x^1" << "x";
	QTest::newRow("declare") << "w:=3" << "3";
	QTest::newRow("nested multiplication") << "x*(x+x)" << "2*x^2";
	QTest::newRow("multiplication") << "x*x" << "x^2";
	QTest::newRow("undefined function call") << "f(2)" << "f(2)";
	QTest::newRow("--simplification") << "-(-x)" << "x";
	QTest::newRow("unneeded --simplification") << "-(x-x)" << "0";
	QTest::newRow("after simp(minus) --simplification") << "-(x-x-x)" << "x";
	
	QTest::newRow("sum") << "sum(n->1..99, n)" << "4950";
	QTest::newRow("sum times simplification") << "sum(n->0..99, n*x)" << "4950*x";
	QTest::newRow("sum times") << "x*sum(n->0..99, n)" << "4950*x";
	QTest::newRow("unrelated sum") << "sum(n->0..99, x)" << "99*x";
	
	QTest::newRow("product") << "product(n->1..5, n)" << "120";
	QTest::newRow("factorial") << "factorial(5)" << "120";
	
	QTest::newRow("simple piecewise") << "piecewise { eq(pi,0)? 3, eq(pi, pi)?33}" << "33";
	QTest::newRow("simple piecewise with otherwise") << "piecewise { eq(pi,0)? 3, ?33}" << "33";
}

void AnalitzaTest::testTrivialEvaluate()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	
	a->setExpression(Expression(expression, false));
	QVERIFY(a->isCorrect());
	QCOMPARE(a->evaluate().toString(), result);
}

void AnalitzaTest::testDerivativeSimple_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");

	QTest::newRow("simple polynomial") << "x^3+1" << "3*x^2";
	QTest::newRow("power and sinus") << "x^2+sin(x)" << "2*x+cos(x)";
	QTest::newRow("power") << "x^2" << "2*x";
	QTest::newRow("division") << "1/x" << "(-1)/x^2";
	QTest::newRow("logarithm") << "ln x" << "1/x";
	QTest::newRow("power derivative and logarithm simplification") << "e^x" << "e^x";
	QTest::newRow("chain rule") << "sin(x**2)" << "2*x*cos(x^2)";
	QTest::newRow("tangent") << "tan(x**2)" << "(2*x)/cos(x^2)^2";
	QTest::newRow("piecewise") << "piecewise { lt(x, 0) ? x**2, ? x } " << "piecewise { lt(x, 0) ? 2*x, ? 1 }";
}

void AnalitzaTest::testDerivativeSimple()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	
	a->setExpression(Expression(expression, false));
	Expression deriv=a->derivative();
	QCOMPARE(deriv.toString(), result);
	
	double val=1.;
	QList<QPair<QString, double> > vars;
	vars.append(QPair<QString, double>("x", val));
	
	double valCalc=a->derivative(vars);
	a->m_vars->modify("x", val);
	a->setExpression(deriv);
	Cn valExp(a->calculate().value());
	a->m_vars->destroy("x");
	
	QCOMPARE(QString::number(valCalc), QString::number(valExp.value()));
}

void AnalitzaTest::testCorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::addColumn<double>("result");
	
	QStringList script;
	script << "fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }";
	script << "fib(6)";
	QTest::newRow("piecewise fibonacci") << script << 8.;
	
	script.clear();
	script << "fact:=n->piecewise { eq(n,1)?1, ? n*fact(n-1) }";
	script << "fact(5)";
	QTest::newRow("piecewise factorial") << script << 120.;
	
	script.clear();
	script << "func:=n->n+1";
	script << "func(5)";
	QTest::newRow("simple function") << script << 6.;
	
	script.clear();
	script << "n:=9";
	script << "func:=n->n+1";
	script << "func(5)";
	QTest::newRow("simple function, shadowed parameter") << script << 6.;
	
	script.clear();
	script << "x:=3";
	script << "x*sum(x->0..99, x)";
	QTest::newRow("sum scope") << script << 14850.;
}

void AnalitzaTest::testCorrection()
{
	QFETCH(QStringList, expression);
	QFETCH(double, result);
	
	Analitza b;
	Expression res;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		res=b.evaluate();
	}
	QCOMPARE(res.toString(), Cn(result).toString());
	
	double val;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		val=b.calculate().value();
	}
	QCOMPARE(val, result);
}

void AnalitzaTest::testUncorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::newRow("different tag") << QStringList("prp { x, y ,z }");
	QTest::newRow("summatory with unknown uplimit") << QStringList("sum(x->1.., x)");
	QTest::newRow("summatory with unknown downlimit") << QStringList("sum(x->..3, x)");
}

void AnalitzaTest::testUncorrection()
{
	QFETCH(QStringList, expression);
	
	bool correct;
	Analitza b;
	Expression res;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		correct=e.isCorrect();
		
		if(correct) {
			b.setExpression(e);
			correct=b.isCorrect();
			res=b.evaluate();
		}
	}
	QVERIFY(!b.isCorrect());
	
	double val;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		correct=e.isCorrect();
		
		if(correct) {
			b.setExpression(e);
			correct=b.isCorrect();
			val=b.calculate().value();
		}
	}
	QVERIFY(!correct);
}

void AnalitzaTest::testEvaluate_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::addColumn<QString>("result");
	
	QStringList script;
	script << "f:=x->x";
	script << "f(x)";
	QTest::newRow("function parameter") << script << "x";
}

void AnalitzaTest::testEvaluate()
{
	QFETCH(QStringList, expression);
	QFETCH(QString, result);
	
	Analitza b;
	Expression res;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		res=b.evaluate();
	}
	QCOMPARE(res.toString(), result);
}

void AnalitzaTest::testVector()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	Expression e(expression, false);
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	QCOMPARE(a->isCorrect(), true);
	QCOMPARE(a->calculate().toString(), result);
}

void AnalitzaTest::testVector_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");

	QTest::newRow("irreductible vector") << "vector { 1, 2, 3 }" << "vector { 1, 2, 3 }";
	QTest::newRow("in-vector operations") << "vector { 2+2, 3*3, 3^3 }" << "vector { 4, 9, 27 }";
	
	QTest::newRow("vect+vect") << "vector { 1, 2, 3 }+vector { 3, 2, 1 }" << "vector { 4, 4, 4 }";
	QTest::newRow("vect+vect2") << "vector { 1, 2, 3 }+vector { 3, 2, sin(pi/2) }" << "vector { 4, 4, 4 }";
	QTest::newRow("vect*scalar") << "vector { 1, 2, 3 }*3" << "vector { 3, 6, 9 }";
	QTest::newRow("scalar*vect") << "3*vector { 1, 2, 3 }" << "vector { 3, 6, 9 }";
}

void AnalitzaTest::testVectorEvaluate()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	Expression e(expression, false);
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	QCOMPARE(a->isCorrect(), true);
	QCOMPARE(a->evaluate().toString(), result);
}

void AnalitzaTest::testVectorEvaluate_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");

	QTest::newRow("irreductible vector") << "vector { x, y, z }" << "vector { x, y, z }";
	QTest::newRow("in-vector operations") << "vector { x+x, y+y, z-z }" << "vector { 2*x, 2*y, 0 }";
	
	QTest::newRow("vect+vect") << "x+vector { 2, 3, 4 }+vector { 4, 3, 2 }" << "x+vector { 6, 6, 6 }";
	QTest::newRow("vect+2vect") << "2*vector { x, y, z }+vector{x,y,z}" << "3*vector { x, y, z }";
	QTest::newRow("vect+null") << "vector { x, y, z }+vector{0,0,0}" << "vector { x, y, z }";
}

void AnalitzaTest::testCrash_data()
{
	QTest::addColumn<QString>("expression");

	QTest::newRow("undefined variable") << "x";
}

void AnalitzaTest::testCrash()
{
	QFETCH(QString, expression);
	Expression e(expression, false);
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	QString str=a->calculate().toString();
}

#include "analitzatest.moc"
