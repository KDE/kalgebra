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
#include "analitza.h"
#include <qtest_kde.h>
#include <cmath>

#include "container.h"
#include "variables.h"
#include "vector.h"

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
	QTest::newRow("sum") << "sum(x : x=1..99)" << 4950.;
	QTest::newRow("diff") << "diff(x)" << 1.;
	QTest::newRow("diffz") << "diff(z:z)" << 1.;
	
	QTest::newRow("product") << "product(n : n=1..5)" << 120.;
	QTest::newRow("factorial") << "factorial(5)" << 120.;
	
	QTest::newRow("simple piecewise") << "piecewise { eq(pi,0)? 3, eq(pi, pi)?33 }" << 33.;
	QTest::newRow("simple piecewise with otherwise") << "piecewise { eq(pi,0)? 3, ?33 }" << 33.;
	QTest::newRow("boolean and") << "and(1,0)" << 0.;
	QTest::newRow("boolean or") << "or(0,1)" << 1.;
	QTest::newRow("boolean not") << "not(0)" << 1.;
	QTest::newRow("lambda") << "(x->x+2)(2)" << 4.;
}

void AnalitzaTest::testTrivialCalculate()
{
	QFETCH(QString, expression);
	QFETCH(double, result);
	Expression e(expression, false);
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	QVERIFY(a->isCorrect());
	QCOMPARE(a->evaluate().toReal().value(), result);
	QVERIFY(a->isCorrect());
	QCOMPARE(a->calculate().toReal().value(), result);
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
	QTest::newRow("minus") << "x-x-x" << "-x";
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
	QTest::newRow("sum times simplification") << "sum(n*x : n=0..99)" << "4950*x";
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

	QTest::newRow("simple polynomial") << "x^3+1" << "3*x^2";
	QTest::newRow("power and sinus") << "x^2+sin(x)" << "2*x+cos(x)";
	QTest::newRow("power") << "x^2" << "2*x";
	QTest::newRow("division") << "1/x" << "(-1)/x^2";
	QTest::newRow("logarithm") << "ln x" << "1/x";
	QTest::newRow("times") << "x*y" << "y";
	QTest::newRow("power derivative and logarithm simplification") << "e^x" << "e^x";
	QTest::newRow("chain rule") << "sin(x**2)" << "2*x*cos(x^2)";
	QTest::newRow("tangent") << "tan(x**2)" << "(2*x)/cos(x^2)^2";
	QTest::newRow("piecewise") << "piecewise { x<0 ? x**2, ? x } " << "piecewise { x<0 ? 2*x, ? 1 }";
}

void AnalitzaTest::testDerivativeSimple()
{
	QFETCH(QString, expression);
	QFETCH(QString, result);
	
	a->setExpression(Expression(expression, false));
	Expression deriv=a->derivative();
	QCOMPARE(deriv.toString(), result);
	QVERIFY(deriv.isCorrect());
	
	double val=1.;
	QList<QPair<QString, double> > vars;
	vars.append(QPair<QString, double>("x", val));
	
	double valCalc=a->derivative(vars);
	a->variables()->modify("x", val);
	a->setExpression(deriv);
	Cn valExp(a->calculate().toReal());
	a->variables()->destroy("x");
	
	QCOMPARE(QString::number(valCalc), QString::number(valExp.value()));
	
	a->setExpression(Expression("diff("+expression+")", false));
	a->simplify();
	QVERIFY(a->isCorrect());
	deriv=a->evaluate();
	QCOMPARE(deriv.toString(), result);
	QVERIFY(a->isCorrect());
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
	
	script.clear();
	script << "n:=9";
	script << "func:=n->n+1";
	script << "func(5)";
	QTest::newRow("simple function, shadowed parameter") << script << "6";
	
	script.clear();
	script << "x:=3";
	script << "x*sum(x : x=0..99)";
	QTest::newRow("bounded scope") << script << "14850";
	
	script.clear();
	script << "f:=x->diff(x^2)";
	script << "f(3)";
	QTest::newRow("diff function") << script << "6";
	
	script.clear();
	script << "fv:=vector{x->x, x->x+2}";
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
}

//testCalculate
void AnalitzaTest::testCorrection()
{
	QFETCH(QStringList, expression);
	QFETCH(QString, result);
	
	Analitza b;
	Expression res;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		if(!e.isCorrect()) qDebug() << "error" << e.error();
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		b.calculate();
		if(!b.isCorrect()) qDebug() << "errors:" << b.errors();
		QVERIFY(b.isCorrect());
	}
	QCOMPARE(b.calculate().toString(), result);
	
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
// 		qDebug() << e.error();
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		res=b.evaluate();
		QVERIFY(b.isCorrect());
	}
	QCOMPARE(res.toString(), result);
}

