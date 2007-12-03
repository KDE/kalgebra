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

#include "algebra.h"
#include "varedit.h"
#include "functionedit.h"
#include "consolehtml.h"
#include "expressionedit.h"
#include "graph2d.h"
#ifdef HAVE_OPENGL
#	include "graph3d.h"
#endif
#include "variables.h"
#include "dictionary.h"

#include <QVBoxLayout>
#include <QLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QDockWidget>
#include <QMessageBox>

#include <KFileDialog>
#include <KMenu>
#include <KMenuBar>
#include <KStatusBar>
#include <KAction>
#include <KLocale>
#include <KHTMLView>
#include <KStandardAction>

KAlgebra::KAlgebra(QWidget *p) : KMainWindow(p)
{
	resize(900, 500);
	
	QTabWidget *tabs = new QTabWidget(this);
	this->setCentralWidget(tabs);
	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
	
	setStatusBar(new KStatusBar(this));
	setMenuBar(new KMenuBar(this));
	
	///////Consola
	QWidget *console = new QWidget(p);
	QVBoxLayout *c_layo = new QVBoxLayout(console);
	c_results = new ConsoleHtml(this);
	c_dock_vars = new QDockWidget(i18n("Variables"), this);
	c_dock_vars->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
	this->addDockWidget(Qt::RightDockWidgetArea, c_dock_vars);
	
	c_variables = new VariableView(c_dock_vars);
	c_exp = new ExpressionEdit(console);
	c_exp->setAnalitza(c_results->analitza());
	c_variables->setAnalitza(c_results->analitza());
	c_dock_vars->setWidget(c_variables);
	
	tabs->addTab(console, i18n("&Console"));
	console->setLayout(c_layo);
	c_layo->addWidget(c_results->view());
	c_layo->addWidget(c_exp);
	
	connect(c_exp, SIGNAL(returnPressed()), this, SLOT(operate()));
// 	connect(c_results, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(insert(QListWidgetItem *)));
	connect(c_results, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	connect(c_results, SIGNAL(changed()), c_variables, SLOT(updateVariables()));
	connect(c_results, SIGNAL(changed()), c_exp, SLOT(updateCompleter()));
	connect(c_variables, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(edit_var(const QModelIndex &)));
	
	////////menu
	QMenu *c_menu = menuBar()->addMenu(i18n("C&onsole"));
	
	c_menu->addAction(KStandardAction::openNew(c_results, SLOT(clear()), this));
	c_menu->addAction(i18nc("@item:inmenu", "&Load Script"), this, SLOT(loadScript()), Qt::CTRL+Qt::Key_L);
	c_menu->addAction(i18nc("@item:inmenu", "&Save Script"), this, SLOT(saveScript()), Qt::CTRL+Qt::Key_G);
	c_menu->addAction(i18nc("@item:inmenu", "&Save Log"), this, SLOT(saveLog()), QKeySequence::Save);
	c_menu->addSeparator();
	c_menu->addAction(KStandardAction::quit(this, SLOT(close()), this));
	////////////
	//////EOConsola
	
	//////2D Graph
	grafic = new Graph2D(this);
	
	b_dock_funcs = new QDockWidget(i18n("Functions"), this);
	b_tools = new QTabWidget(b_dock_funcs);
	b_tools->setTabPosition(QTabWidget::South);
	
	this->addDockWidget(Qt::RightDockWidgetArea, b_dock_funcs);
	
	b_funcs = new QTreeWidget(b_tools);
	b_funcs->headerItem()->setText(0, i18nc("@title:column", "Name"));
	b_funcs->headerItem()->setText(1, i18nc("@title:column", "Function"));
	b_funcs->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	b_funcs->setSelectionMode(QAbstractItemView::SingleSelection);
	b_funcs->setRootIsDecorated(false);
	b_funcs->setSortingEnabled(false);
	
	b_funcs->clear();
	b_tools->addTab(b_funcs, i18n("List"));
	
	b_funced = new FunctionEdit(b_tools);
	connect(b_funced, SIGNAL(accept()), this, SLOT(new_func()));
	b_tools->addTab(b_funced, i18n("&Add"));
	
	b_dock_funcs->setWidget(b_tools);
	b_dock_funcs->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	tabs->addTab(grafic, i18n("&2D Graph"));
	
	connect(b_funcs, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(edit_func(const QModelIndex &)));
	connect(b_funcs, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(change(QTreeWidgetItem*, QTreeWidgetItem*)));
	
	connect(b_funcs, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(different(QTreeWidgetItem *, int)));
	connect(b_tools, SIGNAL(currentChanged(int)), this, SLOT(functools(int)));
	connect(grafic, SIGNAL(status(const QString &)), this, SLOT(changeStatusBar(const QString &)));
	
	////////menu
	QMenu *b_menu = menuBar()->addMenu(i18n("2&D Graph"));
	QAction* b_actions[6];
	b_actions[0] = b_menu->addAction(i18n("&Grid"), this, SLOT(toggleSquares()));
	b_menu->addAction(KStandardAction::zoomIn(grafic, SLOT(zoomIn()), this));
	b_menu->addAction(KStandardAction::zoomOut(grafic, SLOT(zoomOut()), this));
	b_menu->addSeparator();
	b_menu->addAction(KStandardAction::save(this, SLOT(saveGraph()), this));
	b_menu->addAction(i18n("&Reset View"), grafic, SLOT(resetViewport()));
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
	t_menu->addAction(i18n("&Reset View"), grafic3d, SLOT(resetView()));
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
	Dictionary *dic = new Dictionary(tabs);
	tabs->addTab(dic, i18n("&Dictionary"));
	//EODictionary
	//Ego's reminder
	menuBar()->addMenu(helpMenu());
	
	tabChanged(0);
}

void KAlgebra::new_func()
{
	QTreeWidgetItem *item;
	QString name;
	if(!b_funced->editing()) {
		name = b_funced->name();
		grafic->addFunction(function(b_funced->name(), Expression(b_funced->text(), b_funced->isMathML()), b_funced->color(), true));
		item = new QTreeWidgetItem(b_funcs);
		item->setFlags(Qt::ItemIsSelectable| Qt::ItemIsUserCheckable| Qt::ItemIsEnabled| Qt::ItemIsTristate);
	} else {
		item = b_funcs->currentItem();
		grafic->editFunction(item->text(0),
                                     function(item->text(0), Expression(b_funced->text(), b_funced->isMathML()), b_funced->color(), true));
		name = item->text(0);
	}
	QPixmap ico(15, 15);
	ico.fill(b_funced->color());

	item->setIcon(0, ico);
	item->setText(0, name);
	item->setText(1, b_funced->text());
	item->setTextColor(1, b_funced->color());
	item->setCheckState(0, Qt::Checked);
	b_funced->setEditing(false);
	b_funced->clear();
	b_tools->setCurrentIndex(0);
	grafic->setFocus();
	grafic->setSelected(name);
}

void KAlgebra::edit_func(const QModelIndex &)
{
	b_tools->setTabText(1, i18n("&Editing"));
	b_tools->setCurrentIndex(1);
	b_funced->setText(b_funcs->currentItem()->text(1));
	b_funced->setEditing(true);
	b_funced->setFocus();
}

void KAlgebra::functools(int i)
{
	if(i==0)
		b_tools->setTabText(1, i18n("&Add"));
	else {
		b_funced->setName(QString("f").append(QString::number(b_funcs->topLevelItemCount()+1)));
		b_funced->setEditing(false);
		b_funced->setFocus();
	}
}

void KAlgebra::different(QTreeWidgetItem * item, int)
{
	grafic->setShown(item->text(0), item->checkState(0) == Qt::Checked);
}

void KAlgebra::edit_var(const QModelIndex &)
{
	VarEdit e(this, false);
	QString varname = c_variables->currentItem()->text(0);
	QString var(varname);
	e.setAnalitza(c_results->analitza());
	e.setVar(var);
	
	if(e.exec() == QDialog::Accepted)
		c_results->analitza()->m_vars->modify(varname, e.val());
	
	c_variables->updateVariables();
}

void KAlgebra::operate()
{
	if(!c_exp->text().isEmpty()){
		c_exp->setCorrect(c_results->addOperation(c_exp->text(), c_exp->isMathML()));
		c_exp->selectAll();
	}
}

void KAlgebra::insert(QListWidgetItem * item)
{
	c_exp->insertPlainText(item->toolTip());
	c_exp->setFocus();
}

void KAlgebra::changeStatusBar(const QString& msg)
{
	statusBar()->showMessage(msg);
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

void KAlgebra::saveGraph()
{
	QString path = KFileDialog::getSaveFileName(KUrl(), i18n("*.png|Image File\n*.svg|SVG File"), this);
	if(!path.isEmpty())
		grafic->toImage(path);
}

void KAlgebra::change(QTreeWidgetItem *current, QTreeWidgetItem *)
{
	grafic->setSelected(current->text(0));
}

void KAlgebra::tabChanged(int n)
{
	c_dock_vars->hide();
	b_dock_funcs->hide();
	switch(n) {
		case 0:
			c_dock_vars->show();
			c_dock_vars->raise();
			c_exp->setFocus();
			break;
		case 1:
			b_dock_funcs->show();
			b_dock_funcs->raise();
			
			if(b_funcs->topLevelItemCount()==0)
				b_tools->setCurrentIndex(1); //We set the Add tab
// 			b_add->setFocus();
			break;
#ifdef HAVE_OPENGL
		case 2:
			t_exp->setFocus();
			break;
#endif
		default:
			break;
	}
	changeStatusBar(i18nc("@info:status", "Ready"));
}

#include "algebra.moc"
