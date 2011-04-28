/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "expressiontest.h"
#include "analyzer.h"
#include "explexer.h"
#include "expressionparser.h"

#include <qtest_kde.h>
#include <cmath>
#include <analitzautils.h>

using namespace std;
using Analitza::Expression;

QTEST_KDEMAIN_CORE( ExpressionTest )

ExpressionTest::ExpressionTest(QObject *parent)
 : QObject(parent)
{}

ExpressionTest::~ExpressionTest()
{}

void ExpressionTest::initTestCase()
{
	e=new Expression;
}

void ExpressionTest::cleanupTestCase()
{
	delete e;
}

static QString removeTags(const QString& in)
{
	bool tag=false;
	QString out;
	for(int i=0; i<in.length(); i++){
		if(!tag && in[i]=='<')
			tag=true;
		else if(tag && in[i]=='>')
			tag=false;
		else if(!tag) {
			if(in.mid(i,4)=="&gt;"){
				out += '>';
				i+=3;
			} else
				out += in[i];
		}
	}
	return out;
}

void ExpressionTest::testConversion_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("empty") << "";
	QTest::newRow("value") << "2.323232";
	QTest::newRow("addition") << "2+4";
	QTest::newRow("addition with var") << "2+x";
	QTest::newRow("function definition") << "f:=x->x+1";
	QTest::newRow("simple addition and subtraction") << "(2+x)-3";
	QTest::newRow("simple addition and unary minus") << "-x+y";
	QTest::newRow("minusplus") << "-(x+y)";
	QTest::newRow("minuspower") << "-x^2";
	QTest::newRow("sum") << "sum(x^2:x=1..10)";
	QTest::newRow("piecewise") << "piecewise { x ? y, ? 33 }";
	QTest::newRow("function call") << "f(2)";
	QTest::newRow("vector") << "vector { x, y, z }";
	QTest::newRow("bounded") << "diff(x^2:x)";
	QTest::newRow("lambda call") << "(x->x+2)(2)";
	QTest::newRow("lambda call") << "(f[1])(2)";
	QTest::newRow("vectorselect") << "vector { 1, 2, 3 }[1]";
	QTest::newRow("vectorselect2") << "(vector { 1, 2, 3 }+k)[1]";
	QTest::newRow("eq") << "2=2";
	QTest::newRow("vector") << "vector { 1, 2, 3 }";
	QTest::newRow("list") << "list {  }";
	QTest::newRow("listbvar") << "sum(p:x@mylist)";
	
	QTest::newRow("string2") << "\"a\"";
	QTest::newRow("string3") << "\"a&b\"";
	QTest::newRow("string4") << "\"a\\\"b\"";
}

void ExpressionTest::testConversion()
{
	QFETCH(QString, input);
	
	ExpLexer lex(input);
	ExpressionParser parser;
	bool corr=parser.parse(&lex);
	
	if(!parser.error().isEmpty())
		qDebug() << ">>> " << parser.mathML() << "errors:" << parser.error();
	QVERIFY(corr);
	QVERIFY(parser.error().isEmpty());
// 	qDebug() << "result:" << parser.mathML();
	
	e->setMathML(parser.mathML());
	if(!e->isCorrect())
		qDebug() << "semantic errors: " << e->error();
	
	QVERIFY(e->isCorrect());
	QCOMPARE(e->toString(), input);
	QCOMPARE(removeTags(e->toHtml()), input);
	QCOMPARE(parser.mathML(), e->toMathML());
}

void ExpressionTest::testCopy_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("simple addition") << "2+4";
	QTest::newRow("simple addition with var") << "2+x";
	QTest::newRow("function definition") << "f:=x->x+1";
	QTest::newRow("function call") << "f(x, y)";
	QTest::newRow("summatory") << "sum(x:x=1..10)";
	QTest::newRow("conditional") << "piecewise { x ? y, ? 33 }";
	QTest::newRow("vector") << "vector { x, y, z }";
	
	QTest::newRow("simple addition") << "2+4";
	QTest::newRow("simple addition with var") << "2+x";
	QTest::newRow("functin definition") << "f:=x->x+1";
	QTest::newRow("summatory") << "sum(x:x=1..10)";
	QTest::newRow("conditional") << "piecewise { x ? y, ? 33 }";
}

void ExpressionTest::testCopy()
{
	QFETCH(QString, input);
	e->setText(input);
	
	Expression e2(*e);
	QVERIFY(e->isCorrect() && e2.isCorrect());
	QCOMPARE(*e, e2);
	QCOMPARE(e->toString(), input);
	QCOMPARE(removeTags(e->toHtml()), input);
}

void ExpressionTest::testUncorrection_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("incorrect bounds") << "product(x,1:3)";
	
	QTest::newRow("addition with missing operand") << "2+";
	QTest::newRow("function definition") << "f:=n->";
	QTest::newRow("piecewise") << "piecewise { ?3, 2 }";
	
	QTest::newRow("limits") << "f:=n->3..";
	QTest::newRow("summatory with unknown uplimit") << "sum(x=1.. : x)";
	//FIXME: Should be false in runtime, controlling it on the compiler.
	//There is no way to have uplimit/downlimit separatedly with the current Exp parser
	
	QTest::newRow("uncotextualized bounds") << "9..99";
	QTest::newRow("uncotextualized bounds") << "9..(9+9)";
	QTest::newRow("uncotextualized bounds") << "x:=9..(9+9)";
	QTest::newRow("uncotextualized bounds") << "3+(9..(9+9))";
	QTest::newRow("uncotextualized bounds") << "3+9..(9+9)";
	QTest::newRow("missing )") << "(";
	QTest::newRow("missing }") << "vector{";
	QTest::newRow("wrong piecewise") << "piecewise { 0 ? 0 ? 0 }";
	QTest::newRow("vector piece") << "vector { 0 ? 0 }";
	QTest::newRow("wrong assignation") << "2:=3";
	QTest::newRow("non-condition in piecewise") << "piecewise{ 2, ?3 }";
	QTest::newRow("not-a-container") << "or{ x }";
	QTest::newRow("different tag") << "prp { x, y, z }";
	QTest::newRow("different tag") << "a+a=10..";
	QTest::newRow("xxx") << "piecewise {scalarproduct(vector{x, 1/x})}";
	QTest::newRow("wrong piece") << "plus(piece{2+2}, 1,2,3)";
	QTest::newRow("wrong sum") << "sum(x : x)";
	QTest::newRow("nopiece") << "fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, fib(n-1)+fib(n-2) }";
	QTest::newRow("wrong count") << "plus(1)";
	QTest::newRow("wrong parameters") << "selector(vector{1,1/3})";
	QTest::newRow("empty vector") << "vector{}";
	QTest::newRow("bounded sinus") << "sin(x:x)";
	
	QTest::newRow("summatory with unknown uplimit") << "sum(x : x=1..)";
	QTest::newRow("summatory with unknown downlimit") << "sum(x : x=..3)";
	
	QTest::newRow("same args") << "(x,x)->x";
	QTest::newRow("same boundings") << "sum(x : (x,x)=1..10)";
	
	QTest::newRow("empty minus") << "minus()";
}

void ExpressionTest::testUncorrection()
{
	QFETCH(QString, input);
	for(int i=0; i<input.length(); i++)
		e->setText(input.mid(0, i));
	
	e->setText(input);
	
	QVERIFY(!e->isCorrect());
	QVERIFY(!e->error().isEmpty());
	
	QVERIFY(!e->error().contains(QString()));
}

#include "expressiontest.moc"
