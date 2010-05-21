/*************************************************************************************
 *  Copyright (C) 2007-2008 by Aleix Pol <aleixpol@kde.org>                          *
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
#include "analyzer.h"
#include <qtest_kde.h>
#include <cmath>

#include "apply.h"
#include "container.h"
#include "variables.h"
#include "vector.h"
#include "value.h"
#include <variable.h>
#include <analitzautils.h>
//#include <operations.h>

using namespace std;
using Analitza::Cn;
using Analitza::Ci;
using Analitza::Vector;
using Analitza::Object;
using Analitza::Operator;
using Analitza::Container;
using Analitza::Expression;

QTEST_KDEMAIN_CORE( AnalitzaTest )

AnalitzaTest::AnalitzaTest(QObject *parent)
 : QObject(parent)
{}

AnalitzaTest::~AnalitzaTest()
{}

void AnalitzaTest::initTestCase()
{
	a=new Analitza::Analyzer;
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
	QTest::newRow("sum") << "sum(x : x=1..99)" << 4950.;
	QTest::newRow("diff") << "(diff(x:x))(1)" << 1.;
	QTest::newRow("diffz") <<"(diff(z:z))(1)" << 1.;
	
	QTest::newRow("product") << "product(n : n=1..5)" << 120.;
	QTest::newRow("factorial") << "factorial(5)" << 120.;
	
	QTest::newRow("simple piecewise") << "piecewise { eq(pi,0)? 3, eq(pi, pi)?33 }" << 33.;
	QTest::newRow("simple piecewise with otherwise") << "piecewise { eq(pi,0)? 3, ?33 }" << 33.;
	QTest::newRow("boolean and") << "and(1,0)" << 0.;
	QTest::newRow("boolean or") << "or(0,1)" << 1.;
	QTest::newRow("boolean not") << "not(0)" << 1.;
	QTest::newRow("lambda") << "(x->x+2)(2)" << 4.;
	
	//comprehension
	QTest::newRow("sum.2bvars") << "sum(x*y : (x, y)=1..3)" << 36.;
	QTest::newRow("sum.list") << "sum(x : x@list{1,5,44})" << 50.;
}

void AnalitzaTest::testTrivialCalculate()
{
	QFETCH(QString, expression);
	QFETCH(double, result);
	Expression e(expression, false);
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	
	if(!a->isCorrect()) qDebug() << "error: " << a->errors();
	QVERIFY(a->isCorrect());
	QCOMPARE(a->evaluate().toReal().value(), result);
	QVERIFY(a->isCorrect());
	Expression ee=a->calculate();
	QVERIFY(a->isCorrect());
	QCOMPARE(ee.toReal().value(), result);
	QVERIFY(a->isCorrect());
}

void AnalitzaTest::testTrivialEvaluate_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");

	QTest::newRow("simple value") << "2" << "2";
	QTest::newRow("simple addition") << "2+2" << "4";
	QTest::newRow("simple addition with var") << "2+x" << "x+2";
	QTest::newRow("minus irreductibility") << "-x" << "-x";
	QTest::newRow("minus0") << "x-y" << "x-y";
	QTest::newRow("minus1") << "minus(x, y, x)" << "-y";
	QTest::newRow("minus2") << "x-y-y-y-x" << "-3*y";
	QTest::newRow("minus2.1") << "minus(x,y,y,y,x)" << "-3*y";
	QTest::newRow("minus3") << "x-x-x-x-x-x" << "-4*x";
	QTest::newRow("minus3.1") << "x-x-x-x" << "-2*x";
	QTest::newRow("minus3.2") << "minus(x,x,x,x,x,x)" << "-4*x";
	QTest::newRow("addition") << "x+x" << "2*x";
	QTest::newRow("simple polynomial") << "x+x+x**2+x**2" << "2*x+2*x^2";
	QTest::newRow("simplification of unary minus in times") << "x*(-x)" << "-x^2";
	QTest::newRow("strange") << "0*x-1*1" << "-1";
	QTest::newRow("strange2") << "x-x" << "0";
	QTest::newRow("old leak") << "x^1" << "x";
	QTest::newRow("declare") << "wockawocka:=3" << "3";
	QTest::newRow("nested multiplication") << "x*(x+x)" << "2*x^2";
	QTest::newRow("multiplication") << "x*x" << "x^2";
	QTest::newRow("undefined function call") << "f(2)" << "f(2)";
	QTest::newRow("--simplification") << "-(-x)" << "x";
	QTest::newRow("unneeded --simplification") << "-(x-x)" << "0";
	QTest::newRow("minus order") << "1-x" << "1-x";
	QTest::newRow("minus order2") << "x-1" << "x-1";
	QTest::newRow("after simp(minus) --simplification") << "-(x-x-x)" << "x";
	QTest::newRow("and") << "and(6>5, 4<5)" << "true";
	QTest::newRow("or") << "or(6>5, 6<5)" << "true";
	
	QTest::newRow("sum") << "sum(n : n=1..99)" << "4950";
	QTest::newRow("sum times") << "x*sum(n : n=0..99)" << "4950*x";
	QTest::newRow("unrelated sum") << "sum(x : n=0..99)" << "99*x";
	
	QTest::newRow("product") << "product(n : n=1..5)" << "120";
	QTest::newRow("factorial") << "factorial(5)" << "120";
	
	QTest::newRow("simple piecewise") << "piecewise { eq(pi,0)? 3, eq(pi, pi)?33}" << "33";
	QTest::newRow("simple piecewise with otherwise") << "piecewise { eq(pi,0)? 3, ?33}" << "33";
	
	QTest::newRow("lambda") << "f:=q->2" << "q->2";
// 	QTest::newRow("selector lambda") << "selector(2, vector{x->x, x->x+2})" << "x->x+2";
// 	QTest::newRow("boolean and") << "and(x,0)" << "false";

	QTest::newRow("irreductible vector") << "vector { x, y, z }" << "vector { x, y, z }";
	QTest::newRow("in-vector operations") << "vector { x+x, y+y, z-z }" << "vector { 2*x, 2*y, 0 }";
	
	QTest::newRow("vect+vect") << "x+vector { 2, 3, 4 }+vector { 4, 3, 2 }" << "x+vector { 6, 6, 6 }";
	QTest::newRow("vect+2vect") << "2*vector { x, y, z }+vector{x,y,z}" << "3*vector { x, y, z }";
	QTest::newRow("vect+null") << "vector { x, y, z }+vector{0,0,0}" << "vector { x, y, z }";
	QTest::newRow("card") << "card(vector { x, y, z })" << "3";
	QTest::newRow("card+var") << "card(x)" << "card(x)";
	
	QTest::newRow("selector+idx") << "selector(1, vector{x,y,z})" << "x";
	QTest::newRow("selector+var") << "selector(x, vector{x,y,z})" << "selector(x, vector { x, y, z })";
	QTest::newRow("selector+impossible") << "selector(1, v)" << "selector(1, v)";
	
	QTest::newRow("in lists") << "list{w+w}" << "list { 2*w }";
	QTest::newRow("lists") << "union(list{w}, list{x}, list{y,z})" << "list { w, x, y, z }";
	QTest::newRow("lists2") << "union(list{w}, x, list{y}, list{z})" << "union(list { w }, x, list { y, z })";
	
	QTest::newRow("sum.2bvars") << "sum(x*w : (x, y)=1..3)" << "18*w";
// 	QTest::newRow("sum.list") << "sum(x+y : x@list{x,y,z})" << "x+4*y+z";
}

void AnalitzaTest::testTrivialEvaluate()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	
	a->setExpression(Expression(expression, false));
	if(!a->isCorrect())
		qDebug() << "errors:" << a->errors();
	QVERIFY(a->isCorrect());
	QCOMPARE(a->evaluate().toString(), result);
}

void AnalitzaTest::testDerivativeSimple_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");
	
	QTest::newRow("dumb") << "x" << "1";
	QTest::newRow("simple polynomial") << "x^3+1" << "3*x^2";
	QTest::newRow("power and sinus") << "x^2+sin(x)" << "2*x+cos(x)";
	QTest::newRow("power") << "x^2" << "2*x";
	QTest::newRow("division") << "1/x" << "-1/x^2";
	QTest::newRow("logarithm") << "ln x" << "1/x";
	QTest::newRow("times") << "x*y" << "y";
	QTest::newRow("powere") << "e^x" << "e^x"; // power derivative and logarithm simplification
	QTest::newRow("chain rule") << "sin(x**2)" << "2*x*cos(x^2)";
	QTest::newRow("tangent") << "tan(x**2)" << "(2*x)/cos(x^2)^2";
	QTest::newRow("piecewise") << "piecewise { x<0 ? x**2, ? x } " << "piecewise { x<0 ? 2*x, ? 1 }";
	QTest::newRow("lambda") << "x->3" << "0";
	QTest::newRow("timesminus") << "1-x*sin(x)" << "-sin(x)-x*cos(x)";
	QTest::newRow("timesminus2") << "cos(x)-x*sin(x)" << "-2*sin(x)-x*cos(x)";
}

void AnalitzaTest::testDerivativeSimple()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	
	Expression e(expression, false);
	a->setExpression(e);
	QVERIFY(a->isCorrect());
	a->setExpression(a->derivative("x"));
	a->simplify();
	Expression deriv=a->expression();
	QCOMPARE(deriv.toString(), "x->"+result);
	if(!a->isCorrect()) qDebug() << "errors: " << a->errors();
	QVERIFY(a->isCorrect());
	
	double val=1.;
	QList<QPair<QString, double> > vars;
	vars.append(QPair<QString, double>("x", val));
	
	a->setExpression(e);
	double valCalc=a->derivative(vars);
	if(a->isCorrect()) {
		Expression ee(QString("(x->%1)(%2)").arg(result).arg(val));
		a->setExpression(ee);
		QVERIFY(a->isCorrect());
		
		Expression r=a->calculate();
		
		if(a->isCorrect())
			QCOMPARE(QString::number(valCalc), QString::number(r.toReal().value()));
	}
	a->setExpression(Expression("diff("+expression+":x)", false));
	a->simplify();
	QVERIFY(a->isCorrect());
	a->variables()->destroy("x");
	deriv=a->evaluate();
	
	QCOMPARE(deriv.toString(), "x->"+result);
	QVERIFY(a->isCorrect());
}

void AnalitzaTest::testCorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::addColumn<QString>("result");
	
	QStringList script;
	script.clear();
	script << "n:=2";
	script << "n+1";
	QTest::newRow("simple") << script << "3";
	
	script.clear();
	script << "f:=x->x+2";
	script << "f(1)";
	QTest::newRow("simple func") << script << "3";
	
// 	script.clear();
// 	script << "t:=(c, c1, c2, t1, t2)->(t2-t1)/(c2-c1)*(c-c1)+t1";
// 	script << "t(1,2,3,4,5)";
// 	QTest::newRow("long func") << script << "3";
	
	script.clear();
	script << "fact:=n->piecewise { n=1?1, ? n*fact(n-1) }";
	script << "fact(5)";
	QTest::newRow("piecewise factorial") << script << "120";
	
	script.clear();
	script << "fib:=n->piecewise { n=0?0, n=1?1, ?fib(n-1)+fib(n-2) }";
	script << "fib(6)";
	QTest::newRow("piecewise fibonacci") << script << "8";
	
	script.clear();
	script << "n:=vector{1}";
	script << "func:=n->n+1";
	script << "func(5)";
	QTest::newRow("simple function, shadowed parameter") << script << "6";
	
	script.clear();
	script << "x:=3";
	script << "x*sum(x : x=0..99)";
	QTest::newRow("bounded scope") << script << "14850";
	
	script.clear();
	script << "f:=diff(x^2:x)";
	script << "f(3)";
	QTest::newRow("diff function") << script << "6";
	
	script.clear();
	script << "fv:=vector{x->x, x->x+2}";
	script << "(selector(1, fv))(1)";
	script << "(selector(1, fv))(1)+(selector(2, fv))(2)";
	QTest::newRow("selector+lambda") << script << "5";
	
	QTest::newRow("lists") << QStringList("union(list{0}, list{1}, list{2,3})") << "list { 0, 1, 2, 3 }";
	
	script.clear();
	script <<	"valueTableRec := (func, antimages, i) ->"
				"piecewise { i=0 ? list{}, "
					"? union(list{func(selector(i, antimages))}, valueTableRec(func, antimages, i-1))"
				" }"
			<< "valueTableRec(x->x**2, list{1,2,3}, 3)";
	QTest::newRow("yay") << script << "list { 9, 4, 1 }";
	
	script.clear();
	script <<	"findroot:=(der, dee)->piecewise { dee>1 ?"
				"piecewise { rem(der, dee)=0 ? true, ? findroot(der, dee-1)  }, ? false }";
	script << "isprime:=n->not(findroot(n, floor(root(n, 2))))";
	script << "primes:=(from, to)->piecewise { or(from<0, to<0, from>=to)? list{},"
				" isprime(from)? union(list{from}, primes(from+1, to)), ? primes(from+1, to)}";
	script << "primes(1, 25)";
	QTest::newRow("primes") << script << "list { 1, 2, 3, 5, 7, 11, 13, 17, 19, 23 }";
	
	script.clear();
	script << "f:=v->sum(i**2 : i@v)";
	script << "f(list{1,2,3})";
	script << "f(vector{1,2,3})";
	QTest::newRow("sum.list") << script << "14";
}

//testCalculate
void AnalitzaTest::testCorrection()
{
	QFETCH(QStringList, expression);
	QFETCH(QString, result);
	
	Analitza::Analyzer b;
	Expression res;
	
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		qDebug() << "cacacaca" << exp;
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		qDebug() << "p";
		QVERIFY(b.isCorrect());
		res=b.evaluate();
		QVERIFY(b.isCorrect());
	}
	QCOMPARE(res.toString(), result);
	
	Analitza::Analyzer b1;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		if(!e.isCorrect()) qDebug() << "error:" << e.error();
		QVERIFY(e.isCorrect());
		
		b1.setExpression(e);
		
		if(!b1.isCorrect()) qDebug() << "errors: " << b1.errors();
		QVERIFY(b1.isCorrect());
		b1.calculate();
		if(!b1.isCorrect()) qDebug() << "errors:" << e.toString() << b1.errors();
		QVERIFY(b1.isCorrect());
	}
	QCOMPARE(b1.calculate().toString(), result);
}

void AnalitzaTest::testTypeUncorrection()
{
	QFETCH(QStringList, expression);
	
	bool correct=false;
	Analitza::Analyzer b;
	
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		b.setExpression(e);
		correct=b.isCorrect();
		
		if(correct)
			b.calculate().toReal().value();
		
		if(!correct || !b.isCorrect())
			break;
	}
	QVERIFY(!correct);
}

void AnalitzaTest::testTypeUncorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::newRow("vect+sin") << QStringList("3+sin(vector{3,4,2})");
	QTest::newRow("scalar+card") << QStringList("card(3)");
	QTest::newRow("wrong operation") << QStringList("lcm(vector{0}, vector{0})");
	
	QStringList script;
	script << "x:=3";
	script << "x(3)";
	QTest::newRow("value call") << script;
	
	script.clear();
	script << "f:=(x,y)->x*y";
	script << "f(3)";
	QTest::newRow("call missing parameter") << script;
}

void AnalitzaTest::testUncorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::newRow("summatory with unknown uplimit") << QStringList("sum(x : x=1..)");
	QTest::newRow("summatory with unknown downlimit") << QStringList("sum(x : x=..3)");
	QTest::newRow("summatory with uncorrect downlimit") << QStringList("sum(x : x=x..3)");
	QTest::newRow("wrong sum") << QStringList("sum(x : x=10..0)");
	
	QStringList script;
	script << "a:=b";
	script << "b:=a";
	QTest::newRow("var dependency cycle") << script;
}

void AnalitzaTest::testUncorrection()
{
	QFETCH(QStringList, expression);
	
	bool correct=false;
	Analitza::Analyzer b;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		correct=e.isCorrect();
		
		if(correct) {
			b.setExpression(e);
			Expression res=b.evaluate();
			correct=b.isCorrect();
		}
// 		qDebug() << "cycle" << b.isCorrect() << e.toString() << b.errors();
		if(!correct) break;
	}
// 	QVERIFY(!correct);
	
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		correct=e.isCorrect();
		b.setExpression(e);
		
		if(correct) {
			/*double val=*/b.calculate().toReal().value();
			correct=b.isCorrect();
