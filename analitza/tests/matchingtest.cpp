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

#include "matchingtest.h"
#include <qtest_kde.h>

#include <analitza/expression.h>
#include <substituteexpression.h>

QTEST_KDEMAIN_CORE( MatchingTest )

using namespace Analitza;

void MatchingTest::testCompareTrees_data()
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
	QTest::newRow("diff") << true  << "diff(x:x)" << "diff(x:x)" << (QStringList("x") << "x");
	QTest::newRow("correct boundings") << false << "diff(x**2:x)" << "diff(x:x)" << (QStringList("x") << "x");
	
	QTest::newRow("bounded") << true  << "diff(sum(p : w=a..b))" << "diff(sum(x+x:x=0..10))" << (QStringList() <<
		"p" << "x+x" << "w" << "x" << "a" << "0" << "b" << "10");
}

void MatchingTest::testCompareTrees()
{
	QFETCH(bool, matches);
	QFETCH(QString, pattern);
	QFETCH(QString, expression);
	QFETCH(QStringList, outputs);
	
	Expression patt(pattern, false), exp(expression, false);
	QMap<QString, QString> outs;
	
	for(QStringList::const_iterator it=outputs.constBegin(); it!=outputs.constEnd(); ++it) {
		QString key=*it++;
		outs[key]=*it;
	}
	
	QMap<QString, const Object*> outFunc;
	const Object* p=patt.tree();
	bool ret=p->matches(exp.tree(), &outFunc);
	
	QCOMPARE(ret, matches);
	
	foreach(const QString& key, outs.keys()) {
		QVERIFY(outFunc.value(key)!=0);
		QCOMPARE(outs[key], outFunc[key]->toString());
	}
}

void MatchingTest::testSubstitutions_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("pattern");
	QTest::addColumn<QString>("substitution");
	QTest::addColumn<QString>("result");
	
	QTest::newRow("bvars") << "diff(sum(f(x+y):y=0..10):x)" << "diff(sum(x:p=a..b):w)" << "sum(diff(x:w):p=a..b)" << "sum(diff(f(x+y):x):y=0..10)";
	QTest::newRow("sinsquare") << "sin(x**2)" << "sin x" << "sin(2*x)" << "sin(2*x^2)";
	QTest::newRow("sindiff") << "diff(sin(cos x):x)" << "diff(sin w:x)" << "diff(w:x)*cos(w)" << "diff(cos(x):x)*cos(cos(x))";
	QTest::newRow("sindiff-y") << "diff(sin(cos y):y)" << "diff(sin w:x)" << "diff(w:x)*cos(w)" << "diff(cos(y):y)*cos(cos(y))";
}

void MatchingTest::testSubstitutions()
{
	QFETCH(QString, input);
	QFETCH(QString, pattern);
	QFETCH(QString, substitution);
	QFETCH(QString, result);
	
	Expression patt(pattern, false), subs(substitution, false), in(input, false);
	QMap<QString, const Object*> outFunc;
	
	bool ret=patt.tree()->matches(in.tree(), &outFunc);
	QVERIFY(ret);
	
// 	for(QMap<QString, const Object*>::const_iterator it=outFunc.constBegin(), itEnd=outFunc.constEnd(); it!=itEnd; ++it) {
// 		qDebug() << "value:" << it.key() << it.value()->toString();
// 	}
	
	SubstituteExpression substitutor;
	QSharedPointer<Object> substituted(substitutor.run(subs.tree(), outFunc));
	
	QCOMPARE(substituted->toString(), result);
}