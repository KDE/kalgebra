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

#ifndef KALGEBRAMOBILE_H
#define KALGEBRAMOBILE_H

#include <QObject>
#include <QColor>

namespace Analitza { class Variables;}

class VariablesModel;
class PluginsModel;
class FunctionsModel;
class AnalitzaWrapper;

class KAlgebraMobile : public QObject
{
	Q_OBJECT
	public:
		explicit KAlgebraMobile(QObject* parent=0);
		
		void displayPlugin(int plugin);
		
		static KAlgebraMobile* self();
	
	public slots:
		FunctionsModel* functionsModel();
		Analitza::Variables* variables() const;
		QStringList addFunction(const QString& expression, const QString& name = QString(), const QColor& color = QColor(), double up = 0., double down = 0.);
		
	private:
		static KAlgebraMobile* s_self;
		
		FunctionsModel* m_functionsModel;
		Analitza::Variables* m_vars;
};

#endif // KALGEBRAMOBILE_H