// 			qDebug() << "aaaaaaaaagh"  << b.errors() << val << correct;
		}
		if(!correct) break;
	}
	QVERIFY(!correct);
}

void AnalitzaTest::testSimplify_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");
	
	QTest::newRow("identity") << "1*x" << "x";
	QTest::newRow("minus") << "x-x-x" << "-x";
	QTest::newRow("no var") << "2+2" << "4";
	QTest::newRow("simple") << "x+x" << "2*x";
	QTest::newRow("lambda") << "(x->x+1)(2)" << "3";
// 	QTest::newRow("lambda2")<< "(x->x+x)(x)" << "x->2*x";
	QTest::newRow("diff") << "diff(x^2:x)" << "x->2*x";
	QTest::newRow("sum times") << "sum(n*x : n=0..99)" << "4950*x";
	QTest::newRow("levelout") << "-y-(x+y)" << "-2*y-x";
	QTest::newRow("sum") << "n->sum((i+n) * i : i=0..9)" << "n->sum((i+n)*i:i=0..9)";
	
	QTest::newRow("piecewise1") << "piecewise { 1=2 ? 4, ? 3}" << "3";
	QTest::newRow("piecewise2") << "piecewise { x=2 ? 4, ? 3}" << "piecewise { x=2 ? 4, ? 3 }";
	QTest::newRow("piecewise3") << "piecewise { 2=2 ? 4, ? 3}" << "4";
}

