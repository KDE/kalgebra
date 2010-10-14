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

#include "kalgebramobile.h"
#include <QFile>
#include <QScriptEngine>
#include <QLineEdit>
#include <QScriptEngineDebugger>
#include <QToolBar>
#include <KDialog>
#include <KLocalizedString>
#include <KIcon>
#include "uiconfig.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>

#define DEBUG
#include "analitzawrapper.h"
#include <QMessageBox>
#include <analitzagui/functionsmodel.h>

KAlgebraMobile::KAlgebraMobile(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), m_model(0)
{
	m_engine = new QScriptEngine(this);
	connect(m_engine, SIGNAL(signalHandlerException(QScriptValue)), SLOT(handleException(QScriptValue)));
	
	m_wrapper = new AnalitzaWrapper(m_engine, this);
	
	QToolBar* toolbar = addToolBar(i18n("Main Toolbar"));
	toolbar->addAction(KIcon("view-choose"), i18n("Select..."), this, SLOT(selectPlugin()));
	
#ifdef DEBUG
	toolbar->addSeparator();
	toolbar->addAction(KIcon("debug-run"), i18n("Debug"), this, SLOT(debug()));
#endif
	findScripts();
}

void KAlgebraMobile::findScripts()
{
	m_scripts << "/home/kde-devel/kdeedu/kalgebra/mobile/calculator.js"
			  << "/home/kde-devel/kdeedu/kalgebra/mobile/tables.js"
			  << "/home/kde-devel/kdeedu/kalgebra/mobile/plot2d.js";
	m_pluginUI.resize(m_scripts.size());
	displayPlugin(0);
}

void KAlgebraMobile::debug()
{
#ifdef DEBUG
	QScriptEngineDebugger* debugger = new QScriptEngineDebugger(this);
	debugger->attachTo(m_engine);
	QMainWindow *debugWindow = debugger->standardWindow();
	debugWindow->resize(1024, 640);
	debugWindow->show();
#endif
}

void KAlgebraMobile::selectPlugin()
{
	QDialog d;
	d.setLayout(new QVBoxLayout);
	
	QComboBox* combo = new QComboBox(&d);
	d.layout()->addWidget(combo);
	
	int i=0;
	foreach(const QString& script, m_scripts) {
		combo->addItem(script.right(script.length()-script.lastIndexOf('/')-1), QVariant::fromValue<int>(i++));
	}
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &d);
	connect(buttons, SIGNAL(accepted()), &d, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), &d, SLOT(reject()));
	d.layout()->addWidget(buttons);
	
	int ret = d.exec();
	if(ret == QDialog::Accepted) {
		int idx = combo->currentIndex();
		QVariant data = combo->itemData(idx);
		
		displayPlugin(data.toInt());
	}
}

void KAlgebraMobile::displayPlugin(int plugin)
{
	Q_ASSERT(plugin < m_pluginUI.size());
	
	if(!m_pluginUI[plugin]) {
		QString scriptFileName = m_scripts[plugin];
		QFile scriptFile(scriptFileName);
		scriptFile.open(QIODevice::ReadOnly);
		m_engine->evaluate(scriptFile.readAll(), scriptFileName);
		scriptFile.close();

		QScriptValue ctor = m_engine->evaluate("configure");
		QScriptValue scriptUi = m_engine->newQObject(new UiConfig(this), QScriptEngine::ScriptOwnership);
		QScriptValue analitza = m_engine->newQObject(m_wrapper, QScriptEngine::QtOwnership);
		QScriptValue calc = ctor.construct(QScriptValueList() << scriptUi << analitza);
		
		QWidget* ui = qobject_cast<QWidget*>(calc.toQObject());
		Q_ASSERT(ui);
		m_pluginUI[plugin] = ui;
	}
	
	setCentralWidget(m_pluginUI[plugin]);
}

void KAlgebraMobile::handleException(const QScriptValue& exception)
{
	QMessageBox::critical(this, i18n("Exception Thrown"), exception.toString());
}

FunctionsModel* KAlgebraMobile::functionsModel()
{
	if(!m_model)
		m_model = new FunctionsModel(this);
	
	return m_model;
}
