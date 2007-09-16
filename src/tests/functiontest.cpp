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

#include "functiontest.h"
#include "function.h"
#include "expression.h"
#include <qtest_kde.h>
#include <cmath>

using namespace std;

QTEST_KDEMAIN_CORE( FunctionTest )

FunctionTest::FunctionTest(QObject *parent)
 : QObject(parent)
{}

FunctionTest::~FunctionTest()
{}

void FunctionTest::initTestCase()
{
}

void FunctionTest::cleanupTestCase()
{
}

void FunctionTest::testCopy_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("x->x") << "x";
	QTest::newRow("x->addition") << "2+x";
	QTest::newRow("y->trigonometric") << "y->sin y";
	QTest::newRow("polar->scalar") << "q->2";
	QTest::newRow("polar->function") << "q->sin q";
	QTest::newRow("polar->hard") << "q->(1..10, ceil(q/(2*pi)))";
	QTest::newRow("polar->strange") << "q->q/q";
}

void FunctionTest::testCopy()
{
	QFETCH(QString, input);
	function f("hola", Expression(input, false));
	QVERIFY(f.isCorrect());
	function f2(f);
	QVERIFY(f2.isCorrect());
	function f3;
	f3=f2;
	QVERIFY(f3.isCorrect());
	f3.update_points(QRect(-10, 10, 10, -10), 100);
	f3.derivative(QPointF(0., 0.));
	f3.derivative(QPointF(1., 0.));
	f3.calc(QPointF(0., 0.));
	f3.calc(QPointF(1., 0.));
	f3.calc(QPointF(109., 20.)); // peta aki
}

#include "functiontest.moc"