void AnalitzaTest::testSimplify()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	
	a->setExpression(Expression(expression, false));
	QVERIFY(a->isCorrect());
	a->simplify();
	QCOMPARE(a->expression().toString(), result);
}

void AnalitzaTest::testEvaluate_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::addColumn<QString>("result");
	
	QStringList script;
	script << "f:=x->x";
	script << "f(x)";
	QTest::newRow("function parameter") << script << "x";
	
	script.clear();
	script << "pu:=n->sum(p**i:i=0..(floor(n)))";
	script << "pu(3)";
	QTest::newRow("calls") << script << "sum(p^i:i=0..3)";
}

void AnalitzaTest::testEvaluate()
{
	QFETCH(QStringList, expression);
	QFETCH(QString, result);
	
	Analitza::Analyzer b;
	Expression res;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		if(!e.isCorrect()) qDebug() << "XXXX" << e.error();
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		res=b.evaluate();
		QVERIFY(b.isCorrect());
// 		b.calculate(); //we can do that just if we know that all variables doesn't have dependencies
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
	if(!a->isCorrect()) qDebug() << "error:" << a->errors();
	QVERIFY(a->isCorrect());
	QCOMPARE(a->calculate().toString(), result);
	QCOMPARE(a->evaluate().toString(), result);
}

