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
#include "consolehtml.h"
#include "dictionary.h"
#include "askname.h"
#include "variablesdelegate.h"
#include "viewportwidget.h"
#include "functionedit.h"
#ifdef HAVE_OPENGL
#	include <analitzagui//plotsview3d.h>
#endif

#include <analitzagui/operatorsmodel.h>
#include <analitzagui/expressionedit.h>
#include <analitzagui/variablesmodel.h>
#include <analitzagui/plotsview2d.h>
#include <analitzaplot/plotsmodel.h>
#include <analitzaplot/functiongraph.h>
#include <analitzaplot/planecurve.h>
#include <analitza/variables.h>
#include <analitza/value.h>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QDockWidget>
#include <QTableView>
#include <KAction>
#include <KHTMLView>
#include <KHelpMenu>
#include <KFileDialog>
#include <KMenu>
#include <KMenuBar>
#include <KStatusBar>
#include <KLocale>
#include <KStandardAction>
#include <KProcess>
#include <KRecentFilesAction>
#include <KApplication>
#include <kabstractfilewidget.h>
#include <analitzaplot/plotsfactory.h>

class Add2DOption : public InlineOptions
{
	public:
		Add2DOption(KAlgebra* c)
		: m_kalgebra(c)
		{}
		
		virtual QString id() const { return "add2d"; }
		virtual bool matchesExpression(const Analitza::Expression& exp, const Analitza::ExpressionType& functype) const {
			return Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim2D).canDraw();
		}

		virtual QString caption() const { return i18n("Plot 2D"); }

		virtual void triggerOption(const Analitza::Expression& exp) { m_kalgebra->add2D(exp); }
	
	private:
		KAlgebra* m_kalgebra;
};

#ifdef HAVE_OPENGL
class Add3DOption : public InlineOptions
{
	public:
		Add3DOption(KAlgebra* c)
		: m_kalgebra(c)
		{}
		
		virtual QString id() const { return "add3d"; }
		virtual bool matchesExpression(const Analitza::Expression& exp, const Analitza::ExpressionType& functype) const
		{
			return Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim3D).canDraw();
		}

		virtual QString caption() const { return i18n("Plot 3D"); }

		virtual void triggerOption(const Analitza::Expression& exp) { m_kalgebra->add3D(exp); }
	
	private:
		KAlgebra* m_kalgebra;
};
#endif

QColor randomFunctionColor() { return QColor::fromHsv(qrand()%255, 255, 255); }

