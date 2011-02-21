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

#include <analitzagui/variablesmodel.h>
#include <analitzagui/functionsmodel.h>
#include "analitzawrapper.h"
#include "uiconfig.h"

#include <KPluginInfo>
#include <KStandardDirs>
#include <QDir>

// #define DEBUG

#ifdef DEBUG
#include <QScriptEngineDebugger>
#endif

class PluginsModel : public QStandardItemModel
{
	public:
		enum Roles { PathRole = Qt::UserRole+1, PriorityRole };
		
		explicit PluginsModel(QObject* parent = 0)
			:QStandardItemModel(parent)
		{
			KStandardDirs d;
			QStringList basedirs = d.findDirs("data", "kalgebra/scripts");
			QStringList foundPlugins;
			foreach(const QString& dir, basedirs) {
				//we list <dir>/*/*.desktop
				
				QDir d(dir);
				QStringList dirs = d.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
				
				foreach(const QString& plugindir, dirs) {
					QDir pd(d.filePath(plugindir));
					QStringList files = pd.entryList(QStringList("*.desktop"));
					
					foreach(const QString& plugin, files) {
						foundPlugins += pd.absoluteFilePath(plugin);
					}
				}
			}
			
			qDebug() << "Plugins found:" << foundPlugins;
			m_plugins = KPluginInfo::fromFiles(foundPlugins);
			setSortRole(PriorityRole);
			
			foreach(const KPluginInfo& info, m_plugins) {
// 				const KPluginInfo& info;
				QStandardItem* item = new QStandardItem(KIcon(info.icon()), info.name());
				
				QString postfix = "kalgebra/scripts/"+info.pluginName();
				QString scriptPath = KStandardDirs::locate("data", postfix);
				
				Q_ASSERT(!scriptPath.isEmpty());
				
				QVariant priority = info.property("X-KAlgebra-Priority");
				if(!priority.isValid())
					priority = 1000;
				item->setData(scriptPath, PathRole);
				item->setData(priority, PriorityRole);
				
				appendRow(item);
			}
			sort(0);
		}
		
	private:
		KPluginInfo::List m_plugins;
};

KAlgebraMobile::KAlgebraMobile(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), m_functionsModel(0)
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
	m_pluginsModel = new PluginsModel(this);
	m_uiconfig = new UiConfig(this);
	
	QScriptValue global = m_engine->globalObject();
	QScriptValue analitza = m_engine->newQObject(m_wrapper, QScriptEngine::QtOwnership);
	QScriptValue varsmodel= m_engine->newQObject(m_wrapper->variablesModel(), QScriptEngine::QtOwnership);
	global.setProperty("Analitza", analitza, QScriptValue::Undeletable|QScriptValue::ReadOnly);
	global.setProperty("VariablesModel", varsmodel, QScriptValue::Undeletable|QScriptValue::ReadOnly);
	m_engine->setGlobalObject(global);
	
	setCentralWidget(new QWidget(this));
	centralWidget()->setLayout(new QVBoxLayout(centralWidget()));
	findScripts();
}

void KAlgebraMobile::findScripts()
{
	m_pluginUI.resize(m_pluginsModel->rowCount());
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
	QPointer<QDialog> d(new QDialog);
	d->setLayout(new QVBoxLayout);
	
	QListView* combo = new QListView(d.data());
	combo->setViewMode(QListView::IconMode);
	combo->setModel(m_pluginsModel);
	combo->setFrameStyle(QFrame::NoFrame);
	combo->setBackgroundRole(QPalette::NoRole);
	combo->setEditTriggers(0);
	connect(combo, SIGNAL(clicked(QModelIndex)), d.data(), SLOT(accept()));
	d->layout()->addWidget(combo);
	
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, d.data());
	connect(buttons, SIGNAL(accepted()), d.data(), SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), d.data(), SLOT(reject()));
	d->layout()->addWidget(buttons);
	
	int ret = d->exec();
	if(ret == QDialog::Accepted) {
		displayPlugin(combo->currentIndex().row());
	}
	delete d.data();
}

void KAlgebraMobile::displayPlugin(int plugin)
{
	Q_ASSERT(plugin < m_pluginUI.size());
	
	if(!m_pluginUI[plugin]) {
		QString scriptFileName = m_pluginsModel->index(plugin, 0).data(PluginsModel::PathRole).toString();
		QFile scriptFile(scriptFileName);
		scriptFile.open(QIODevice::ReadOnly);
		m_engine->evaluate(scriptFile.readAll(), scriptFileName);
		scriptFile.close();

		QScriptValue ctor = m_engine->evaluate("KAlgebraExtension");
		QScriptValue scriptUi = m_engine->newQObject(m_uiconfig, QScriptEngine::ScriptOwnership);
		QScriptValue calc = ctor.construct(QScriptValueList() << scriptUi);
		if(m_engine->hasUncaughtException())
			qDebug() << m_engine->uncaughtException().toString();
		
		QWidget* ui = qobject_cast<QWidget*>(calc.property("ui").toQObject());
		Q_ASSERT(ui);
		ui->setParent(this);
		m_pluginUI[plugin] = ui;
	}
	
	QLayout* layout = centralWidget()->layout();
	while(!layout->isEmpty()) {
		QLayoutItem* item = layout->takeAt(0);;
		item->widget()->hide();
		delete item;
	}
	
	layout->addWidget(m_pluginUI[plugin]);
	m_pluginUI[plugin]->show();
}

void KAlgebraMobile::handleException(const QScriptValue& exception)
{
	QMessageBox::critical(this, i18n("Exception Thrown"), exception.toString());
}

FunctionsModel* KAlgebraMobile::functionsModel()
{
	if(!m_functionsModel)
		m_functionsModel = new FunctionsModel(this);
	
	return m_functionsModel;
}

VariablesModel* KAlgebraMobile::variablesModel()
{
	return m_wrapper->variablesModel();
}