void AnalitzaTest::testVector_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QString>("result");

	QTest::newRow("avector") << "vector { 1, 2, 3 }" << "vector { 1, 2, 3 }";
	QTest::newRow("card(vect)") << "card(vector { 1, 2, 3 })" << "3";
	QTest::newRow("in-vector operations") << "vector { 2+2, 3*3, 3^3 }" << "vector { 4, 9, 27 }";
	
	QTest::newRow("vect+vect") << "vector { 1, 2, 3 }+vector { 3, 2, 1 }" << "vector { 4, 4, 4 }";
	QTest::newRow("vect+vect2") << "vector { 1, 2, 3 }+vector { 3, 2, sin(pi/2) }" << "vector { 4, 4, 4 }";
	QTest::newRow("vect*scalar") << "vector { 1, 2, 3 }*3" << "vector { 3, 6, 9 }";
	QTest::newRow("scalar*vect") << "3*vector { 1, 2, 3 }" << "vector { 3, 6, 9 }";
	
	QTest::newRow("sum") << "sum(vector {x,x,x} : x=1..99)" << "vector { 4950, 4950, 4950 }";
	QTest::newRow("product") << "product(vector {x,x,x} : x=1..5)" << "vector { 120, 120, 120 }";
	
	QTest::newRow("selector1+vector") << "selector(1, vector{1,2,3})" << "1";
	QTest::newRow("selector2+vector") << "selector(2, vector{1,2,3})" << "2";
	QTest::newRow("selector3+vector") << "selector(3, vector{1,2,3})" << "3";
	
	QTest::newRow("selector1+list") << "selector(1, list{1,2,3})" << "1";
	QTest::newRow("selector2+list") << "selector(2, list{1,2,3})" << "2";
	QTest::newRow("selector3+list") << "selector(3, union(list{1,2}, list{3}))" << "3";
}

