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

#ifndef EXPRESSIONTEST_H
#define EXPRESSIONTEST_H

#include <QObject>

/**
	@author Aleix Pol
*/
class Expression;

class ExpressionTest : public QObject
{
Q_OBJECT
	public:
		ExpressionTest(QObject *parent = 0);
		~ExpressionTest();
	
	private slots:
		void initTestCase();
		
		/**
		* We switch it to mathml and then switch back to expression
		* and we check that receive the same.
		*/
		void testConversion();
		void testConversion_data();
		
		/** We check that the same tree is equal to another equal tree. */
		void testCopy();
		void testCopy_data();
		
		/** We check that some expressions are correct or not.*/
		void testCorrection();
		void testCorrection_data();
		
		void cleanupTestCase();
		
	private:
		Expression *e;
};

#endif
