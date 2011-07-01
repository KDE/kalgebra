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

#include <QMainWindow>

namespace Analitza { class Variables;}

class VariablesModel;
class PluginsModel;
class FunctionsModel;
class AnalitzaWrapper;

class KAlgebraMobile : public QMainWindow
{
	Q_OBJECT
	public:
		explicit KAlgebraMobile(QWidget* parent = 0, Qt::WindowFlags flags = 0);
		
		void displayPlugin(int plugin);
		
		static KAlgebraMobile* self();
	
	public slots:
		FunctionsModel* functionsModel();
		Analitza::Variables* variables() const;
		QStringList addFunction(const QString& expression, const QString& name = QString(), const QColor& color = QColor(), double up = 0., double down = 0.);
		
	private slots:
		void selectPlugin();
		
	private:
		static KAlgebraMobile* s_self;
		void findScripts();
		
		QVector<QWidget*> m_pluginUI;
		
		FunctionsModel* m_functionsModel;
		PluginsModel* m_pluginsModel;
		Analitza::Variables* m_vars;
};

#endif // KALGEBRAMOBILE_H