void AnalitzaTest::testUncorrection_data()
{
	QTest::addColumn<QStringList>("expression");
	QTest::newRow("wrong count") << QStringList("plus(1)");
	QTest::newRow("summatory with unknown uplimit") << QStringList("sum(x : x=1..)");
	QTest::newRow("summatory with unknown downlimit") << QStringList("sum(x : x=..3)");
	QTest::newRow("summatory with uncorrect downlimit") << QStringList("sum(x : x=x..3)");
	QTest::newRow("vect+sin") << QStringList("3+sin(vector{3,4,2})");
	QTest::newRow("scalar+card") << QStringList("card(3)");
	QTest::newRow("wrong parameters") << QStringList("selector(vector{1,1/3})");
	QTest::newRow("wrong operation") << QStringList("lcm(vector{0}, vector{0})");
	QTest::newRow("wrong sum") << QStringList("sum(x : x=10..0)");
	QTest::newRow("recursive var") << QStringList("ww:=ww+1");
	QTest::newRow("nopiece") << QStringList("fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, fib(n-1)+fib(n-2) }");
	
	QStringList script;
	script << "a:=b";
	script << "b:=a";
	QTest::newRow("var dependency cycle") << script;
	
	script.clear();
	script << "x:=3";
	script << "x(3)";
	QTest::newRow("value call") << script;
}

void AnalitzaTest::testUncorrection()
{
	QFETCH(QStringList, expression);
	
	bool correct=false;
	Analitza b;
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
	QVERIFY(!correct);
	
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
	
	QTest::newRow("no var") << "2+2" << "4";
	QTest::newRow("simple") << "x+x" << "2*x";
	QTest::newRow("lambda") << "(x->x+1)(2)" << "3";
// 	QTest::newRow("lambda2") << "(x->x+1)(y+y)" << "2*y+1";
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
}

void AnalitzaTest::testEvaluate()
{
	QFETCH(QStringList, expression);
	QFETCH(QString, result);
	
	Analitza b;
	Expression res;
	foreach(const QString &exp, expression) {
		Expression e(exp, false);
		if(!e.isCorrect()) qDebug() << "XXXX" << e.error();
		QVERIFY(e.isCorrect());
		
		b.setExpression(e);
		QVERIFY(b.isCorrect());
		res=b.evaluate();
		QVERIFY(b.isCorrect());
		b.calculate();
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
	QTest::newRow("selector3+list") << "selector(3, list{1,2}+list{3})" << "3";
}

void AnalitzaTest::testCrash_data()
{
	QTest::addColumn<QString>("expression");
	
	QTest::newRow("undefined variable") << "x";
// 	QTest::newRow("too few operators") << "divide(2)";
// 	QTest::newRow("too much operators") << "divide(2,2,2,2)";
// 	QTest::newRow("empty math") << "<math />";
	QTest::newRow("selector overflow") << "selector(9, vector{1,2})";
	QTest::newRow("selector underflow") << "selector(0, vector{1,2})";
	QTest::newRow("simple piecewise") << "piecewise { pi=0? 3, eq(pi, pi)?33 }";
	QTest::newRow("oscarmartinez piecewise") << "piecewise { gt(x,23)?a }";
// 	QTest::newRow("oscarmartinez derivative") << "diff(gt(x))";
// 	QTest::newRow("product_max") << "product(max(x) : x=1..5)";
	QTest::newRow("vector+ovf") << "selector(2, vector{x})";
	QTest::newRow("wrong func") << "xsin(x)";
	QTest::newRow("scalarprod") << "scalarproduct(vector{0}, vector{x,0})";
}

void AnalitzaTest::testCrash()
{
	QFETCH(QString, expression);
	Expression e(expression, Expression::isMathML(expression));
	QCOMPARE(e.isCorrect(), true);
	
	a->setExpression(e);
	QString str=a->evaluate().toString();
	str=a->calculate().toString();
	
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
	
	#warning improve the test for bounded operations
	if(o.operatorType()==Operator::sum || o.operatorType()==Operator::product) 
		return;
	
	foreach(Object* obj, values) {
		foreach(int paramCnt, params) {
			Container* apply=new Container(Container::apply);
			apply->appendBranch(new Operator(o));
			
			for(; paramCnt>0; paramCnt--)  {
				apply->appendBranch(obj->copy());
			}
			Expression e(apply);
			
			a->setExpression(e);
			a->calculate();
			a->evaluate();
			a->derivative();
			
			if(o.isBounded()) {
				Container *bvar=new Container(Container::bvar);
				apply->m_params.prepend(bvar);
				
				a->calculate();
				a->evaluate();
				a->derivative();
				
				QList<Object*> bvarValues=QList<Object*>() << new Ci("x") << new Cn(0.);
				foreach(Object* obvar, bvarValues) {
					Container* cc=(Container*) apply->copy();
					Container* bvar=(Container*) cc->m_params[0];
					bvar->appendBranch(obvar->copy());
					
					Expression e1(cc);
					a->setExpression(e1);
					
					a->calculate();
					a->evaluate();
					a->derivative();
				}
				qDeleteAll(bvarValues);
			}
		}
	}
	qDeleteAll(values);
	
	QList<double> diffValues = QList<double>() << 0. << 0.5 << -0.5 << 1. << -1.;
	QString bvar="x";
	foreach(double v, diffValues) {
		foreach(int paramCnt, params) {
			Container *diffApply=new Container(Container::apply);
			diffApply->appendBranch(new Operator(Operator::diff));
			
			Container* apply=new Container(Container::apply);
			apply->appendBranch(new Operator(o));
			diffApply->appendBranch(apply);
			
			for(; paramCnt>0; paramCnt--)
				apply->appendBranch(new Ci(bvar));
			
			Expression e(diffApply);
			a->setExpression(e);
			a->calculate();
			a->evaluate();
			a->simplify();
			a->derivative();
			a->derivative(QList< QPair<QString, double> >() << qMakePair(bvar, v));
		}
	}
}

#include "analitzatest.moc"
