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

#include "functiontest.h"
#include "function.h"
#include "expression.h"
#include "analitza.h"
#include "variables.h"
#include <qtest_kde.h>
#include <cmath>

using namespace std;

QTEST_KDEMAIN_CORE( FunctionTest )

FunctionTest::FunctionTest(QObject *parent)
	: QObject(parent)
{
	m_vars=new Variables;
}

FunctionTest::~FunctionTest()
{
	delete m_vars;
}

void FunctionTest::initTestCase()
{}

void FunctionTest::cleanupTestCase()
{}

void FunctionTest::testCopy_data()
{
	QTest::addColumn<QString>("input");
	
	QTest::newRow("x->flat") << "1";
	QTest::newRow("x->x") << "x";
	QTest::newRow("x->and") << "and(gt(x,-1), lt(x,1))";
	QTest::newRow("x->abs") << "abs(x)";
	QTest::newRow("x->addition") << "2+x";
	QTest::newRow("x->minus") << "x-2";
	QTest::newRow("x->log") << "log x";
	QTest::newRow("x->tan") << "tan x";
	QTest::newRow("x->factorof") << "factorof(x,x)";
	QTest::newRow("x->sum") << "sum(t : t=0..3)";
	QTest::newRow("x->piece") << "piecewise { gt(x,0) ? selector(1, vector{x, 1/x}),"
									"? selector(2, vector{x, 1/x} ) }";
	QTest::newRow("x->diff") << "diff(x)";
	QTest::newRow("x->diffx") << "diff(x^2)";
	QTest::newRow("y->flat") << "y->1";
	QTest::newRow("y->trigonometric") << "y->sin y";
	QTest::newRow("polar->scalar") << "q->2";
	QTest::newRow("polar->function") << "q->sin q";
	QTest::newRow("polar->hard") << "q=1..10->ceiling(q/(2*pi))";
	QTest::newRow("polar->hard") << "q=2..(4+4)->ceiling(q/(2*pi))";
	QTest::newRow("polar->strange") << "q->q/q";
	
	QTest::newRow("parametric") << "t->vector{t,t**2}";
}

#include "container.h"

void FunctionTest::testCopy()
{
	QFETCH(QString, input);
	Expression exp(input, false);
	if(input.contains("..")) {
		QVERIFY(exp.uplimit().isCorrect());
		QVERIFY(exp.downlimit().isCorrect());
	}
	
	function f("hola", exp, m_vars);
	if(!f.isCorrect()) qDebug() << "xxxxxx" << f.errors();
	QVERIFY(f.isCorrect());
	function f2(f);
	QVERIFY(f2.isCorrect());
	function f3;
	f3=f2;
	QVERIFY(f3.isCorrect());
	QRectF viewp(QPoint(-12, 10), QPoint(12, -10));
	int resolution=800;
	f3.update_points(viewp.toRect(), resolution);
	
	QVERIFY(f3.points().size()>1);
	QVERIFY(f3.points().size()<=resolution);
	f3.update_points(viewp.toRect(), resolution);
	
// 	bool found=false;
// 	foreach(const QPointF& pt, f3.points()) {
// 		if(viewp.contains(pt)) {
// 			found=true;
// 			break;
// 		}
// 	}
// 	QVERIFY(found);
	
	int ant=-1;
	foreach(int pos, f3.jumps()) {
		QVERIFY(pos>ant);
		ant=pos;
	}
	
	f3.derivative(QPointF(0., 0.));
	f3.derivative(QPointF(1., 0.));
	f3.calc(QPointF(0., 0.));
	f3.calc(QPointF(1., 0.));
	f3.calc(QPointF(109., 20.));
}
void FunctionTest::testCorrect_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("correct");
	
	QTest::newRow("empty function") << "" << false;
// 	QTest::newRow("q->empty range") << "q=0..0->(q)" << false;
	QTest::newRow("undefined var") << "x:=y" << false;
	QTest::newRow("parametric-novector") << "t->3" << false;
	QTest::newRow("parametric-wrongvector") << "t->vector{}" << false;
}

void FunctionTest::testCorrect()
{
	QFETCH(QString, input);
	QFETCH(bool, correct);
	function f3("hola", Expression(input, false), m_vars);
	bool corr=f3.isCorrect();
	
	if(corr) {
		f3.calc(QPointF(0,0));
		f3.update_points(QRect(-10, 10, 10, -10), 100);
	}
	corr=corr && f3.isCorrect();
	
// 	qDebug() << f3.errors();
	QCOMPARE(correct, corr);
	QCOMPARE(correct, f3.isCorrect());
	
	if(correct)
	{
		f3.update_points(QRect(-10, 10, 10, -10), 100);
		QVERIFY(f3.points().size()>1);
		QVERIFY(f3.points().size()<=100);
	}
}

#include "functiontest.moc"
