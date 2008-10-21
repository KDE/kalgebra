/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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
#include "operatorsmodel.h"
#ifdef HAVE_OPENGL
#	include "graph3d.h"
#endif

#include <QVBoxLayout>
#include <QHeaderView>
#include <QDockWidget>
#include <KAction>
#include <KHTMLView>
#include <KFileDialog>
#include <KMenu>
#include <KMenuBar>
#include <KStatusBar>
#include <KLocale>
#include <KStandardAction>

KAlgebra::KAlgebra(QWidget *p) : KMainWindow(p)
{
	resize(900, 500);
	
	QTabWidget *tabs = new QTabWidget(this);
	this->setCentralWidget(tabs);
	
	setStatusBar(new KStatusBar(this));
	setMenuBar(new KMenuBar(this));
	
	m_status = new QLabel(this);
	statusBar()->insertWidget(0, m_status);
	
	///////Consola
	QWidget *console = new QWidget(p);
	QVBoxLayout *c_layo = new QVBoxLayout(console);
	c_results = new ConsoleHtml(this);
	c_dock_vars = new QDockWidget(i18n("Variables"), this);
	c_dock_vars->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
	this->addDockWidget(Qt::RightDockWidgetArea, c_dock_vars);
	
	
	c_varsModel=new VariablesModel;
	c_varsModel->setVariables(c_results->analitza()->variables());
	
	c_variables = new QTreeView(c_dock_vars);
	c_variables->setModel(c_varsModel);
	c_variables->setRootIsDecorated(false);
	c_variables->setSelectionMode(QAbstractItemView::SingleSelection);
	c_variables->header()->resizeSections(QHeaderView::ResizeToContents);
	
	c_exp = new ExpressionEdit(console);
	c_exp->setAnalitza(c_results->analitza());
	c_dock_vars->setWidget(c_variables);
	
	tabs->addTab(console, i18n("&Console"));
	console->setLayout(c_layo);
	c_layo->addWidget(c_results->view());
	c_layo->addWidget(c_exp);
	
	connect(c_exp, SIGNAL(returnPressed()), this, SLOT(operate()));
	connect(c_results, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	connect(c_results, SIGNAL(changed()), this, SLOT(updateInformation()));
	connect(c_results, SIGNAL(changed()), c_exp, SLOT(updateCompleter()));
	connect(c_variables, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(edit_var(const QModelIndex &)));
	
	////////menu
	QMenu *c_menu = menuBar()->addMenu(i18n("C&onsole"));
	
	c_menu->addAction(KStandardAction::openNew(c_results, SLOT(clear()), this));
	c_menu->addAction(KIcon("document-open"), i18nc("@item:inmenu", "&Load Script"),
						this, SLOT(loadScript()), Qt::CTRL+Qt::Key_L);
	c_menu->addAction(KIcon("document-save"), i18nc("@item:inmenu", "&Save Script"),
						this, SLOT(saveScript()), Qt::CTRL+Qt::Key_G);
	c_menu->addAction(KIcon("document-save"), i18nc("@item:inmenu", "&Save Log"),
						this, SLOT(saveLog()), QKeySequence::Save);
	c_menu->addSeparator();
	c_menu->addAction(KStandardAction::quit(this, SLOT(close()), this));
	////////////
	//////EOConsola
	
	//////2D Graph
	b_funcsModel=new FunctionsModel(this);
	
	grafic = new Graph2D(b_funcsModel, this);
	
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
	connect(b_funced, SIGNAL(accept()), this, SLOT(new_func()));
	b_tools->addTab(b_funced, KIcon("list-add"), i18n("&Add"));
	
	b_dock_funcs->setWidget(b_tools);
	b_dock_funcs->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	tabs->addTab(grafic, i18n("&2D Graph"));
	
	connect(b_funcs, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(edit_func(const QModelIndex &)));
	connect(b_tools, SIGNAL(currentChanged(int)), this, SLOT(functools(int)));
	connect(grafic, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	
	////////menu
	QMenu *b_menu = menuBar()->addMenu(i18n("2&D Graph"));
	QAction* b_actions[6];
	b_actions[0] = b_menu->addAction(i18n("&Grid"), this, SLOT(toggleSquares()));
	b_actions[1] = b_menu->addAction(i18n("&Keep Aspect Ratio"), this, SLOT(toggleKeepAspect()));
	b_menu->addAction(KStandardAction::zoomIn(grafic, SLOT(zoomIn()), this));
	b_menu->addAction(KStandardAction::zoomOut(grafic, SLOT(zoomOut()), this));
	b_menu->addSeparator();
	b_menu->addAction(KStandardAction::save(this, SLOT(saveGraph()), this));
	b_menu->addAction(KIcon("zoom-original"), i18n("&Reset View"), grafic, SLOT(resetViewport()));
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
	//////EO2D Graph
	
	/////3DGraph
#ifdef HAVE_OPENGL
	QWidget *tridim = new QWidget(p);
	QVBoxLayout *t_layo = new QVBoxLayout(tridim);
	t_exp = new ExpressionEdit(tridim);
	grafic3d = new Graph3D(tridim);
	
	tridim->setLayout(t_layo);
	tabs->addTab(tridim, i18n("&3D Graph"));
	t_layo->addWidget(grafic3d);
	t_layo->addWidget(t_exp);
	
	connect(t_exp,  SIGNAL(returnPressed()), this, SLOT(new_func3d()));
	connect(grafic3d, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	
	////////menu
	QMenu *t_menu = menuBar()->addMenu(i18n("3D &Graph"));
	QAction* t_actions[5];
	t_actions[0] = t_menu->addAction(i18n("&Transparency"), this, SLOT(toggleTransparency()));
	t_menu->addAction(KStandardAction::save(this, SLOT(save3DGraph()), this));
	t_menu->addAction(KIcon("zoom-original"), i18n("&Reset View"), grafic3d, SLOT(resetView()));
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
	connect(d_filter, SIGNAL(textChanged(const QString&)), dic, SLOT(filterChanged(const QString&)));
	d_list = new QListView(w);
	d_list->setModel(dic->model());
	d_list->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	leftLayo->addWidget(new QLabel(i18n("Look for:"), d_dock));
	leftLayo->addWidget(d_filter);
	leftLayo->addWidget(d_list);
	d_dock->setWidget(w);
	
	connect(d_list->selectionModel(), SIGNAL(currentChanged ( const QModelIndex &, const QModelIndex & )),
		dic, SLOT(activated(const QModelIndex& , const QModelIndex& )));
	
	//EODictionary
	//Ego's reminder
	menuBar()->addMenu(helpMenu());
	
	connect(b_funcsModel, SIGNAL(functionModified(const QString &, const Expression & )),
			c_results, SLOT(modifyVariable(const QString & , const Expression & )));
	connect(b_funcsModel, SIGNAL(functionRemoved(const QString &)),
			c_results, SLOT(removeVariable(const QString &)));
	
	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
	tabChanged(0);
}

void KAlgebra::new_func()
{
	QString name = b_funced->name();
	function f(b_funced->name(), Expression(b_funced->text(), b_funced->isMathML()), b_funced->color());
	
	if(b_funced->editing()) {
		b_funcsModel->editFunction(name, f);
	} else {
		b_funcsModel->addFunction(f);
	}
	
	b_funced->setEditing(false);
	b_funced->clear();
	b_tools->setCurrentIndex(0);
	b_funcsModel->setSelected(name);
	grafic->setFocus();
}

void KAlgebra::edit_func(const QModelIndex &idx)
{
	b_tools->setTabText(1, i18n("&Editing"));
	b_tools->setCurrentIndex(1);
	b_funced->setName(b_funcsModel->data(idx.sibling(idx.row(), 0)).toString());
	b_funced->setText(b_funcsModel->data(idx.sibling(idx.row(), 1)).toString());
	b_funced->setEditing(true);
	b_funced->setFocus();
}

void KAlgebra::functools(int i)
{
	if(i==0)
		b_tools->setTabText(1, i18n("&Add"));
	else {
		b_funced->setName(QString("f").append(QString::number(b_funcsModel->rowCount()+1)));
		b_funced->setEditing(false);
		b_funced->setFocus();
	}
}

void KAlgebra::edit_var(const QModelIndex &idx)
{
	QModelIndex idxName=idx.sibling(idx.row(), 0);
	VarEdit e(this, false);
	QString var = c_variables->model()->data(idxName, Qt::DisplayRole).toString();
	
	e.setAnalitza(c_results->analitza());
	e.setVar(var);
	
	if(e.exec() == QDialog::Accepted)
		c_results->analitza()->insertVariable(var, e.val());
}

void KAlgebra::operate()
{
	if(!c_exp->text().isEmpty()){
		c_exp->setCorrect(c_results->addOperation(c_exp->text(), c_exp->isMathML()));
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
	QString path = KFileDialog::getOpenFileName(KUrl(), "*.kal|"+i18n("Script (*.kal)"), this, i18n("Choose a script"));
	if(!path.isEmpty())
		c_results->loadScript(path);
}

void KAlgebra::saveScript()
{
	QString path = KFileDialog::getSaveFileName(KUrl(), "*.kal|"+i18n("Script (*.kal)"), this);
	if(!path.isEmpty())
		c_results->saveScript(path);
}

void KAlgebra::saveLog()
{
	QString path = KFileDialog::getSaveFileName(KUrl(), "*|"+i18n("Text File (*)"),  this);
	if(!path.isEmpty())
		c_results->saveLog(path);
}

void KAlgebra::set_res_low()	{ grafic->setResolution(416); }
void KAlgebra::set_res_std()	{ grafic->setResolution(832); }
void KAlgebra::set_res_fine()	{ grafic->setResolution(1664);}
void KAlgebra::set_res_vfine()	{ grafic->setResolution(3328);}

void KAlgebra::new_func3d()
{
#ifdef HAVE_OPENGL
	grafic3d->setFunc(Expression(t_exp->text(), t_exp->isMathML()));
	grafic3d->setFocus();
#endif
}

void KAlgebra::set_dots()
{
#ifdef HAVE_OPENGL
	grafic3d->setMethod(Graph3D::Dots);
#endif
}

void KAlgebra::set_lines()
{
#ifdef HAVE_OPENGL
	grafic3d->setMethod(Graph3D::Lines);
#endif
}

void KAlgebra::set_solid()
{
#ifdef HAVE_OPENGL
	grafic3d->setMethod(Graph3D::Solid);
#endif
}

void KAlgebra::save3DGraph()
{
#ifdef HAVE_OPENGL
	QString path = KFileDialog::getSaveFileName(KUrl(), i18n("*.png|PNG File"), this);
	if(!path.isEmpty())
		grafic3d->toPixmap().save(path, "PNG");
#endif
}

void KAlgebra::toggleTransparency()
{
#ifdef HAVE_OPENGL
	grafic3d->setTransparency(!grafic3d->transparency());
#endif
}

void KAlgebra::toggleSquares()
{
	grafic->setSquares(!grafic->squares());
}

void KAlgebra::toggleKeepAspect()
{
	grafic->setKeepAspectRatio(!grafic->keepAspectRatio());
}

void KAlgebra::saveGraph()
{
	QString path = KFileDialog::getSaveFileName(KUrl(), i18n("*.png|Image File\n*.svg|SVG File"), this);
	if(!path.isEmpty())
		grafic->toImage(path);
}

void KAlgebra::tabChanged(int n)
{
	c_dock_vars->hide();
	b_dock_funcs->hide();
	d_dock->hide();
	switch(n) {
		case 0:
			c_dock_vars->show();
			c_dock_vars->raise();
			c_exp->setFocus();
			break;
		case 1:
			b_dock_funcs->show();
			b_dock_funcs->raise();
			
			if(b_funcsModel->rowCount()==0)
				b_tools->setCurrentIndex(1); //We set the Add tab
// 			b_add->setFocus();
			break;
#ifdef HAVE_OPENGL
		case 2:
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

#include "kalgebra.moc"
