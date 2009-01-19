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

#ifndef ANALITZATEST_H
#define ANALITZATEST_H

#include <QObject>

/**
	@author 
*/
class Analitza;

class AnalitzaTest : public QObject
{
Q_OBJECT
	public:
		AnalitzaTest(QObject *parent = 0);
		~AnalitzaTest();
	
	private slots:
		void initTestCase();
		void testTrivialEvaluate_data();
		void testTrivialEvaluate();
		void testTrivialCalculate_data();
		void testTrivialCalculate();
		void testDerivativeSimple_data();
		void testDerivativeSimple();
		
		void testVector_data();
		void testVector();
		
		void testVectorEvaluate_data();
		void testVectorEvaluate();
		
		void testCrash_data();
		void testCrash();
		
		///We check that it is correct
		void testCorrection_data();
		void testCorrection();
		
		void testEvaluate_data();
		void testEvaluate();
		
		///We check that it is not correct
		void testUncorrection_data();
		void testUncorrection();
		
		void testCompareTrees_data();
		void testCompareTrees();
		
		void testOperators();
		
// 		void testJumps_data();
// 		void testJumps();
		void cleanupTestCase();
	private:
		Analitza *a;
};

#endif
