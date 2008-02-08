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

#include "exptest.h"
#include "exp.h"
#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( ExpTest )

ExpTest::ExpTest(QObject *parent)
 : QObject(parent)
{}

ExpTest::~ExpTest()
{}

Q_DECLARE_METATYPE(QList<int>)

void ExpTest::initTestCase()
{
}

void ExpTest::cleanupTestCase()
{
}

void ExpTest::testExp_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("output");

	QString fourX="<math><apply><plus /><ci>x</ci><ci>x</ci><ci>x</ci><ci>x</ci></apply></math>";

	QTest::newRow("simple expression") << "x+x+x+x" << fourX;
	QTest::newRow("composed expression") << QString::fromUtf8("2²")
			<< "<math><apply><power /><cn>2</cn><cn>2</cn></apply></math>";
	QTest::newRow("plus operator in plus() form") << "plus(x,x,x,x)" << fourX;
	QTest::newRow("sum") << "x*sum(x->1..10, x)" << "<math><apply><times /><ci>x</ci>"
			"<apply><sum /><bvar><ci>x</ci></bvar><uplimit><cn>10</cn></uplimit><downlimit>"
			"<cn>1</cn></downlimit><ci>x</ci></apply></apply></math>";
	
	QTest::newRow("lol") << "times((x),(y))" << "<math><apply><times /><ci>x</ci><ci>y</ci></apply></math>";
	QTest::newRow("lol1") << "times((x),(y),((z)))" << "<math><apply><times /><ci>x</ci><ci>y</ci><ci>z</ci></apply></math>";
	
}

void ExpTest::testExp()
{
	QFETCH(QString, input);
	QFETCH(QString, output);
	
	Exp e(input);
	e.parse();
	if(!e.error().isEmpty())
		qDebug() << "errors:" << e.error();
	QVERIFY(e.error().isEmpty());
	QCOMPARE(e.mathML(), output);
}

void ExpTest::testLength_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QList<int> >("lengths");
	
	QList<int> lengths;
	lengths << 1 << 1 << 1 << 1 << 1;
	QTest::newRow("simple addition") << "2+2+2" << lengths;
	
	lengths.clear();
	lengths << 1 << 0 << 1;
	QTest::newRow("power with utf composition") << QString::fromUtf8("2²") << lengths;
}

void ExpTest::testLength()
{
	QFETCH(QString, input);
	QFETCH(QList<int>, lengths);
	
	int len=-1, pos;
	QString op=input.trimmed();
	TOKEN t=Exp::getToken(op, len, tMaxOp);
	
	for(pos=0; pos<input.length() && input[pos].isSpace(); pos++);
	
	int current=0;
	while(!op.isEmpty() && pos < input.length() && t.type!=tEof) {
		QVERIFY(t.type!=tMaxOp);
		pos += len;
		QCOMPARE(lengths[current], len);
		
		t=Exp::getToken(op, len, t.type);
		current++;
	}
}

void ExpTest::testCorrection_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("correct");
	
	QTest::newRow("stack killing") << "k+++k" << false;
	QTest::newRow("more stack killing") << "k-++k" << false;
}

void ExpTest::testCorrection()
{
	QFETCH(QString, input);
	QFETCH(bool, correct);
	
	Exp e(input);
	e.parse();
	QCOMPARE(e.error().isEmpty(), correct);
}

#include "exptest.moc"
