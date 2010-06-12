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

#include "kalgebra.h"
#include "varedit.h"
#include "functionedit.h"
#include "consolehtml.h"
#include "expressionedit.h"
#include "graph2d.h"
#include "variables.h"
#include "variablesmodel.h"
#include "dictionary.h"
#include "functionsmodel.h"
#include "functionsview.h"
#include "askname.h"
#include "value.h"
#include "variablesdelegate.h"
#include "viewportwidget.h"
#ifdef HAVE_OPENGL
#	include "graph3d.h"
#endif

#include <analitzagui/operatorsmodel.h>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QItemEditorFactory>
#include <QDockWidget>
#include <QTableView>
#include <KAction>
#include <KHTMLView>
#include <KFileDialog>
#include <KMenu>
#include <KMenuBar>
#include <KStatusBar>
#include <KLocale>
#include <KStandardAction>
#include <KProcess>
#include <KRecentFilesAction>
#include <KApplication>

KAlgebra::KAlgebra(QWidget *parent) : KMainWindow(parent)
{
	resize(900, 500);
	
	QTabWidget *tabs = new QTabWidget(this);
	this->setCentralWidget(tabs);
	
	setStatusBar(new KStatusBar(this));
	setMenuBar(new KMenuBar(this));
	
	QMenu* g_menu = menuBar()->addMenu(i18n("Session"));
	g_menu->addAction(KStandardAction::openNew(this, SLOT(newInstance()), this));
	g_menu->addAction(KStandardAction::quit(this, SLOT(close()), this));
	
	m_status = new QLabel(this);
	statusBar()->insertWidget(0, m_status);
    menuBar()->addMenu("|")->setEnabled(false);
	
	///////Console
	QWidget *console = new QWidget(tabs);
	QVBoxLayout *c_layo = new QVBoxLayout(console);
	c_results = new ConsoleHtml(console);
	c_results->view()->setFocusPolicy(Qt::NoFocus);
	c_dock_vars = new QDockWidget(i18n("Variables"), this);
	c_dock_vars->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
	this->addDockWidget(Qt::RightDockWidgetArea, c_dock_vars);
	
	c_varsModel=new VariablesModel(c_results->analitza()->variables());
	c_varsModel->setEditable(false);
	
	c_variables = new QTreeView(c_dock_vars);
	c_variables->setModel(c_varsModel);
	c_variables->setRootIsDecorated(false);
	c_variables->header()->setStretchLastSection(true);
	c_variables->setSelectionBehavior(QAbstractItemView::SelectRows);
	c_variables->setSelectionMode(QAbstractItemView::SingleSelection);
	
	c_exp = new ExpressionEdit(console);
	c_exp->setAnalitza(c_results->analitza());
	c_exp->setExamples(QStringList() << "square:=x->x**2" << "fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }");
	c_dock_vars->setWidget(c_variables);
	
	tabs->addTab(console, i18n("&Console"));
	console->setLayout(c_layo);
	c_layo->addWidget(c_results->view());
	c_layo->addWidget(c_exp);
	
	connect(c_exp, SIGNAL(returnPressed()), this, SLOT(operate()));
	connect(c_results, SIGNAL(status(QString)), this, SLOT(changeStatusBar(QString)));
	connect(c_results, SIGNAL(changed()), this, SLOT(updateInformation()));
	connect(c_results, SIGNAL(changed()), c_exp, SLOT(updateCompleter()));
	connect(c_results, SIGNAL(paste(QString)), c_exp, SLOT(insertText(QString)));
	connect(c_variables, SIGNAL(clicked(QModelIndex)), this, SLOT(edit_var(QModelIndex)));
	
	////////menu
	c_menu = menuBar()->addMenu(i18n("C&onsole"));
	
	c_menu->addAction(KIcon("document-open"), i18nc("@item:inmenu", "&Load Script..."),
						this, SLOT(loadScript()), Qt::CTRL+Qt::Key_L);
	c_recentScripts=new KRecentFilesAction(KIcon("document-open-recent"), i18n("Recent Scripts"), this);
	connect(c_recentScripts, SIGNAL(urlSelected(KUrl)), this, SLOT(loadScript(KUrl)));
	c_menu->addAction(c_recentScripts);
	
	c_menu->addAction(KIcon("document-save"), i18nc("@item:inmenu", "&Save Script..."),
						this, SLOT(saveScript()), Qt::CTRL+Qt::Key_G);
	c_menu->addAction(KIcon("document-save"), i18nc("@item:inmenu", "&Export Log..."),
						this, SLOT(saveLog()), QKeySequence::Save);
	c_menu->addSeparator();
	c_menu->addAction(KStandardAction::clear(c_results, SLOT(clear()), this));
	initializeRecentScripts();
	////////////
	//////EOConsola
	
	//////2D Graph
	b_funcsModel=new FunctionsModel(this);
	
	m_graph2d = new Graph2D(b_funcsModel, tabs);
	
	b_dock_funcs = new QDockWidget(i18n("Functions"), this);
	b_tools = new QTabWidget(b_dock_funcs);
	b_tools->setTabPosition(QTabWidget::South);
	this->addDockWidget(Qt::RightDockWidgetArea, b_dock_funcs);
	
	b_funcs = new FunctionsView(b_tools);
	b_funcs->setModel(b_funcsModel);
	b_funcs->header()->resizeSections(QHeaderView::ResizeToContents);
	b_funcs->setSelectionMode(QAbstractItemView::SingleSelection);
	b_funcs->setRootIsDecorated(false);
	b_funcs->setSortingEnabled(false);
	
	b_tools->addTab(b_funcs, i18n("List"));
	
	b_funced = new FunctionEdit(b_tools);
	b_funced->setVariables(new Analitza::Variables);
	connect(b_funced, SIGNAL(accept()), this, SLOT(new_func()));
	b_tools->addTab(b_funced, KIcon("list-add"), i18n("&Add"));
	
	QTableView* b_varsView=new QTableView(b_tools);
	b_varsModel=new VariablesModel(b_funced->variables());
	b_varsView->setModel(b_varsModel);
	b_varsView->setShowGrid(false);
	b_varsView->verticalHeader()->hide();
	b_varsView->horizontalHeader()->setStretchLastSection(true);
	b_varsView->setSelectionBehavior(QAbstractItemView::SelectRows);
	b_varsView->setContextMenuPolicy(Qt::CustomContextMenu);
	VariablesDelegate* delegate=new VariablesDelegate(b_varsView);
	b_varsView->setItemDelegate(delegate);
	b_tools->addTab(b_varsView, i18n("Variables"));
	
	ViewportWidget* b_viewport = new ViewportWidget(this);
	b_viewport->setViewport(m_graph2d->definedViewport());
	b_tools->addTab(b_viewport, i18n("Viewport"));
	
	b_dock_funcs->setWidget(b_tools);
	b_dock_funcs->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	tabs->addTab(m_graph2d, i18n("&2D Graph"));
	
	connect(b_varsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(valueChanged()));
	connect(b_funcs, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(edit_func(const QModelIndex &)));
	connect(b_tools, SIGNAL(currentChanged(int)), this, SLOT(functools(int)));
	connect(m_graph2d, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	connect(m_graph2d, SIGNAL(viewportChanged(QRectF)), b_viewport, SLOT(setViewport(QRectF)));
	connect(b_viewport, SIGNAL(viewportChange(QRectF)), m_graph2d, SLOT(setViewport(QRectF)));
	connect(b_varsView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(varsContextMenu(QPoint)));
	
	////////menu
	b_menu = menuBar()->addMenu(i18n("2&D Graph"));
	QAction* b_actions[6];
	b_actions[0] = b_menu->addAction(i18n("&Grid"), this, SLOT(toggleSquares()));
	b_actions[1] = b_menu->addAction(i18n("&Keep Aspect Ratio"), this, SLOT(toggleKeepAspect()));
	b_menu->addAction(KStandardAction::save(this, SLOT(saveGraph()), this));
	b_menu->addSeparator();
	b_menu->addAction(KStandardAction::zoomIn(m_graph2d, SLOT(zoomIn()), this));
	b_menu->addAction(KStandardAction::zoomOut(m_graph2d, SLOT(zoomOut()), this));
	KAction* ac=KStandardAction::actualSize(m_graph2d, SLOT(resetViewport()), this);
	ac->setShortcut(Qt::ControlModifier + Qt::Key_0);
	b_menu->addAction(ac);
	b_menu->addSeparator()->setText(i18n("Resolution"));
	b_actions[2] = b_menu->addAction(i18nc("@item:inmenu", "Poor"), this, SLOT(set_res_low()));
	b_actions[3] = b_menu->addAction(i18nc("@item:inmenu", "Normal"), this, SLOT(set_res_std()));
	b_actions[4] = b_menu->addAction(i18nc("@item:inmenu", "Fine"), this, SLOT(set_res_fine()));
	b_actions[5] = b_menu->addAction(i18nc("@item:inmenu", "Very Fine"), this, SLOT(set_res_vfine()));
	
	QActionGroup *res = new QActionGroup(b_menu);
	res->addAction(b_actions[2]);
	res->addAction(b_actions[3]);
	res->addAction(b_actions[4]);
	res->addAction(b_actions[5]);
	
	b_actions[0]->setCheckable(true);
	b_actions[0]->setChecked(true);
	b_actions[1]->setCheckable(true);
	b_actions[1]->setChecked(true);
	b_actions[2]->setCheckable(true);
	b_actions[3]->setCheckable(true);
	b_actions[3]->setChecked(true);
	b_actions[4]->setCheckable(true);
	b_actions[5]->setCheckable(true);
	set_res_std();
	//////EO2D Graph
	
	/////3DGraph
#ifdef HAVE_OPENGL
	QWidget *tridim = new QWidget(tabs);
	QVBoxLayout *t_layo = new QVBoxLayout(tridim);
	t_exp = new ExpressionEdit(tridim);
	t_exp->setExamples(QStringList() << "sin x+sin y" << "(x,y)->x" << "x*y");
	t_exp->setAns("x");
	m_graph3d = new Graph3D(tridim);
	
	tridim->setLayout(t_layo);
	tabs->addTab(tridim, i18n("&3D Graph"));
	t_layo->addWidget(m_graph3d);
	t_layo->addWidget(t_exp);
	
	connect(t_exp,  SIGNAL(returnPressed()), this, SLOT(new_func3d()));
	connect(m_graph3d, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	
	////////menu
	t_menu = menuBar()->addMenu(i18n("3D &Graph"));
	QAction* t_actions[5];
	t_actions[0] = t_menu->addAction(i18n("&Transparency"), this, SLOT(toggleTransparency()));
	t_menu->addAction(KStandardAction::save(this, SLOT(save3DGraph()), this));
	t_menu->addAction(KIcon("zoom-original"), i18n("&Reset View"), m_graph3d, SLOT(resetView()));
	t_menu->addSeparator()->setText(i18n("Type"));
	t_actions[2] = t_menu->addAction(i18n("Dots"), this, SLOT(set_dots()));
	t_actions[3] = t_menu->addAction(i18n("Lines"), this, SLOT(set_lines()));
	t_actions[4] = t_menu->addAction(i18n("Solid"), this, SLOT(set_solid()));
	
	QActionGroup *t_type = new QActionGroup(t_menu);
	t_type->addAction(t_actions[2]);
	t_type->addAction(t_actions[3]);
	t_type->addAction(t_actions[4]);
	
	t_actions[0]->setCheckable(true);
	t_actions[2]->setCheckable(true);
	t_actions[3]->setCheckable(true);
	t_actions[4]->setCheckable(true);
	t_actions[4]->setChecked(true);
#endif
	////////////
	//////EO3D Graph
    menuBar()->addMenu("|")->setEnabled(false);
	
	//Dictionary tab
	d_dock = new QDockWidget(i18n("Operations"), this);
	d_dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	this->addDockWidget(Qt::RightDockWidgetArea, d_dock);
	Dictionary *dic = new Dictionary(tabs);
	tabs->addTab(dic, i18n("&Dictionary"));
	
	QWidget *w=new QWidget;
	QLayout *leftLayo=new QVBoxLayout(w);
	d_filter=new KLineEdit(w);
	d_filter->setClearButtonShown(true);
	d_filter->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
	connect(d_filter, SIGNAL(textChanged(const QString&)), dic, SLOT(setFilter(const QString&)));
	connect(d_filter, SIGNAL(textChanged(const QString&)), this, SLOT(dictionaryFilterChanged(const QString&)));
	d_list = new QListView(w);
	d_list->setModel(dic->model());
	d_list->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	leftLayo->addWidget(new QLabel(i18n("Look for:"), d_dock));
	leftLayo->addWidget(d_filter);
	leftLayo->addWidget(d_list);
	d_dock->setWidget(w);
	
	connect(d_list->selectionModel(), SIGNAL(currentChanged (QModelIndex, QModelIndex)),
		dic, SLOT(activated(QModelIndex , QModelIndex)));
	
	//EODictionary
	//Ego's reminder
	menuBar()->addMenu(helpMenu());
	
	connect(b_funcsModel, SIGNAL(functionModified(QString, Expression)),
			c_results, SLOT(modifyVariable(QString, Expression)));
	connect(b_funcsModel, SIGNAL(functionRemoved(QString)),
			c_results, SLOT(removeVariable(QString)));
	
	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
	tabChanged(0);
}

KAlgebra::~KAlgebra()
{
	KConfig conf("kalgebrarc");
	KConfigGroup config(&conf, "Default");
	QStringList urls;
	foreach(const KUrl& url, c_recentScripts->urls())
		urls += url.prettyUrl();
	
	config.writeEntry("recent", urls);
}

void KAlgebra::initializeRecentScripts()
{
	KConfig conf("kalgebrarc");
	KConfigGroup config(&conf, "Default");
	
	QStringList urls=config.readEntry("recent", QStringList());
	foreach(const QString& url, urls) {
		c_recentScripts->addUrl(KUrl(url));
	}
}

void KAlgebra::newInstance()
{
	KProcess::startDetached(QApplication::applicationFilePath());
}

void KAlgebra::new_func()
{
	function f=b_funced->createFunction();
	
	if(b_funced->editing()) {
		b_funcsModel->editFunction(f.name(), f);
	} else {
		b_funcsModel->addFunction(f);
	}
	
	b_funced->setEditing(false);
	b_funced->clear();
	b_tools->setCurrentIndex(0);
	b_funcsModel->setSelected(f.name());
	m_graph2d->setFocus();
}

void KAlgebra::edit_func(const QModelIndex &idx)
{
	b_tools->setTabText(1, i18n("&Editing"));
	b_tools->setCurrentIndex(1);
	b_funced->setName(b_funcsModel->data(idx.sibling(idx.row(), 0)).toString());
	b_funced->setFunction(b_funcsModel->data(idx.sibling(idx.row(), 1)).toString());
	b_funced->setEditing(true);
	b_funced->setFocus();
}

void KAlgebra::functools(int i)
{
	if(i==0)
		b_tools->setTabText(1, i18n("&Add"));
	else {
		b_funced->setName(b_funcsModel->freeId());
		b_funced->setEditing(false);
		b_funced->setFocus();
	}
}

void KAlgebra::edit_var(const QModelIndex &idx)
{
	if(idx.column()==0) {
		c_exp->insertText(idx.data().toString());
	} else {
		QModelIndex idxName=idx.sibling(idx.row(), 0);
		VarEdit e(this, false);
		QString var = c_variables->model()->data(idxName, Qt::DisplayRole).toString();
		
		e.setAnalitza(c_results->analitza());
		e.setName(var);
		
		if(e.exec() == QDialog::Accepted) {
			QString str=var+" := "+e.val().toString();
			c_results->addOperation(Analitza::Expression(str, false), str);
		}
	}
}

void KAlgebra::operate()
{
	if(!c_exp->text().isEmpty()){
		c_exp->setCorrect(c_results->addOperation(c_exp->expression(), c_exp->toPlainText()));
		c_exp->selectAll();
	}
}

void KAlgebra::changeStatusBar(const QString& msg)
{
// 	statusBar()->showMessage(msg);
	m_status->setText(msg);
}

void KAlgebra::loadScript()
{
	KUrl path = KFileDialog::getOpenUrl(KUrl(), "*.kal|"+i18n("Script (*.kal)"), this, i18n("Choose a script"));
	
	if(!path.isEmpty())
		loadScript(path);
}

void KAlgebra::loadScript(const KUrl& path)
{
	bool loaded=c_results->loadScript(path);
	
	if(loaded)
		c_recentScripts->addUrl(path);
}

void KAlgebra::saveScript()
{
	KUrl path = KFileDialog::getSaveUrl(KUrl(), "*.kal|"+i18n("Script (*.kal)"), this);
	bool loaded=false;
	if(!path.isEmpty())
		loaded=c_results->saveScript(path);
	
	if(loaded)
		c_recentScripts->addUrl(path);
}

void KAlgebra::saveLog()
{
	KUrl path = KFileDialog::getSaveUrl(KUrl(), "*.html|"+i18n("HTML File (*.html)"),  this);
	if(!path.isEmpty())
		c_results->saveLog(path);
}

void KAlgebra::set_res_low()	{ b_funcsModel->setResolution(416); }
void KAlgebra::set_res_std()	{ b_funcsModel->setResolution(832); }
void KAlgebra::set_res_fine()	{ b_funcsModel->setResolution(1664);}
void KAlgebra::set_res_vfine()	{ b_funcsModel->setResolution(3328);}

void KAlgebra::new_func3d()
{
#ifdef HAVE_OPENGL
	m_graph3d->setFunc(t_exp->expression());
	m_graph3d->setFocus();
#endif
}

void KAlgebra::set_dots()
{
#ifdef HAVE_OPENGL
	m_graph3d->setMethod(Graph3D::Dots);
#endif
}

void KAlgebra::set_lines()
{
#ifdef HAVE_OPENGL
	m_graph3d->setMethod(Graph3D::Lines);
#endif
}

void KAlgebra::set_solid()
{
#ifdef HAVE_OPENGL
	m_graph3d->setMethod(Graph3D::Solid);
#endif
}

void KAlgebra::save3DGraph()
{
#ifdef HAVE_OPENGL
	QString path = KFileDialog::getSaveFileName(KUrl(), i18n("*.png|PNG File"), this);
	if(!path.isEmpty())
		m_graph3d->toPixmap().save(path, "PNG");
#endif
}

void KAlgebra::toggleTransparency()
{
#ifdef HAVE_OPENGL
	m_graph3d->setTransparency(!m_graph3d->transparency());
#endif
}

void KAlgebra::toggleSquares()
{
	m_graph2d->setSquares(!m_graph2d->squares());
}

void KAlgebra::toggleKeepAspect()
{
	m_graph2d->setKeepAspectRatio(!m_graph2d->keepAspectRatio());
}

void KAlgebra::saveGraph()
{
	QString path = KFileDialog::getSaveFileName(KUrl(), i18n("*.png|Image File\n*.svg|SVG File"), this);
	if(!path.isEmpty())
		m_graph2d->toImage(path);
}

void KAlgebra::tabChanged(int n)
{
	c_dock_vars->hide();
	b_dock_funcs->hide();
	d_dock->hide();
	
	c_menu->setEnabled(false);
	b_menu->setEnabled(false);
#ifdef HAVE_OPENGL
	t_menu->setEnabled(false);
#endif
	m_status->clear();
	
	switch(n) {
		case 0:
			c_menu->setEnabled(true);
			c_dock_vars->show();
			c_dock_vars->raise();
			c_exp->setFocus();
			break;
		case 1:
			b_menu->setEnabled(true);
			b_dock_funcs->show();
			b_dock_funcs->raise();
			
			if(b_funcsModel->rowCount()==0)
				b_tools->setCurrentIndex(1); //We set the Add tab
// 			b_add->setFocus();
			break;
#ifdef HAVE_OPENGL
		case 2:
			t_menu->setEnabled(true);
			t_exp->setFocus();
			break;
#endif
		case 3:
			d_filter->setFocus();
			d_dock->show();
			d_dock->raise();
		default:
			break;
	}
	changeStatusBar(i18nc("@info:status", "Ready"));
}

void KAlgebra::select(const QModelIndex & idx)
{
	b_funcsModel->setSelected(idx);
}

void KAlgebra::updateInformation()
{
	c_varsModel->updateInformation();
	c_variables->header()->resizeSections(QHeaderView::ResizeToContents);
}

void KAlgebra::valueChanged()
{
	//FIXME: Should only repaint the affected ones.
	if(b_funcsModel->rowCount()>0)
		m_graph2d->update(b_funcsModel->index(0,0), b_funcsModel->index(b_funcsModel->rowCount()-1,0));
}

void KAlgebra::varsContextMenu(const QPoint& p)
{
	QMenu m;
	m.addAction(i18n("Add variable"));
	QAction* ac=m.exec(b_dock_funcs->widget()->mapToGlobal(p));
	
	if(ac) {
		AskName a(i18n("Enter a name for the new variable"), 0);
		
		if(a.exec()==QDialog::Accepted)
			b_varsModel->insertVariable(a.name(), Analitza::Expression(Analitza::Cn(0)));
	}
}

void KAlgebra::dictionaryFilterChanged(const QString&)
{
	if(d_list->model()->rowCount()==1)
		d_list->setCurrentIndex(d_list->model()->index(0,0));
}

#include "kalgebra.moc"
