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

#include <container.h>

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
	QTest::newRow("sum") << "sum(x->1..99, x)" << 4950.;
	QTest::newRow("declare") << "x:=3" << 3.;
	
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
	
	QTest::newRow("sum") << "sum(n->1..99, n)" << "4950";
	QTest::newRow("sum times simplification") << "sum(n->0..99, n*x)" << "4950*x";
	QTest::newRow("sum times") << "x*sum(n->0..99, n)" << "4950*x";
	QTest::newRow("unrelated sum") << "sum(n->0..99, x)" << "99*x";
	
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
	QCOMPARE(a->derivative().toString(), result);
}

void AnalitzaTest::testCorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::addColumn<QString>("result");
	
	QStringList script;
	script << "fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }";
	script << "fib(6)";
	QTest::newRow("piecewise fibonacci") << script << "8";
	
	script.clear();
	script << "fact:=n->piecewise { eq(n,1)?1, ? n*fact(n-1) }";
	script << "fact(5)";
	QTest::newRow("piecewise factorial") << script << "120";
	
	script.clear();
	script << "func:=n->n+1";
	script << "func(5)";
	QTest::newRow("simple function") << script << "6";
}

void AnalitzaTest::testCorrection()
{
	QFETCH(QStringList, expression);
	QFETCH(QString, result);
	
	Analitza b;
	Expression res;
	foreach(QString exp, expression) {
		Expression e(exp, false);
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		res=b.evaluate();
	}
	QCOMPARE(res.toString(), QString(result));
	
	Cn val;
	foreach(QString exp, expression) {
		Expression e(exp, false);
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		val=b.calculate();
	}
	QCOMPARE(val.toString(), result);
}

	void AnalitzaTest::testUncorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::newRow("summatory with unknown uplimit") << QStringList("sum(x->1.., x)");
	QTest::newRow("summatory with unknown downlimit") << QStringList("sum(x->..3, x)");
}

void AnalitzaTest::testUncorrection()
{
	QFETCH(QStringList, expression);
	
	bool correct;
	Analitza b;
	Expression res;
	foreach(QString exp, expression) {
		Expression e(exp, false);
		correct=e.isCorrect();
		
		if(correct) {
			b.setExpression(e);
			correct=b.isCorrect();
			res=b.evaluate();
		}
	}
	QVERIFY(!b.isCorrect());
	
	Cn val;
	foreach(QString exp, expression) {
		Expression e(exp, false);
		correct=e.isCorrect();
		
		if(correct) {
			b.setExpression(e);
			correct=b.isCorrect();
			val=b.calculate();
		}
	}
	QVERIFY(!correct);
}

#include "analitzatest.moc"