void AnalitzaTest::testCrash_data()
{
	QTest::addColumn<QString>("expression");
	
	QTest::newRow("undefined variable") << "x";
	QTest::newRow("selector overflow") << "selector(9, vector{1,2})";
	QTest::newRow("selector underflow") << "selector(0, vector{1,2})";
	QTest::newRow("simple piecewise") << "piecewise { pi=0? 3, eq(pi, pi)?33 }";
	QTest::newRow("oscarmartinez piecewise") << "piecewise { gt(x,23)?a }";
	QTest::newRow("vector+ovf") << "selector(2, vector{x})";
	QTest::newRow("wrong func") << "xsin(x)";
	QTest::newRow("scalarprod") << "scalarproduct(vector{0}, vector{x,0})";
}

void AnalitzaTest::testCrash()
{
	QFETCH(QString, expression);
	Expression e(expression, Expression::isMathML(expression));
	QVERIFY(e.isCorrect());
	
	a->setExpression(e);
	a->evaluate();
	a->calculate();
	
	//We don't want it to crash, so we try to
	for(int i=0; i<expression.size(); i++)
	{
		QString aux=expression.left(i);
		QString aux1=expression.right(i);
		
		Expression e1(aux, false);
		Expression e2(aux, true);
		
		Expression e3(aux1, false);
		Expression e4(aux1, true);
	}
}

