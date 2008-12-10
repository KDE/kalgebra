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
#include "explexer.h"
#include "expressionparser.h"
#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( ExpTest )
ExpTest::ExpTest(QObject *parent)
 : QObject(parent)
{}

ExpTest::~ExpTest()
{}

Q_DECLARE_METATYPE(QList<uchar>)

void ExpTest::initTestCase() {}

void ExpTest::cleanupTestCase() {}

void ExpTest::testSimple_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("output");
	
	QTest::newRow("1 value") << "1" << "<math><cn>1</cn></math>";
	QTest::newRow("addition") << "1+2" << "<math><apply><plus /><cn>1</cn><cn>2</cn></apply></math>";
	QTest::newRow("substraction") << "2-3" << "<math><apply><minus /><cn>2</cn><cn>3</cn></apply></math>";
	QTest::newRow("unary minus alone") << "-3" << "<math><apply><minus /><cn>3</cn></apply></math>";
	QTest::newRow("x*unary minus") << "x*(-3)" << "<math><apply><times /><ci>x</ci>"
											  "<apply><minus /><cn>3</cn></apply></apply></math>";
	QTest::newRow("unary minus*x") << "-3*x" << "<math><apply><times /><apply><minus />"
											  "<cn>3</cn></apply><ci>x</ci></apply></math>";
	
	QTest::newRow("assignation") << "x:=2+3" << "<math><declare><ci>x</ci><apply><plus /><cn>2</cn>"
													"<cn>3</cn></apply></declare></math>";
	QTest::newRow("simple expression") << "2+1-3" << "<math><apply><minus /><apply><plus />"
										"<cn>2</cn><cn>1</cn></apply><cn>3</cn></apply></math>";
	QTest::newRow("times") << "1*2" << "<math><apply><times /><cn>1</cn><cn>2</cn></apply></math>";
	QTest::newRow("power") << "1^2" << "<math><apply><power /><cn>1</cn><cn>2</cn></apply></math>";
	QTest::newRow("power") << "1**2"<< "<math><apply><power /><cn>1</cn><cn>2</cn></apply></math>";
	QTest::newRow("times") << "1/2" << "<math><apply><divide /><cn>1</cn><cn>2</cn></apply></math>";
	QTest::newRow("priority") << "2+3*5" << "<math><apply><plus /><cn>2</cn><apply><times />"
											"<cn>3</cn><cn>5</cn></apply></apply></math>";
	
	QTest::newRow("function") << "func(x, y)" << "<math><apply><ci type='function'>func</ci>"
												"<ci>x</ci><ci>y</ci></apply></math>";
	QTest::newRow("function_np") << "sin 1/x" << "<math><apply><divide /><apply><sin /><cn>1</cn></apply><ci>x</ci></apply></math>";
	QTest::newRow("block") << "blk{x, y}" << "<math><blk><ci>x</ci><ci>y</ci></blk></math>";
	QTest::newRow("lambda") << "x->(3)" << "<math><lambda><bvar><ci>x</ci></bvar><cn>3</cn></lambda></math>";
	
	QTest::newRow("sum") << "sum(x : x=1..10)" << "<math>"
			"<apply><sum /><bvar><ci>x</ci></bvar><uplimit><cn>10</cn></uplimit><downlimit>"
			"<cn>1</cn></downlimit><ci>x</ci></apply></math>";
	QTest::newRow("x*sum") << "x*sum(x : x=1..10)" <<
			"<math><apply><times /><ci>x</ci><apply><sum /><bvar><ci>x</ci></bvar>"
			"<uplimit><cn>10</cn></uplimit><downlimit><cn>1</cn></downlimit><ci>x</ci></apply></apply></math>";
	QTest::newRow("piecewise") << "piecewise{x?a, ?b}" << "<math><piecewise><piece><ci>a</ci><ci>x</ci></piece>"
														"<otherwise><ci>b</ci></otherwise></piecewise></math>";
	QTest::newRow("piecewise2") << "piecewise{?b}" << "<math><piecewise><otherwise><ci>b</ci></otherwise></piecewise></math>";
	
	QTest::newRow("piecewise2") << "fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }" << "<math><declare><ci>fib</ci><lambda><bvar><ci>n</ci></bvar><piecewise><piece><cn>0</cn><apply><eq /><ci>n</ci><cn>0</cn></apply></piece><piece><cn>1</cn><apply><eq /><ci>n</ci><cn>1</cn></apply></piece><otherwise><apply><plus /><apply><ci type='function'>fib</ci><apply><minus /><ci>n</ci><cn>1</cn></apply></apply><apply><ci type='function'>fib</ci><apply><minus /><ci>n</ci><cn>2</cn></apply></apply></apply></otherwise></piecewise></lambda></declare></math>";
	
	QTest::newRow("lambda2") << "(x, y)->(x+y)" << "<math><lambda><bvar><ci>x</ci></bvar><bvar><ci>y</ci></bvar>"
								"<apply><plus /><ci>x</ci><ci>y</ci></apply></lambda></math>";
	QTest::newRow("lambda3") << "y->y" << "<math><lambda><bvar><ci>y</ci></bvar>"
								"<ci>y</ci></lambda></math>";
	QTest::newRow("unary minus") << "1*(-2)" << "<math><apply><times /><cn>1</cn><apply>"
												"<minus /><cn>2</cn></apply></apply></math>";
	QTest::newRow("boundedlambda") << "q=0..10->q" << "<math><lambda><bvar><ci>q</ci></bvar><uplimit><cn>10</cn>"
										"</uplimit><downlimit><cn>0</cn></downlimit><ci>q</ci></lambda></math>";
	QTest::newRow("bounds and !limit") << "func(x:x)" << "<math><apply><ci type='function'>func</ci>"
									"<bvar><ci>x</ci></bvar><ci>x</ci></apply></math>";
	QTest::newRow("bounds and limit") << "func(x+y : x=0..1)" << 
				"<math><apply><ci type='function'>func</ci>"
				"<bvar><ci>x</ci></bvar><uplimit><cn>1</cn></uplimit><downlimit><cn>0</cn></downlimit>"
				"<apply><plus /><ci>x</ci><ci>y</ci></apply></apply></math>";
	QTest::newRow("bounds and limit") << "card(vector { x, y, z })"
				<< "<math><apply><card /><vector><ci>x</ci><ci>y</ci><ci>z</ci></vector></apply></math>";
}

