/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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


#ifndef KALGEBRAPLASMOID_HEADER
#define KALGEBRAPLASMOID_HEADER

#include <Plasma/Applet>
#include "analitza.h"

class QSizeF;
class QLineEdit;
class QLabel;

class KAlgebraPlasmoid : public Plasma::Applet
{
	Q_OBJECT
	public:
		KAlgebraPlasmoid(QObject *parent, const QVariantList &args);
		~KAlgebraPlasmoid();
		
		void init();
	private slots:
		void simplify();
		void addOperation();
        void constraintsEvent(Plasma::Constraints constraints);

	private:
		QLineEdit *m_input;
		QLabel *m_output;
		Analitza a;
};

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(kalgebraplasmoid, KAlgebraPlasmoid)

#endif
