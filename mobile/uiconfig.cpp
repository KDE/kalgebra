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

#include "uiconfig.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDialog>
#include <analitzagui/graph2d.h>
#include "kalgebramobile.h"
#include "console.h"
#include "widgetswrappers.h"
#include "functionsdialog.h"

#define WIDGET_CREATOR(klassname, args...)\
QWidget* UiConfig::new##klassname(const QString& name)\
{\
	QWidget* w = new klassname( args );\
	w->setObjectName(name);\
	return w;\
}\

WIDGET_CREATOR(Console)
WIDGET_CREATOR(QLineEdit)
WIDGET_CREATOR(VerticalLayout)
WIDGET_CREATOR(ListWidget)
WIDGET_CREATOR(QDoubleSpinBox)
WIDGET_CREATOR(QPushButton)
WIDGET_CREATOR(FunctionsDialog, m_app->functionsModel())
WIDGET_CREATOR(Graph2D, m_app->functionsModel())

UiConfig::UiConfig(KAlgebraMobile* a, QObject* parent)
	: QObject(parent), m_app(a)
{}
