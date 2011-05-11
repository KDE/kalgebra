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

#ifndef TYPECHECKTEST_H
#define TYPECHECKTEST_H

#include <QtCore/QObject>
#include <variables.h>


class TypeCheckTest : public QObject
{
	Q_OBJECT
	public:
		TypeCheckTest(QObject* parent = 0);
		~TypeCheckTest();
		
	private slots:
		void testConstruction();
		void testConstruction_data();
		
		void testUncorrection();
		void testUncorrection_data();
		
		void testReduction();
		void testReduction_data();
	private:
		Analitza::Variables* v;
};

#endif // TYPECHECKTEST_H
