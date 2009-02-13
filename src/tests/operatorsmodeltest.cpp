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

#include "operatorsmodeltest.h"
#include "operatorsmodel.h"
#include "expression.h"
#include "analitza.h"
#include <qtest_kde.h>
#include <variables.h>
#include <cmath>

using namespace std;

QTEST_KDEMAIN_CORE( OperatorsModelTest )

OperatorsModelTest::OperatorsModelTest(QObject *parent)
	: QObject(parent)
{}

OperatorsModelTest::~OperatorsModelTest() {}
void OperatorsModelTest::initTestCase() {}
void OperatorsModelTest::cleanupTestCase() {}

void OperatorsModelTest::testExamples()
{
	OperatorsModel m;
	
	for(int i=0; i<m.rowCount(); i++) {
		QModelIndex idx(m.index(i, 0));
		
		QModelIndex nameIdx, descriptionIdx, sampleIdx, exampleIdx;
		nameIdx = idx.sibling(idx.row(), 0);
		descriptionIdx = idx.sibling(idx.row(), 1);
		sampleIdx = idx.sibling(idx.row(), 2);
		exampleIdx = idx.sibling(idx.row(), 3);
		
		QString name=m.data(nameIdx).toString();
		QString description=m.data(descriptionIdx).toString();
		QString sample=m.data(sampleIdx).toString();
		QString example=m.data(exampleIdx).toString();
		
// 		qDebug() << "testing: " << name;
		
		QVERIFY(!name.isEmpty());
		QVERIFY(!description.isEmpty());
		QVERIFY(!sample.isEmpty());
		QVERIFY(!example.isEmpty());
		
		Analitza a;
		a.setExpression(Expression(example, false));
		if(!a.isCorrect()) qDebug() << example << "error" << a.errors();// QVERIFY(a.isCorrect());
		a.simplify();
		if(!a.isCorrect()) qDebug() << example << "error" << a.errors();// QVERIFY(a.isCorrect());
		a.variables()->modify("x", 0.1);
		Expression e = a.calculate();
		if(!a.isCorrect()) qDebug() << example << "error" << a.errors();// QVERIFY(a.isCorrect());
		if(!e.isCorrect()) qDebug() << example << "error" << e.error(); // QVERIFY(e.isCorrect());
		QVERIFY(!a.expression()->toMathMLPresentation().isEmpty());
		
		e = a.evaluate();
		if(!a.isCorrect()) qDebug() << example << "error" << a.errors();// QVERIFY(a.isCorrect());
		if(!e.isCorrect()) qDebug() << example << "error" << e.error(); // QVERIFY(e.isCorrect());
		QVERIFY(!a.expression()->toMathMLPresentation().isEmpty());
	}
}

#include "operatorsmodeltest.moc"
