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

#ifndef EXPTEST_H
#define EXPTEST_H

#include <QObject>

/**
	@author Aleix Pol
*/

class ExpTest : public QObject
{
Q_OBJECT
	public:
		ExpTest(QObject *parent = 0);
		~ExpTest();
	
	private slots:
		void initTestCase();
		
		/** We check that the conversion to MathML is correct*/
		void testExp();
		void testExp_data();
		
		/** We check if expressions are correct as expected. */
		void testCorrection();
		void testCorrection_data();
		
		/** We check the token lengths*/
		void testLength();
		void testLength_data();
		
		void cleanupTestCase();
};

#endif
