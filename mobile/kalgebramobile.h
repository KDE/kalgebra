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
#include <QPointer>

class VariablesModel;
class PluginsModel;
class FunctionsModel;
class QScriptValue;
class AnalitzaWrapper;
class QScriptEngine;

class KAlgebraMobile : public QMainWindow
{
	Q_OBJECT
	public:
		explicit KAlgebraMobile(QWidget* parent = 0, Qt::WindowFlags flags = 0);
		
		void displayPlugin(int plugin);
		FunctionsModel* functionsModel();
		VariablesModel* variablesModel();
		QScriptEngine* engine() { return m_engine; }
	public slots:
		void selectPlugin();
		void debug();
		void handleException(const QScriptValue& exception);
		
	private:
		void findScripts();
		
		QScriptEngine* m_engine;
		QVector<QWidget*> m_pluginUI;
		
		AnalitzaWrapper* m_wrapper;
		FunctionsModel* m_functionsModel;
		PluginsModel* m_pluginsModel;
		VariablesModel* m_variablesModel;
};

#endif // KALGEBRAMOBILE_H
