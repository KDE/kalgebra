/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "typechecktest.h"

#include <qtest_kde.h>
#include <cmath>
#include "expression.h"
#include <expressiontypechecker.h>
#include <analitza.h>

QTEST_KDEMAIN_CORE( TypeCheckTest )

TypeCheckTest::TypeCheckTest(QObject* parent)
	: QObject(parent)
	, v(new Analitza::Variables)
{
	v->modify("fnum", Analitza::Expression("x->3"));
	v->modify("fplus", Analitza::Expression("x->x+x"));
	v->modify("tovector", Analitza::Expression("x->vector{x,x}"));
}

TypeCheckTest::~TypeCheckTest()
{
	delete v;
}

void TypeCheckTest::testConstruction_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("output");
	
	QTest::newRow("addition") << "2+4" << "num";
	QTest::newRow("additionvec") << "vector{2,2}+vector{2,2}" << "<num>";
	QTest::newRow("sum") << "sum(x : x=1..3)" << "num";
	QTest::newRow("sumvec") << "sum(vector{x,x} : x=1..3)" << "<num>";
	QTest::newRow("sumvec2") << "sum(x : x=vector{1,1}..vector{3,3})" << "<num>";
	QTest::newRow("vector") << "vector {2,2}" << "<num>";
	QTest::newRow("list") << "list {2,2}" << "[num]";
	QTest::newRow("direct") << "(x->x)(3)" << "num";
	QTest::newRow("call") << "fnum(3)" << "num";
	QTest::newRow("call plus") << "fplus(3)" << "num";
	QTest::newRow("call plus vect") << "fplus(vector{3})" << "<num>";
	QTest::newRow("num to vec") << "tovector(3)" << "<num>";
	QTest::newRow("vec to vec") << "tovector(vector{3})" << "<<num>>";
	QTest::newRow("piecewise") << "piecewise { x=3? 3, ?2 }" << "num";
	QTest::newRow("selector") << "selector(2, vector{3, 3})" << "num";
	QTest::newRow("selector") << "selector(2, vector{vector{3}, vector{3}})" << "<num>";
}

void TypeCheckTest::testConstruction()
{
	QFETCH(QString, input);
	QFETCH(QString, output);
	
	Analitza::Expression e(input);
	QVERIFY(e.isCorrect());
	Analitza::ExpressionTypeChecker t(e, v);
	
	QCOMPARE(t.check().toString(), output);
}
