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

// #define DEBUG

// #ifdef DEBUG
// #include <QScriptEngineDebugger>
// #endif

class PluginsModel : public QStandardItemModel
{
	public:
		enum Roles { PathRole = Qt::UserRole+1, PriorityRole };
		
		explicit PluginsModel(QObject* parent = 0)
			:QStandardItemModel(parent)
		{
			KStandardDirs d;
			QStringList basedirs = d.findDirs("data", "kalgebra/plugins");
			QStringList foundPlugins;
			foreach(const QString& dir, basedirs) {
				//we list <dir>/*/*.desktop
				
				QDir d(dir);
				QStringList files = d.entryList(QStringList("*.desktop"));
				
				qDebug() << "lalala" << dir << files;
				foreach(const QString& plugin, files) {
					foundPlugins += d.absoluteFilePath(plugin);
				}
			}
			
			qDebug() << "Plugins found:" << foundPlugins;
			m_plugins = KPluginInfo::fromFiles(foundPlugins);
			setSortRole(PriorityRole);
			
			foreach(const KPluginInfo& info, m_plugins) {
// 				const KPluginInfo& info;
				QStandardItem* item = new QStandardItem(KIcon(info.icon()), info.name());
				
				QString postfix = "kalgebra/plugins/"+info.pluginName();
				QString scriptPath = KStandardDirs::locate("data", postfix);
				
				Q_ASSERT(!scriptPath.isEmpty());
				
				QVariant priority = info.property("X-KAlgebra-Priority");
				if(!priority.isValid())
					priority = 1000;
				item->setData(scriptPath, PathRole);
				item->setData(priority, PriorityRole);
				
				appendRow(item);
			}
			setSortRole(PriorityRole);
			sort(0);
		}
		
	private:
		KPluginInfo::List m_plugins;
};

KAlgebraMobile* KAlgebraMobile::s_self=0;
KAlgebraMobile* KAlgebraMobile::self() { return s_self; }


KAlgebraMobile::KAlgebraMobile(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), m_functionsModel(0), m_vars(new Analitza::Variables)
{
	Q_ASSERT(s_self==0);
	s_self=this;
	
	setWindowTitle(i18n("KAlgebra Mobile"));
	
	menuBar()->addAction(i18n("Select..."), this, SLOT(selectPlugin()));
	
#ifdef DEBUG
	menuBar()->addSeparator();
	menuBar()->addAction(KIcon("debug-run"), i18n("Debug"), this, SLOT(debug()));
#endif
	m_pluginsModel = new PluginsModel(this);
// 	m_uiconfig = new UiConfig(this);
	
	qmlRegisterType<AnalitzaWrapper>("org.kde.analitza", 1, 0, "Analitza");
	qmlRegisterType<FunctionsModel>("org.kde.analitza", 1, 0, "FunctionsModel");
	qmlRegisterType<Graph2DMobile>("org.kde.analitza", 1, 0, "Graph2D");
// 	global.setProperty("VariablesModel", varsmodel, QScriptValue::Undeletable|QScriptValue::ReadOnly);
	
	setCentralWidget(new QWidget(this));
	centralWidget()->setLayout(new QVBoxLayout(centralWidget()));
	findScripts();
}

void KAlgebraMobile::findScripts()
{
	m_pluginUI.resize(m_pluginsModel->rowCount());
	displayPlugin(0);
}

void KAlgebraMobile::selectPlugin()
{
	QPointer<QDialog> d(new QDialog);
	d->setLayout(new QVBoxLayout);
	
	QListView* combo = new QListView(d.data());
// 	combo->setViewMode(QListView::IconMode);
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
// 	qDebug() << "laalalala" << plugin;
	
	if(!m_pluginUI[plugin]) {
		QString scriptFileName = m_pluginsModel->index(plugin, 0).data(PluginsModel::PathRole).toString();
		QDeclarativeView* view = new QDeclarativeView(this);
		view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
		view->engine()->setOutputWarningsToStandardError(true);
		view->engine()->rootContext()->setContextProperty("app", this);
		view->setSource(m_pluginsModel->item(plugin)->data(PluginsModel::PathRole).toUrl());
		
		m_pluginUI[plugin] = view;
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

// void KAlgebraMobile::handleException(const QScriptValue& exception)
// {
// 	QMessageBox::critical(this, i18n("Exception Thrown"), exception.toString());
// }

FunctionsModel* KAlgebraMobile::functionsModel()
{
	if(!m_functionsModel) {
		m_functionsModel = new FunctionsModel(this);
		m_functionsModel->setResolution(1000);
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