KAlgebra::KAlgebra(QWidget *parent) : KMainWindow(parent)
{
	resize(900, 500);
	
	m_tabs = new QTabWidget(this);
	setCentralWidget(m_tabs);
	
	setStatusBar(new KStatusBar(this));
	setMenuBar(new KMenuBar(this));
	
	QMenu* g_menu = menuBar()->addMenu(i18n("Session"));
	g_menu->addAction(KStandardAction::openNew(this, SLOT(newInstance()), this));
	g_menu->addAction(KStandardAction::quit(this, SLOT(close()), this));
	
	m_status = new QLabel(this);
	statusBar()->insertWidget(0, m_status);
    menuBar()->addMenu("|")->setEnabled(false);
	
	///////Console
	QWidget *console = new QWidget(m_tabs);
	QVBoxLayout *c_layo = new QVBoxLayout(console);
	c_results = new ConsoleHtml(console);
	c_results->view()->setFocusPolicy(Qt::NoFocus);
	c_dock_vars = new QDockWidget(i18n("Variables"), this);
	c_dock_vars->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::RightDockWidgetArea, c_dock_vars);
	
	c_varsModel=new Analitza::VariablesModel(c_results->analitza()->variables());
	c_varsModel->setEditable(false);
	
	c_variables = new QTreeView(c_dock_vars);
	c_variables->setModel(c_varsModel);
	c_variables->setRootIsDecorated(false);
	c_variables->header()->setStretchLastSection(true);
	c_variables->setSelectionBehavior(QAbstractItemView::SelectRows);
	c_variables->setSelectionMode(QAbstractItemView::SingleSelection);
	
	c_exp = new Analitza::ExpressionEdit(console);
	c_exp->setAnalitza(c_results->analitza());
	c_exp->setExamples(QStringList() << "square:=x->x**2" << "fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }");
	c_dock_vars->setWidget(c_variables);
	
	m_tabs->addTab(console, i18n("&Console"));
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
	c_menu->addSeparator()->setText(i18n("Execution Mode"));
	QActionGroup *execGroup = new QActionGroup(c_menu);
	QAction* calc = c_menu->addAction(i18nc("@item:inmenu", "Calculate"), this, SLOT(consoleCalculate()));
	QAction* eval = c_menu->addAction(i18nc("@item:inmenu", "Evaluate"), this, SLOT(consoleEvaluate()));
	
	calc->setCheckable(true);
	eval->setCheckable(true);
	eval->setChecked(true);
	execGroup->addAction(calc);
	execGroup->addAction(eval);
	c_menu->addSeparator();
	c_menu->addAction(KStandardAction::clear(c_results, SLOT(clear()), this));
	initializeRecentScripts();
	////////////
	//////EOConsola
	
	//////2D Graph
	b_funcsModel=new Analitza::PlotsModel(this);
	
	m_graph2d = new Analitza::PlotsView2D(m_tabs);
	m_graph2d->setTicksShown(0);
	m_graph2d->setModel(b_funcsModel);
	
	b_dock_funcs = new QDockWidget(i18n("Functions"), this);
	b_tools = new QTabWidget(b_dock_funcs);
	b_tools->setTabPosition(QTabWidget::South);
	addDockWidget(Qt::RightDockWidgetArea, b_dock_funcs);
	
	b_funcs = new QTreeView(b_tools);
	b_funcs->setRootIsDecorated(false);
	b_funcs->setModel(b_funcsModel);
	b_funcs->header()->resizeSections(QHeaderView::ResizeToContents);
	b_funcs->setSelectionMode(QAbstractItemView::SingleSelection);
	m_graph2d->setSelectionModel(b_funcs->selectionModel());
	
	b_tools->addTab(b_funcs, i18n("List"));
	
	b_funced = new FunctionEdit(b_tools);
	b_funced->setVariables(new Analitza::Variables);
	connect(b_funced, SIGNAL(accept()), this, SLOT(new_func()));
	b_tools->addTab(b_funced, KIcon("list-add"), i18n("&Add"));
	
	QTableView* b_varsView=new QTableView(b_tools);
	b_varsModel=new Analitza::VariablesModel(b_funced->variables());
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
	m_tabs->addTab(m_graph2d, i18n("&2D Graph"));
	c_results->addOptionsObserver(new Add2DOption(this));
	
	connect(b_varsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(valueChanged()));
	connect(b_funcs, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(edit_func(QModelIndex)));
	connect(b_tools, SIGNAL(currentChanged(int)), this, SLOT(functools(int)));
	connect(m_graph2d, SIGNAL(status(QString)), this, SLOT(changeStatusBar(QString)));
	connect(m_graph2d, SIGNAL(viewportChanged(QRectF)), b_viewport, SLOT(setViewport(QRectF)));
	connect(b_viewport, SIGNAL(viewportChange(QRectF)), m_graph2d, SLOT(changeViewport(QRectF)));
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
	m_graph2d->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_graph2d->addActions(b_menu->actions());
	
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
	QWidget *tridim = new QWidget(m_tabs);
	QVBoxLayout *t_layo = new QVBoxLayout(tridim);
	t_exp = new Analitza::ExpressionEdit(tridim);
	t_exp->setExamples(QStringList() << "sin x+sin y" << "(x,y)->x" << "x*y");
	t_exp->setAns("x");
	t_model3d = new Analitza::PlotsModel(this);
	m_graph3d = new Analitza::PlotsView3D(tridim);
	m_graph3d->setModel(t_model3d);
	m_graph3d->setUseSimpleRotation(true);
	
	tridim->setLayout(t_layo);
	m_tabs->addTab(tridim, i18n("&3D Graph"));
	t_layo->addWidget(m_graph3d);
	t_layo->addWidget(t_exp);
	
	connect(t_exp,  SIGNAL(returnPressed()), this, SLOT(new_func3d()));
	c_results->addOptionsObserver(new Add3DOption(this));
	
	////////menu
	t_menu = menuBar()->addMenu(i18n("3D &Graph"));
	QAction* t_actions[5];
	t_menu->addAction(KStandardAction::save(this, SLOT(save3DGraph()), this));
	t_menu->addAction(KIcon("zoom-original"), i18n("&Reset View"), m_graph3d, SLOT(resetView()));
	t_menu->addSeparator();
	t_actions[2] = t_menu->addAction(i18n("Dots"), this, SLOT(set_dots()));
	t_actions[3] = t_menu->addAction(i18n("Lines"), this, SLOT(set_lines()));
	t_actions[4] = t_menu->addAction(i18n("Solid"), this, SLOT(set_solid()));
	
	QActionGroup *t_type = new QActionGroup(t_menu);
	t_type->addAction(t_actions[2]);
	t_type->addAction(t_actions[3]);
	t_type->addAction(t_actions[4]);
	
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
	addDockWidget(Qt::RightDockWidgetArea, d_dock);
	Dictionary *dic = new Dictionary(m_tabs);
	m_tabs->addTab(dic, i18n("&Dictionary"));
	
	QWidget *w=new QWidget;
	QLayout *leftLayo=new QVBoxLayout(w);
	d_filter=new KLineEdit(w);
	d_filter->setClearButtonShown(true);
	d_filter->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
	connect(d_filter, SIGNAL(textChanged(QString)), dic, SLOT(setFilter(QString)));
	connect(d_filter, SIGNAL(textChanged(QString)), this, SLOT(dictionaryFilterChanged(QString)));
	d_list = new QListView(w);
	d_list->setModel(dic->model());
	d_list->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	leftLayo->addWidget(new QLabel(i18n("Look for:"), d_dock));
	leftLayo->addWidget(d_filter);
	leftLayo->addWidget(d_list);
	d_dock->setWidget(w);
	
	connect(d_list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
		dic, SLOT(activated(QModelIndex,QModelIndex)));
	
	//EODictionary
	//Ego's reminder
	KHelpMenu* help = new KHelpMenu(this, KGlobal::mainComponent().aboutData());
	menuBar()->addMenu(help->menu());

