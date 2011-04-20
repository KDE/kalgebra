/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "builtintest.h"

#include <qtest_kde.h>
#include <cmath>
#include "expression.h"
#include <expressiontypechecker.h>
#include <analyzer.h>
#include <builtinmethods.h>
#include <value.h>

QTEST_KDEMAIN_CORE( BuiltInTest )

Q_DECLARE_METATYPE(int*);

using namespace Analitza;

class CFib : public Analitza::FunctionDefinition
{
	public:
		int fib(uint n) {
			switch(n) {
				case 0:
					return 0;
				case 1:
					return 1;
				default:
					return fib(n-1)+fib(n-2);
			}
		}
		
		virtual Expression operator()(const QList<Expression>& args) {
			int val = args.first().toReal().intValue();
			if(val>=0)
				return Expression(Cn(fib(val)));
			else {
				Expression err;
				err.addError("cannot calculate negative fibonacci");
				return err;
			}
		}
};

class VehicleConstructor : public Analitza::FunctionDefinition
{
	virtual Expression operator()(const QList< Expression >& args)
	{
		return Expression::constructCustomObject(QVariant(args.first().toReal().intValue()),0);
	}
};

class Tires : public Analitza::FunctionDefinition
{
	virtual Expression operator()(const QList< Expression >& args)
	{
		QVariant value = args.first().customObjectValue();
		return Expression(Cn(value.toInt()));
	}
};

class NewIntConstructor : public Analitza::FunctionDefinition
{
	static void destroy(const QVariant& v)
	{
		int* vv=v.value<int*>();
		delete vv;
	}
	
	virtual Expression operator()(const QList< Expression >& args)
	{
		int* pi=new int(args.first().toReal().intValue());
		return Expression::constructCustomObject(QVariant::fromValue<int*>(pi), destroy);
	}
};

class ReadInt : public Analitza::FunctionDefinition
{
	virtual Expression operator()(const QList< Expression >& args)
	{
		Cn n(*args.first().customObjectValue().value<int*>());
		return Expression(n);
	}
};

BuiltInTest::BuiltInTest(QObject* parent)
	: QObject(parent)
{
	//fibonacci tests
	ExpressionType fibType(ExpressionType::Lambda);
	fibType.addParameter(ExpressionType(ExpressionType::Value));
	fibType.addParameter(ExpressionType(ExpressionType::Value));
	a.builtinMethods()->insertFunction("fib", fibType, new CFib);
	
	//object tests: tires
	ExpressionType tiresType(ExpressionType::Lambda);
	tiresType.addParameter(ExpressionType("Vehicle"));
	tiresType.addParameter(ExpressionType(ExpressionType::Value));
	a.builtinMethods()->insertFunction("tires", tiresType, new Tires);
	
	//object tests: constructor
	ExpressionType vehicleType(ExpressionType::Lambda);
	vehicleType.addParameter(ExpressionType(ExpressionType::Value));
	vehicleType.addParameter(ExpressionType("Vehicle"));
	a.builtinMethods()->insertFunction("vehicle", vehicleType, new VehicleConstructor);
	
	//object tests: newint
	ExpressionType refintType(ExpressionType::Lambda);
	refintType.addParameter(ExpressionType(ExpressionType::Value));
	refintType.addParameter(ExpressionType("RefInt"));
	a.builtinMethods()->insertFunction("refint", refintType, new NewIntConstructor);
	
	//object tests: readint
	ExpressionType readintType(ExpressionType::Lambda);
	readintType.addParameter(ExpressionType("RefInt"));
	readintType.addParameter(ExpressionType(ExpressionType::Value));
	a.builtinMethods()->insertFunction("readint", readintType, new ReadInt);
}

BuiltInTest::~BuiltInTest()
{}

#define IN QStringList() <<

void BuiltInTest::testCall_data()
{
	QTest::addColumn<QStringList>("inputs");
	QTest::addColumn<QString>("output");
	
	QTest::newRow("negfib") << (IN "fib(-1)") << "err";
	QTest::newRow("zerofib") << (IN "fib(0)") << "0";
	QTest::newRow("fib") << (IN "fib(23)") << "28657";
	
	QTest::newRow("vechicle") << (IN "tires(vehicle(2))") << "2";
	QTest::newRow("varcar") << (IN "car:=vehicle(4)" << "tires(car)") << "4";
	
	QTest::newRow("ref") << (IN "sum(readint(refint(x)) : x=1..10)") << "55";
}

void BuiltInTest::testCall()
{
	QFETCH(QStringList, inputs);
	QFETCH(QString, output);
	
	Expression calc;
	foreach(const QString& input, inputs) {
		Expression ei(input);
		QVERIFY(ei.isCorrect());
		
		a.setExpression(ei);
// 		qDebug() << "peee" << a.errors() << input;
		calc = a.calculate();
	}
	if(a.isCorrect())
		QCOMPARE(calc.toString(), output);
	else
		QCOMPARE(QString("err"), output);
}
