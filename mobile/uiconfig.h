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

#ifndef UICONFIG_H
#define UICONFIG_H

#include <QtGui/QWidget>
#include <QScriptValue>

class QAbstractItemModel;
class VariablesModel;
class KAlgebraMobile;
namespace Analitza {
class Analyzer;
}

class UiConfig : public QObject
{
	Q_OBJECT
	public:
		explicit UiConfig(KAlgebraMobile* a, QObject* parent = 0);
		
	public slots:
		QWidget* newVerticalLayout(const QString& name=QString());
		QWidget* newQLineEdit(const QString& name);
		QWidget* newConsole(const QString& name);
		QWidget* newListWidget(const QString& name);
		QWidget* newQDoubleSpinBox(const QString& name);
		QWidget* newTreeView(const QString& name);
		QWidget* newQPushButton(const QString& name);
		QWidget* newFunctionsDialog(const QString& name);
		QWidget* newGraph2D(const QString& name);
		
	private:
		KAlgebraMobile* m_app;
};

#endif // UICONFIG_H