void AnalitzaTest::testCompareTrees_data()
{
	QTest::addColumn<bool>("matches");
	QTest::addColumn<QString>("pattern");
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QStringList>("outputs");
	
	QStringList outputs = QStringList() << "x" << "2+2";
	QStringList outputsXY = QStringList() << "x" << "2+2" << "y" << "2+3";
	
	QTest::newRow("tree") << true << "x" << "2+2" << outputs;
	QTest::newRow("simple") << true << "sin(x)" << "sin(2+2)" << outputs;
	QTest::newRow("multiple") << true << "sin(x)+sin(x)" << "sin(2+2)+sin(2+2)" << outputs;
	QTest::newRow("deep") << true << "cos(sin(x))" << "cos(sin(2+2))" << outputs;
	QTest::newRow("vector") << true << "vector{x,sin(x),x}" << "vector{2+2,sin(2+2),2+2}" << outputs;
	QTest::newRow("multiple_wrong") << false << "sin(x)+sin(x)" << "sin(2+2)+sin(2+3)" << outputs;
	QTest::newRow("multiple_vars") << true  << "sin(x)+sin(y)" << "sin(2+2)+sin(2+3)" << outputsXY;
}

void AnalitzaTest::testCompareTrees()
{
	QFETCH(bool, matches);
	QFETCH(QString, pattern);
	QFETCH(QString, expression);
	QFETCH(QStringList, outputs);
	
	Expression patt(pattern, false), exp(expression, false);
	QMap<QString, QString> outs;
	QMap<QString, const Object*> outFunc;
	
	for(QStringList::const_iterator it=outputs.constBegin(); it!=outputs.constEnd(); ++it) {
		QString key=*it++;
		outFunc[key]=0;
		outs[key]=*it;
	}
	
	const Object* p=patt.tree();
	bool ret=p->matches(exp.tree(), &outFunc);
	
	QCOMPARE(ret, matches);
	
	foreach(const QString& key, outs.keys()) {
		Q_ASSERT(outFunc.value(key)!=0);
		QCOMPARE(outs[key], outFunc[key]->toString());
	}
}

/*
Q_DECLARE_METATYPE(QList<double>)
Q_DECLARE_METATYPE(Analitza::Bounds)

void AnalitzaTest::testJumps_data()
{
	QTest::addColumn<QString>("expression");
	QTest::addColumn<QList<double> >("values");
	QTest::addColumn< Analitza::Bounds >("bounds");
	
	Analitza::Bounds defB(-1,1);
	Analitza::Bounds bigB(-2,2), trigB(0,pi);
	QTest::newRow("undefined variable") << "1/x" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "rem(x,5)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "tan(x)" << (QList<double>()<<pi/2) << trigB;
	QTest::newRow("undefined variable") << "eq(x,0)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "approx(x,0)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "lt(x,0)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "gt(x,0)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "leq(x,0)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "geq(x,0)" << (QList<double>()<<0.) << defB;
	QTest::newRow("undefined variable") << "or(lt(x,-1), gt(x,1))" << (QList<double>() << -1. << 1.) << bigB;
	QTest::newRow("undefined variable") << "and(lt(x,1), gt(x,-1))" << (QList<double>() << -1. << 1.) << bigB;
}

void AnalitzaTest::testJumps()
{
	QFETCH(QString, expression);
	QFETCH(QList<double>, values);
	QFETCH(Analitza::Bounds, bounds);
	
	Expression e(expression, false);
	QVERIFY(e.isCorrect());
	
	a->setExpression(e);
	a->simplify();
	QVERIFY(a->isCorrect());
	QCOMPARE(a->discontinuities("x", bounds), values);
}*/