#warning TODO: Port to PlotsModel
// 	connect(b_funcsModel, SIGNAL(functionModified(QString,Analitza::Expression)),
// 			c_results, SLOT(modifyVariable(QString,Analitza::Expression)));
// 	connect(b_funcsModel, SIGNAL(functionRemoved(QString)),
// 			c_results, SLOT(removeVariable(QString)));
	
	connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
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

void KAlgebra::add2D(const Analitza::Expression& exp)
{
	qDebug() << "adding" << exp.toString();
	
	Analitza::PlotItem* curve = Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim2D).create(randomFunctionColor(), b_funcsModel->freeId(),
																		   c_results->analitza()->variables());
	b_funcsModel->addPlot(curve);
	
	m_tabs->setCurrentIndex(1);
}

void KAlgebra::new_func()
{
	Analitza::FunctionGraph* f=b_funced->createFunction();
	
	if(b_funced->editing()) {
		QModelIndex idx = b_funcsModel->indexForName(f->name());
		b_funcsModel->updatePlot(idx.row(), f);
	} else {
		b_funcsModel->addPlot(f);
	}
	
	b_funced->setEditing(false);
	b_funced->clear();
	b_tools->setCurrentIndex(0);
	b_funcs->setCurrentIndex(b_funcsModel->indexForName(f->name()));
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
		b_funced->setColor(randomFunctionColor());
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
		
		QPointer<VarEdit> e(new VarEdit(this, false));
		QString var = c_variables->model()->data(idxName, Qt::DisplayRole).toString();
		
		e->setAnalitza(c_results->analitza());
		e->setName(var);
		
		if(e->exec() == QDialog::Accepted) {
			QString str=var+" := "+e->val().toString();
			c_results->addOperation(Analitza::Expression(str, false), str);
		}
		delete e;
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
	KUrl path = KFileDialog::getSaveUrl(KUrl(), "*.kal|"+i18n("Script (*.kal)"), this, QString(), KFileDialog::ConfirmOverwrite);
	bool loaded=false;
	if(!path.isEmpty())
		loaded=c_results->saveScript(path);
	
	if(loaded)
		c_recentScripts->addUrl(path);
}