void ExpTest::testSimple()
{
	QFETCH(QString, input);
	QFETCH(QString, output);
	
	ExpLexer lex(input);
	ExpressionParser parser;
	bool corr=parser.parse(&lex);
	
	if(!parser.error().isEmpty())
		qDebug() << ">>> " << parser.mathML() << "errors:" << parser.error();
	QVERIFY(corr);
	QVERIFY(parser.error().isEmpty());
	QCOMPARE(parser.mathML(), output);
}

void ExpTest::testExp_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QString>("output");

	QString fourX="<math><apply><plus /><ci>x</ci><ci>x</ci><ci>x</ci><ci>x</ci></apply></math>";

	//FIXME: Repetition not suported
// 	QTest::newRow("simple expression") << "x+x+x+x" << fourX;
	QTest::newRow("composed expression") << QString::fromUtf8("2²")
			<< "<math><apply><power /><cn>2</cn><cn>2</cn></apply></math>";
	QTest::newRow("plus operator in plus() form") << "plus(x,x,x,x)" << fourX;
	QTest::newRow("sum") << "x*sum(x : x=1..10)" << "<math><apply><times /><ci>x</ci>"
			"<apply><sum /><bvar><ci>x</ci></bvar><uplimit><cn>10</cn></uplimit><downlimit>"
			"<cn>1</cn></downlimit><ci>x</ci></apply></apply></math>";
	
	QTest::newRow("lol") << "times((x),(y))" << "<math><apply><times /><ci>x</ci><ci>y</ci></apply></math>";
	QTest::newRow("lol1") << "times((x),(y),((z)))" << "<math><apply><times /><ci>x</ci><ci>y</ci><ci>z</ci></apply></math>";
	
}

void ExpTest::testExp()
{
	QFETCH(QString, input);
	QFETCH(QString, output);
	
	ExpLexer lex(input);
	ExpressionParser parser;
	bool corr=parser.parse(&lex);
	if(!parser.error().isEmpty())
		qDebug() << "errors:" << parser.error();
	QVERIFY(parser.error().isEmpty() && corr);
	QCOMPARE(parser.mathML(), output);
}

void ExpTest::testLength_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<QList<uchar> >("lengths");
	
	QList<uchar> lengths;
	lengths << 1 << 1 << 1 << 1 << 1;
	QTest::newRow("simple addition") << "2+2+2" << lengths;
	
	lengths.clear();
	lengths << 1 << 0 << 1;
	QTest::newRow("power with utf composition") << QString::fromUtf8("2²") << lengths;
}

void ExpTest::testLength()
{
	QFETCH(QString, input);
	QFETCH(QList<uchar>, lengths);
	
	ExpLexer lex=ExpLexer(input);
	
	int current=0;
	while(lex.lex()!=0/*EOF*/) {
		QVERIFY(lex.current.type>0);
		QCOMPARE((int) lengths[current], (int) lex.current.len);
		
		current++;
	}
}

void ExpTest::testCorrection_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<bool>("correct");
	
	QTest::newRow("stack killing") << "k+++k" << false;
	QTest::newRow("more stack killing") << "k-++k" << false;
	QTest::newRow("omartinez bug") << "piecewise { gt(x,23)?a }" << true;
}

void ExpTest::testCorrection()
{
	QFETCH(QString, input);
	QFETCH(bool, correct);
	
	ExpLexer lex(input);
	ExpressionParser parser;
	bool corr=parser.parse(&lex);
	QCOMPARE(parser.error().isEmpty() && corr, correct);
}

#include "exptest.moc"