void AnalitzaTest::testOperators_data()
{
	QTest::addColumn<int>("i");
	
	for(int i=Operator::none+1; i<Operator::nOfOps; i++) {
		QTest::newRow( Operator::words[i] ) << i;
	}
}

void AnalitzaTest::testOperators()
{
	QFETCH(int, i);
	Operator o(static_cast<Operator::OperatorType>(i));
	QVERIFY(o.nparams()>=-1);
	if(!o.isCorrect())
		qDebug() << o.toString();
	QVERIFY(o.isCorrect());
	QCOMPARE(static_cast<Operator::OperatorType>(i), o.operatorType());
	QCOMPARE(Operator::toOperatorType(o.toString()), o.operatorType());
	
	if(o.operatorType()==Operator::function)
		return;
	
// 	QVERIFY(!Analitza::Operations::infer(o.operatorType()).isEmpty() || !Analitza::Operations::inferUnary(o.operatorType()).isEmpty());
	
	Vector* v=new Vector(3);
	v->appendBranch(new Cn(0.));
	v->appendBranch(new Cn(1.));
	v->appendBranch(new Cn(2.));
	
	QList<Object*> values=QList<Object*>()	<< new Cn(0.)
											<< new Cn(0.5)
											<< new Cn(1.)
											<< new Cn(-1.)
											<< new Cn(-.5)
											<< new Ci("x")
											<< v; //lets try to make it crash
	QList<int> params;
	if(o.nparams()<0)
		params /*<< 0 << 1 << 2*/ << 3;
	else
		params << o.nparams();
	
#ifdef Q_CC_GNU
	#warning improve the test for bounded operations
#endif    
	if(o.operatorType()==Operator::sum || o.operatorType()==Operator::product) 
		return;
	
	foreach(Object* obj, values) {
		foreach(int paramCnt, params) {
			Analitza::Apply* apply=new Analitza::Apply;
			apply->appendBranch(new Operator(o));
			for(; paramCnt>0; paramCnt--)  {
				apply->appendBranch(obj->copy());
			}
			
			if(o.isBounded()) {
				Container *bvar=new Container(Container::bvar);
				apply->appendBranch(bvar);
				
				QList<Object*> bvarValues=QList<Object*>() << new Ci("x");
				foreach(Object* obvar, bvarValues) {
					Analitza::Apply* cc=(Analitza::Apply*) apply->copy();
					Container* bvar=(Container*) cc->bvarCi().first();
					bvar->appendBranch(obvar->copy());
					
					Expression e1(cc);
					a->setExpression(e1);
					
					a->calculate();
					a->evaluate();
					a->derivative("x");
				}
				qDeleteAll(bvarValues);
			} else {
				Expression e(apply);
				a->setExpression(e);
				a->calculate();
				a->evaluate();
				a->derivative("x");
			}
		}
	}
	qDeleteAll(values);
	
	QList<double> diffValues = QList<double>() << 0. << 0.5 << -0.5 << 1. << -1.;
	QString bvar="x";
	foreach(double v, diffValues) {
		foreach(int paramCnt, params) {
			Analitza::Apply *diffApply=new Analitza::Apply;
			diffApply->appendBranch(new Operator(Operator::diff));
			Container* diffBVar=new Container(Container::bvar);
			diffBVar->appendBranch(new Ci(bvar));
			diffApply->appendBranch(diffBVar);
			
			Analitza::Apply* apply=new Analitza::Apply;
			apply->appendBranch(new Operator(o));
			diffApply->appendBranch(apply);
			
			for(; paramCnt>0; paramCnt--)
				apply->appendBranch(new Ci(bvar));
			
			Expression e(diffApply);
			a->setExpression(e);
			a->calculate();
			a->evaluate();
			a->simplify();
			a->derivative("x");
			a->derivative(QList< QPair<QString, double> >() << qMakePair(bvar, v));
		}
	}
}

#include "analitzatest.moc"