void KAlgebra::saveLog()
{
	KUrl path = KFileDialog::getSaveUrl(KUrl(), "*.html|"+i18n("HTML File (*.html)"), this, QString(), KFileDialog::ConfirmOverwrite);
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
	Analitza::Expression exp = t_exp->expression();
	Analitza::PlotBuilder plot = Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim3D);
	if(plot.canDraw()) {
		t_model3d->clear();
		t_model3d->addPlot(plot.create(Qt::yellow, "func3d", c_results->analitza()->variables()));
	} else
		changeStatusBar(i18n("Errors: %1", plot.errors().join(i18n(", "))));
#endif
}

void KAlgebra::set_dots()
{
#ifdef HAVE_OPENGL
    m_graph3d->setPlotStyle(Analitza::Dots);
#endif
}

void KAlgebra::set_lines()
{
#ifdef HAVE_OPENGL
    m_graph3d->setPlotStyle(Analitza::Wired);
#endif
}

void KAlgebra::set_solid()
{
#ifdef HAVE_OPENGL
    m_graph3d->setPlotStyle(Analitza::Solid);
#endif
}

void KAlgebra::save3DGraph()
{
#ifdef HAVE_OPENGL
	QString path = KFileDialog::getSaveFileName(KUrl(), i18n("*.png|PNG File"), this, QString(), KFileDialog::ConfirmOverwrite);
	if(!path.isEmpty()) {
		QPixmap px = m_graph3d->renderPixmap();
		px.save(path);
	}
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
	QPointer<KFileDialog> dialog=new KFileDialog(KUrl(), i18n("*.png|Image File\n*.svg|SVG File"), this);
	dialog->setOperationMode(KFileDialog::Saving);
	dialog->setConfirmOverwrite(true);
	
	if(dialog->exec()) {
		QString filter = dialog->fileWidget()->currentFilter();
		QString filename = dialog->selectedFile();
		
		bool isSvg = filename.endsWith(".svg") || (!filename.endsWith(".png") && filter.mid(2, 3)=="svg");
		Analitza::PlotsView2D::Format f = isSvg ? Analitza::PlotsView2D::PNG : Analitza::PlotsView2D::SVG;
		m_graph2d->toImage(filename, f);
	}
	delete dialog;
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
	b_funcs->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
}

void KAlgebra::updateInformation()
{
	c_varsModel->updateInformation();
	c_variables->header()->resizeSections(QHeaderView::ResizeToContents);
}

void KAlgebra::consoleCalculate()
{
	c_results->setMode(ConsoleHtml::Calculation);
}

void KAlgebra::consoleEvaluate()
{
	c_results->setMode(ConsoleHtml::Evaluation);
}

void KAlgebra::valueChanged()
{
	//FIXME: Should only repaint the affected ones.
	if(b_funcsModel->rowCount()>0)
		m_graph2d->updateFunctions(QModelIndex(), 0, b_funcsModel->rowCount()-1);
}

void KAlgebra::varsContextMenu(const QPoint& p)
{
	QPointer<QMenu> m=new QMenu;
	m->addAction(i18n("Add variable"));
	QAction* ac=m->exec(b_dock_funcs->widget()->mapToGlobal(p));
	
	if(ac) {
		QPointer<AskName> a=new AskName(i18n("Enter a name for the new variable"), 0);
		
		if(a->exec()==QDialog::Accepted)
			b_varsModel->insertVariable(a->name(), Analitza::Expression(Analitza::Cn(0)));
		delete a;
	}
	delete m;
}

void KAlgebra::add3D(const Analitza::Expression& exp)
{
#ifdef HAVE_OPENGL
	t_model3d->clear();
	t_model3d->addPlot(Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim3D).create(Qt::yellow, "func3d_console", c_results->analitza()->variables()));
	m_tabs->setCurrentIndex(2);
#endif
}

void KAlgebra::dictionaryFilterChanged(const QString&)
{
	if(d_list->model()->rowCount()==1)
		d_list->setCurrentIndex(d_list->model()->index(0,0));
}

#include "kalgebra.moc"
