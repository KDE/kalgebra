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

void ExpressionTest::testConversion_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("simple expression") << "2+4";
	QTest::newRow("simple expression") << "2+x";
	QTest::newRow("simple expression") << "f:=x->x+1";
	QTest::newRow("simple expression") << "sum(x->1..10, x)";
}

void ExpressionTest::testConversion()
{
	QFETCH(QString, input);
	
	e->setText(input);
	QCOMPARE(e->toString(), input);
}

void ExpressionTest::testExp_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("output");

	QString fourX="<math><apply><plus /><ci>x</ci><ci>x</ci><ci>x</ci><ci>x</ci></apply></math>";

	QTest::newRow("simple expression") << "x+x+x+x" << fourX;
	QTest::newRow("plus operator in plus() form") << "plus(x,x,x,x)" << fourX;
}

void ExpressionTest::testExp()
{
	QFETCH(QString, input);
	QFETCH(QString, output);

	e->setText(input);
	QCOMPARE(e->toMathML(), output);
}


#include "expressiontest.moc"
