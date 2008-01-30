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
	QTest::newRow("sum") << "sum(x->1..10, x)";
	QTest::newRow("piecewise") << "piecewise { x ? y, ? 33 }";
	QTest::newRow("function call") << "f(2)";
	QTest::newRow("vector") << "vector { x, y, z }";
}

void ExpressionTest::testConversion()
{
	QFETCH(QString, input);
	
	e->setText(input);
	QVERIFY(e->isCorrect());
	QCOMPARE(e->toString(), input);
	QCOMPARE(removeTags(e->toHtml()), input);
}

void ExpressionTest::testCopy_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("simple addition") << "2+4";
	QTest::newRow("simple addition with var") << "2+x";
	QTest::newRow("function definition") << "f:=x->x+1";
	QTest::newRow("summatory") << "sum(x->1..10, x)";
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
}

void ExpressionTest::testCorrection_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("isCorrect");
	
	QTest::newRow("simple addition") << "2+4" << true;
	QTest::newRow("simple addition with var") << "2+x" << true;
	QTest::newRow("functin definition") << "f:=x->x+1" << true;
	QTest::newRow("summatory") << "sum(x->1..10, x)" << true;
	QTest::newRow("conditional") << "piecewise { x ? y, ? 33 }" << true;
	QTest::newRow("conditional") << "sum(n->1..10, n*x)" << true;
	
	QTest::newRow("addition with missing operand") << "2+" << false;
	QTest::newRow("function definition") << "f:=n->" << false;
	QTest::newRow("piecewise") << "piecewise { ?3, 2 }" << false;
	
	QTest::newRow("limits") << "f:=n->3.." << false;
	QTest::newRow("summatory with unknown uplimit") << "sum(x->1.., x)" << false;
		//FIXME: Should be false in runtime, controlling it on the compiler.
		//There is no way to have uplimit/downlimit separatedly with the current Exp parser
}

void ExpressionTest::testCorrection()
{
	QFETCH(QString, input);
	QFETCH(bool, isCorrect);
	e->setText(input);
	QCOMPARE(e->isCorrect(), isCorrect);
}

#include "expressiontest.moc"
