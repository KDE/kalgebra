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

#include "expressiontest.h"
#include "analitza.h"
#include "explexer.h"
#include "expressionparser.h"

#include <qtest_kde.h>
#include <cmath>

using namespace std;

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

QString removeTags(const QString& in)
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
	
	QTest::newRow("addition") << "2+4";
	QTest::newRow("addition with var") << "2+x";
	QTest::newRow("function definition") << "f:=x->x+1";
	QTest::newRow("simple addition and subtraction") << "2+x-3";
	QTest::newRow("simple addition and unary minus") << "(-x)+y";
	QTest::newRow("sum") << "sum(x^2:x=1..10)";
	QTest::newRow("piecewise") << "piecewise { x ? y, ? 33 }";
	QTest::newRow("function call") << "f(2)";
	QTest::newRow("vector") << "vector { x, y, z }";
	QTest::newRow("boundedlambda") << "q=0..10->q";
	QTest::newRow("bounded") << "diff(x^2:x)";
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
	
	e->setMathML(parser.mathML());
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

void ExpressionTest::testCorrection_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("isCorrect");
	
	QTest::newRow("simple addition") << "2+4" << true;
	QTest::newRow("simple addition with var") << "2+x" << true;
	QTest::newRow("functin definition") << "f:=x->x+1" << true;
	QTest::newRow("summatory") << "sum(x : x=1..10)" << true;
	QTest::newRow("conditional") << "piecewise { x ? y, ? 33 }" << true;
	QTest::newRow("summatory") << "sum(n*x : n=1..10)" << true;
	QTest::newRow("incorrect bounds") << "product(x,1:3)" << false;
	
	QTest::newRow("addition with missing operand") << "2+" << false;
	QTest::newRow("function definition") << "f:=n->" << false;
	QTest::newRow("piecewise") << "piecewise { ?3, 2 }" << false;
	
	QTest::newRow("limits") << "f:=n->3.." << false;
	QTest::newRow("summatory with unknown uplimit") << "sum(x=1.. : x)" << false;
	//FIXME: Should be false in runtime, controlling it on the compiler.
	//There is no way to have uplimit/downlimit separatedly with the current Exp parser
	
	QTest::newRow("uncotextualized bounds") << "9..99" << false;
	QTest::newRow("uncotextualized bounds") << "9..(9+9)" << false;
	QTest::newRow("uncotextualized bounds") << "x:=9..(9+9)" << false;
	QTest::newRow("uncotextualized bounds") << "3+(9..(9+9))" << false;
	QTest::newRow("uncotextualized bounds") << "3+9..(9+9)" << false;
	QTest::newRow("missing )") << "(" << false;
	QTest::newRow("missing }") << "vector{" << false;
	QTest::newRow("wrong piecewise") << "piecewise { 0 ? 0 ? 0 }" << false;
	QTest::newRow("vector piece") << "vector { 0 ? 0 }" << false;
	QTest::newRow("wrong assignation") << "2:=3" << false;
	QTest::newRow("non-condition in piecewise") << "piecewise{ 2, ?3 }" << false;
	QTest::newRow("not-a-container") << "or{ x }" << false;
	QTest::newRow("different tag") << "prp { x, y, z }" << false;
	QTest::newRow("different tag") << "a+a=10.." << false;
}

void ExpressionTest::testCorrection()
{
	QFETCH(QString, input);
	QFETCH(bool, isCorrect);
	for(int i=0; i<input.length(); i++)
	{
		e->setText(input.mid(0, i));
	}
	e->setText(input);
	
	QCOMPARE(e->isCorrect(), isCorrect);
	QCOMPARE(e->error().isEmpty(), isCorrect);
	foreach(const QString& s, e->error()) {
		QVERIFY(!s.isEmpty());
		if(s.isEmpty())
			qDebug() << e->error();
	}
}

#include "expressiontest.moc"
