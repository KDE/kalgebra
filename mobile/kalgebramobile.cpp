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

#include "kalgebramobile.h"
#include <QFile>
#include <QScriptEngine>
#include <QLineEdit>
#include <QToolBar>
#include <KDialog>
#include <KLocalizedString>
#include <KIcon>
#include <QVBoxLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QListView>

#include <analitza/variables.h>
#include <analitzagui/functionsmodel.h>
#include "analitzawrapper.h"

#include <KPluginInfo>
#include <KStandardDirs>
#include <QDir>
#include <QDesktopServices>
#include <QMenuBar>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <qdeclarative.h>
#include "graph2dmobile.h"
#include <qdeclarativecontext.h>
#include "pluginsmodel.h"

// #define DEBUG

// #ifdef DEBUG
// #include <QScriptEngineDebugger>
// #endif

KAlgebraMobile* KAlgebraMobile::s_self=0;
KAlgebraMobile* KAlgebraMobile::self() { return s_self; }


KAlgebraMobile::KAlgebraMobile(QObject* parent)
	: QObject(parent), m_functionsModel(0), m_vars(new Analitza::Variables)
{
	Q_ASSERT(s_self==0);
	s_self=this;
	
	qmlRegisterType<PluginsModel>("org.kde.analitza", 1, 0, "PluginsModel");
	qmlRegisterType<AnalitzaWrapper>("org.kde.analitza", 1, 0, "Analitza");
	qmlRegisterType<ExpressionWrapper>("org.kde.analitza", 1, 0, "Expression");
	qmlRegisterType<FunctionsModel>("org.kde.analitza", 1, 0, "FunctionsModel");
	qmlRegisterType<Graph2DMobile>("org.kde.analitza", 1, 0, "Graph2DView");
// 	global.setProperty("VariablesModel", varsmodel, QScriptValue::Undeletable|QScriptValue::ReadOnly);
}

FunctionsModel* KAlgebraMobile::functionsModel()
{
	if(!m_functionsModel) {
		m_functionsModel = new FunctionsModel(this);
		m_functionsModel->setResolution(500);
	}
	
	return m_functionsModel;
}

Analitza::Variables* KAlgebraMobile::variables() const { return m_vars; }

QColor randomFunctionColor() { return QColor::fromHsv(qrand()%255, 255, 255); }

QStringList KAlgebraMobile::addFunction(const QString& expression, const QString& name, const QColor& color, double up, double down)
{
	FunctionsModel* model=functionsModel();
	
	Analitza::Expression e(expression, Analitza::Expression::isMathML(expression));
	QString fname = name.isEmpty() ? model->freeId() : name;
	QColor fcolor = color.isValid() ? color : randomFunctionColor();
	
	QStringList err;
	Function f(fname, e, variables(), fcolor, up,down);
	
	if(f.isCorrect()) {
		bool b = model->addFunction(f);
		Q_ASSERT(b);
	} else
		err = f.errors();
	
	qDebug() << "aaaaaaadd" << fname << expression << err;
	return err;
}
