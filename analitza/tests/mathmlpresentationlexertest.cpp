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

#include "mathmlpresentationlexertest.h"
#include "mathmlpresentationlexer.h"
#include "expressionparser.h"
#include "expression.h"
#include <qtest_kde.h>

QTEST_KDEMAIN_CORE(MathMLPresentationLexerTest)
Q_DECLARE_METATYPE(AbstractLexer::TOKEN*)

MathMLPresentationLexerTest::MathMLPresentationLexerTest(QObject *parent)
 : QObject(parent)
{}

MathMLPresentationLexerTest::~MathMLPresentationLexerTest()
{}

void MathMLPresentationLexerTest::initTestCase() {}
void MathMLPresentationLexerTest::cleanupTestCase() {}

void MathMLPresentationLexerTest::testSimple_data()
{
	QTest::addColumn<QString>("input");
	QTest::addColumn<AbstractLexer::TOKEN*>("output");
	
	QTest::newRow("1 value") << "<mn>123</mn>" << new AbstractLexer::TOKEN(ExpressionTable::tVal, 0, "<cn>123</cn>");
	QTest::newRow("1 variable") << "<mi>x</mi>" << new AbstractLexer::TOKEN(ExpressionTable::tVal, 0, "<ci>x</ci>");
// 	QTest::newRow("1 variable pi") << "<mi>&pi;</mi>" << new AbstractLexer::TOKEN(ExpressionTable::tVal, 0, "<ci>&pi;</ci>");
	
	QTest::newRow("1 operator") << "<mo>+</mo>" << new AbstractLexer::TOKEN(ExpressionTable::tAdd, 0, QString());
}

void MathMLPresentationLexerTest::testSimple()
{
	QFETCH(QString, input);
	QFETCH(AbstractLexer::TOKEN*, output);
	
	MathMLPresentationLexer lex(input);
	
	int t=lex.lex();
	
	if(!lex.error().isEmpty())
		qDebug() << "error!" << lex.error();
	QVERIFY(lex.error().isEmpty());
	QCOMPARE(lex.current.type, output->type);
	QCOMPARE(t, output->type);
	QCOMPARE(lex.current.val, output->val);
	
	delete output;
}

void MathMLPresentationLexerTest::testConversion_data()
{
	QTest::addColumn<QString>("mml_pr");
	QTest::addColumn<QString>("expression");
	
	QTest::newRow("square per square") <<
	"<math><mrow>"
		"<msup>"
			"<mi>x</mi>"
			"<mn>2</mn>"
		"</msup>"
		"<mo>*</mo>"
		"<msup>"
			"<mi>y</mi>"
			"<mn>2</mn>"
		"</msup>"
	"</mrow></math>" << "x^2*y^2";
	
	QTest::newRow("division") << "<math mode='display' xmlns='http://www.w3.org/1998/Math/MathML'>"
	"<mrow>"
		"<mfrac>"
			"<mrow>"
				"<mi>x</mi>"
				"<mo>+</mo>"
				"<msup>"
					"<mi>y</mi>"
					"<mn>2</mn>"
				"</msup>"
			"</mrow>"
			"<mrow>"
				"<mi>k</mi>"
				"<mo>+</mo>"
				"<mn>1</mn>"
			"</mrow>"
		"</mfrac>"
	"</mrow>"
	"</math>" << "(x+y^2)/(k+1)";
	
	QTest::newRow("powers") <<
	"<math mode='display' xmlns='http://www.w3.org/1998/Math/MathML'>"
		"<mrow>"
			"<msup>"
				"<mn>2</mn>"
				"<msup>"
					"<mn>2</mn>"
					"<msup>"
						"<mn>2</mn>"
						"<mi>x</mi>"
					"</msup>"
				"</msup>"
			"</msup>"
		"</mrow>"
	"</math>" << "2^(2^(2^x))";
	
	QTest::newRow("x+y^smth") <<
	"<math mode='display' xmlns='http://www.w3.org/1998/Math/MathML'>"
		"<mrow>"
			"<mi>x</mi>"
			"<mo>+</mo>"
			"<msup>"
				"<mi>y</mi>"
				"<mfrac>"
					"<mn>2</mn>"
					"<mrow>"
						"<mi>k</mi>"
						"<mo>+</mo>"
						"<mn>1</mn>"
					"</mrow>"
				"</mfrac>"
		"</msup>"
	"</mrow>"
	"</math>" << "x+y^(2/(k+1))";
	
	QTest::newRow("x+y^smth") <<
	"<math mode='display' xmlns='http://www.w3.org/1998/Math/MathML'>"
		"<mrow>"
			"<mfrac>"
				"<mi>a</mi>"
				"<mrow>"
					"<mi>b</mi>"
					"<mo>/</mo>"
					"<mn>2</mn>"
				"</mrow>"
			"</mfrac>"
		"</mrow>"
	"</math>" << "a/(b/2)";
}

void MathMLPresentationLexerTest::testConversion()
{
	QFETCH(QString, mml_pr);
	QFETCH(QString, expression);
	
	MathMLPresentationLexer lex(mml_pr);
	ExpressionParser parser;
	bool corr=parser.parse(&lex);
	QString mathML=parser.mathML();
	
	if(!corr)
		qDebug() << "error!" << parser.error();
	QVERIFY(corr);
	QVERIFY(!mathML.isEmpty());
	
	Expression e(mathML, true);
	QVERIFY(e.isCorrect());
	QCOMPARE(e.toString(), expression);
}

#include "mathmlpresentationlexertest.moc"
