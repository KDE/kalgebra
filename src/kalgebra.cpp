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
#include "askname.h"
#include "consolehtml.h"
#include "dictionary.h"
#include "functionedit.h"
#include "varedit.h"
#include "variablesdelegate.h"
#include "viewportwidget.h"
#include <analitzagui/plotsview3d_es.h>

#include <analitza/value.h>
#include <analitza/variables.h>
#include <analitzagui/expressionedit.h>
#include <analitzagui/operatorsmodel.h>
#include <analitzagui/plotsview2d.h>
#include <analitzagui/variablesmodel.h>
#include <analitzaplot/functiongraph.h>
#include <analitzaplot/planecurve.h>
#include <analitzaplot/plotsfactory.h>
#include <analitzaplot/plotsmodel.h>

#include <KConfig>
#include <KConfigGroup>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KRecentFilesAction>
#include <KStandardAction>
#include <KToggleFullScreenAction>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenuBar>
#include <QPointer>
#include <QProcess>
#include <QRandomGenerator>
#include <QStatusBar>
#include <QTableView>
#include <QToolButton>
#include <QVBoxLayout>

using namespace Qt::Literals::StringLiterals;

class Add2DOption : public InlineOptions
{
public:
    Add2DOption(KAlgebra *c)
        : m_kalgebra(c)
    {
    }

    QString id() const override
    {
        return QStringLiteral("add2d");
    }
    bool matchesExpression(const Analitza::Expression &exp) const override
    {
        return Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim2D).canDraw();
    }

    QString caption() const override
    {
        return i18n("Plot 2D");
    }

    void triggerOption(const Analitza::Expression &exp) override
    {
        m_kalgebra->add2D(exp);
    }

private:
    KAlgebra *m_kalgebra;
};

class Add3DOption : public InlineOptions
{
public:
    Add3DOption(KAlgebra *c)
        : m_kalgebra(c)
    {
    }

    QString id() const override
    {
        return QStringLiteral("add3d");
    }
    bool matchesExpression(const Analitza::Expression &exp) const override
    {
        return Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim3D).canDraw();
    }

    QString caption() const override
    {
        return i18n("Plot 3D");
    }

    void triggerOption(const Analitza::Expression &exp) override
    {
        m_kalgebra->add3D(exp);
    }

private:
    KAlgebra *m_kalgebra;
};

QColor randomFunctionColor()
{
    return QColor::fromHsv(QRandomGenerator::global()->bounded(255), 255, 255);
}

KAlgebra::KAlgebra(QWidget *parent)
    : QMainWindow(parent)
{
    resize(900, 500);

    m_tabs = new QTabWidget;
    setCentralWidget(m_tabs);

    setStatusBar(new QStatusBar(this));
    setMenuBar(new QMenuBar(this));

    KToggleFullScreenAction *fullScreenAction = KStandardAction::fullScreen(this, SLOT(fullScreen(bool)), this, this);

    QMenu *g_menu = menuBar()->addMenu(i18n("Session"));
    g_menu->addAction(KStandardAction::openNew(this, SLOT(newInstance()), this));
    g_menu->addAction(fullScreenAction);
    g_menu->addSeparator();
    g_menu->addAction(KStandardAction::quit(this, SLOT(close()), this));

    QToolButton *fullScreenButton = new QToolButton(this);
    fullScreenButton->setDefaultAction(fullScreenAction);
    m_tabs->setCornerWidget(fullScreenButton);

    m_status = new QLabel(this);
    statusBar()->insertWidget(0, m_status);
    menuBar()->addAction(QStringLiteral("|"))->setEnabled(false);

    ///////Console
    QWidget *console = new QWidget(m_tabs);
    QVBoxLayout *c_layo = new QVBoxLayout(console);
    c_results = new ConsoleHtml(console);
    c_results->setFocusPolicy(Qt::NoFocus);
    c_dock_vars = new QDockWidget(i18n("Variables"), this);
    c_dock_vars->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, c_dock_vars);

    c_varsModel = new Analitza::VariablesModel(c_results->analitza()->variables());
    c_varsModel->setEditable(false);

    c_variables = new QTreeView(c_dock_vars);
    c_variables->setModel(c_varsModel);
    c_variables->setRootIsDecorated(false);
    c_variables->header()->setStretchLastSection(true);
    c_variables->setSelectionBehavior(QAbstractItemView::SelectRows);
    c_variables->setSelectionMode(QAbstractItemView::SingleSelection);

    c_exp = new Analitza::ExpressionEdit(console);
    c_exp->setAnalitza(c_results->analitza());
    c_exp->setExamples(QStringList() << QStringLiteral("square:=x->x**2") << QStringLiteral("fib:=n->piecewise { eq(n,0)?0, eq(n,1)?1, ?fib(n-1)+fib(n-2) }"));
    c_dock_vars->setWidget(c_variables);

    m_tabs->addTab(console, i18n("&Calculator"));
    console->setLayout(c_layo);
    c_layo->addWidget(c_results);
    c_layo->addWidget(c_exp);

    connect(c_exp, &Analitza::ExpressionEdit::returnPressed, this, &KAlgebra::operate);
    connect(c_results, &ConsoleHtml::status, this, &KAlgebra::changeStatusBar);
    connect(c_results, &ConsoleHtml::changed, this, &KAlgebra::updateInformation);
    connect(c_results, SIGNAL(changed()), c_exp, SLOT(updateCompleter()));
    connect(c_results, SIGNAL(paste(QString)), c_exp, SLOT(insertText(QString)));
    connect(c_variables, &QAbstractItemView::clicked, this, &KAlgebra::edit_var);
    ////////menu
    c_menu = menuBar()->addMenu(i18n("C&alculator"));
    c_menu->addAction(QIcon::fromTheme(QStringLiteral("document-open")),
                      i18nc("@item:inmenu", "&Load Script..."),
                      Qt::CTRL | Qt::Key_L,
                      this,
                      SLOT(loadScript()));
    c_recentScripts = new KRecentFilesAction(QIcon::fromTheme(QStringLiteral("document-open-recent")), i18n("Recent Scripts"), this);
    connect(c_recentScripts, SIGNAL(urlSelected(QUrl)), this, SLOT(loadScript(QUrl)));
    c_menu->addAction(c_recentScripts);

    c_menu->addAction(QIcon::fromTheme(QStringLiteral("document-save")),
                      i18nc("@item:inmenu", "&Save Script..."),
                      Qt::CTRL | Qt::Key_G,
                      this,
                      &KAlgebra::saveScript);
    c_menu->addAction(QIcon::fromTheme(QStringLiteral("document-save")), i18nc("@item:inmenu", "&Export Log..."), QKeySequence::Save, this, &KAlgebra::saveLog);
    c_menu->addSeparator();
    c_menu->addAction(i18nc("@item:inmenu", "&Insert ans..."), Qt::Key_F3, this, &KAlgebra::insertAns);
    c_menu->addSeparator()->setText(i18n("Execution Mode"));
    QActionGroup *execGroup = new QActionGroup(c_menu);
    QAction *calc = c_menu->addAction(i18nc("@item:inmenu", "Calculate"), this, &KAlgebra::consoleCalculate);
    QAction *eval = c_menu->addAction(i18nc("@item:inmenu", "Evaluate"), this, &KAlgebra::consoleEvaluate);

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
    b_funcsModel = new Analitza::PlotsModel(this);

    m_graph2d = new Analitza::PlotsView2D(m_tabs);
    m_graph2d->setTicksShown(Qt::Orientation(0));
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
    b_funced->setVariables(c_varsModel->variables());
    connect(b_funced, &FunctionEdit::accept, this, &KAlgebra::new_func);
    connect(b_funced, &FunctionEdit::removeEditingPlot, this, &KAlgebra::remove_func);
    b_tools->addTab(b_funced, QIcon::fromTheme(QStringLiteral("list-add")), i18n("&Add"));

    QTableView *b_varsView = new QTableView(b_tools);
    b_varsModel = new Analitza::VariablesModel(b_funced->variables());
    b_varsView->setModel(b_varsModel);
    b_varsView->setShowGrid(false);
    b_varsView->verticalHeader()->hide();
    b_varsView->horizontalHeader()->setStretchLastSection(true);
    b_varsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    b_varsView->setContextMenuPolicy(Qt::CustomContextMenu);
    VariablesDelegate *delegate = new VariablesDelegate(b_varsView);
    b_varsView->setItemDelegate(delegate);
    b_tools->addTab(b_varsView, i18n("Variables"));

    ViewportWidget *b_viewport = new ViewportWidget(this);
    b_viewport->setViewport(m_graph2d->definedViewport());
    b_tools->addTab(b_viewport, i18n("Viewport"));

    b_dock_funcs->setWidget(b_tools);
    b_dock_funcs->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    m_tabs->addTab(m_graph2d, i18n("&2D Graph"));
    c_results->addOptionsObserver(new Add2DOption(this));

    connect(b_varsModel, &QAbstractItemModel::dataChanged, this, &KAlgebra::valueChanged);
    connect(b_funcs, &QAbstractItemView::doubleClicked, this, &KAlgebra::edit_func);
    connect(b_tools, &QTabWidget::currentChanged, this, &KAlgebra::functools);
    connect(m_graph2d, &Analitza::PlotsView2D::status, this, &KAlgebra::changeStatusBar);
    connect(m_graph2d, SIGNAL(viewportChanged(QRectF)), b_viewport, SLOT(setViewport(QRectF)));
    connect(b_viewport, SIGNAL(viewportChange(QRectF)), m_graph2d, SLOT(changeViewport(QRectF)));
    connect(b_varsView, &QWidget::customContextMenuRequested, this, &KAlgebra::varsContextMenu);

    ////////menu
    b_menu = menuBar()->addMenu(i18n("2&D Graph"));
    QAction *b_actions[6];
    b_actions[0] = b_menu->addAction(i18n("&Grid"), this, &KAlgebra::toggleSquares);
    b_actions[1] = b_menu->addAction(i18n("&Keep Aspect Ratio"), this, &KAlgebra::toggleKeepAspect);
    b_menu->addAction(KStandardAction::save(this, SLOT(saveGraph()), this));
    b_menu->addSeparator();
    b_menu->addAction(KStandardAction::zoomIn(m_graph2d, SLOT(zoomIn()), this));
    b_menu->addAction(KStandardAction::zoomOut(m_graph2d, SLOT(zoomOut()), this));
    QAction *ac = KStandardAction::actualSize(m_graph2d, SLOT(resetViewport()), this);
    ac->setShortcut({Qt::ControlModifier | Qt::Key_0});
    b_menu->addAction(ac);
    b_menu->addSeparator()->setText(i18n("Resolution"));
    b_actions[2] = b_menu->addAction(i18nc("@item:inmenu", "Poor"), this, &KAlgebra::set_res_low);
    b_actions[3] = b_menu->addAction(i18nc("@item:inmenu", "Normal"), this, &KAlgebra::set_res_std);
    b_actions[4] = b_menu->addAction(i18nc("@item:inmenu", "Fine"), this, &KAlgebra::set_res_fine);
    b_actions[5] = b_menu->addAction(i18nc("@item:inmenu", "Very Fine"), this, &KAlgebra::set_res_vfine);
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
    QWidget *tridim = new QWidget(m_tabs);
    QVBoxLayout *t_layo = new QVBoxLayout(tridim);
    t_exp = new Analitza::ExpressionEdit(tridim);
    t_exp->setExamples(Analitza::PlotsFactory::self()->examples(Analitza::Dim3D));
    t_exp->setAns(QStringLiteral("x"));
    t_model3d = new Analitza::PlotsModel(this);
    m_graph3d = new Analitza::PlotsView3DES(tridim);
    m_graph3d->setModel(t_model3d);
    m_graph3d->setUseSimpleRotation(true);

    tridim->setLayout(t_layo);
    m_tabs->addTab(tridim, i18n("&3D Graph"));
    t_layo->addWidget(m_graph3d);
    t_layo->addWidget(t_exp);

    connect(t_exp, &Analitza::ExpressionEdit::returnPressed, this, &KAlgebra::new_func3d);
    c_results->addOptionsObserver(new Add3DOption(this));

    ////////menu
    t_menu = menuBar()->addMenu(i18n("3D &Graph"));
    QAction *t_actions[5];
    t_menu->addAction(KStandardAction::save(this, SLOT(save3DGraph()), this));
    t_menu->addAction(QIcon::fromTheme(QStringLiteral("zoom-original")), i18n("&Reset View"), m_graph3d, [this]() {
        m_graph3d->resetViewport();
    });
    t_menu->addSeparator();
    t_actions[2] = t_menu->addAction(i18n("Dots"), this, &KAlgebra::set_dots);
    t_actions[3] = t_menu->addAction(i18n("Lines"), this, &KAlgebra::set_lines);
    t_actions[4] = t_menu->addAction(i18n("Solid"), this, &KAlgebra::set_solid);

    QActionGroup *t_type = new QActionGroup(t_menu);
    t_type->addAction(t_actions[2]);
    t_type->addAction(t_actions[3]);
    t_type->addAction(t_actions[4]);

    t_actions[2]->setCheckable(true);
    t_actions[3]->setCheckable(true);
    t_actions[4]->setCheckable(true);
    t_actions[4]->setChecked(true);

    ////////////
    //////EO3D Graph
    menuBar()->addAction(QStringLiteral("|"))->setEnabled(false);

    // Dictionary tab
    d_dock = new QDockWidget(i18n("Operations"), this);
    d_dock->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, d_dock);
    Dictionary *dic = new Dictionary(m_tabs);
    m_tabs->addTab(dic, i18n("&Dictionary"));

    QWidget *w = new QWidget;
    QLayout *leftLayo = new QVBoxLayout(w);
    d_filter = new QLineEdit(w);
    d_filter->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    connect(d_filter, &QLineEdit::textChanged, dic, &Dictionary::setFilter);
    connect(d_filter, &QLineEdit::textChanged, this, &KAlgebra::dictionaryFilterChanged);
    d_list = new QListView(w);
    d_list->setModel(dic->model());
    d_list->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
    leftLayo->addWidget(new QLabel(i18n("Look for:"), d_dock));
    leftLayo->addWidget(d_filter);
    leftLayo->addWidget(d_list);
    d_dock->setWidget(w);

    connect(d_list->selectionModel(), &QItemSelectionModel::currentChanged, dic, &Dictionary::activated);

    // EODictionary
    // Ego's reminder
    KHelpMenu *help = new KHelpMenu(this);
    menuBar()->addMenu(help->menu());

#pragma message("TODO: Port to PlotsModel")
    //     connect(b_funcsModel, SIGNAL(functionModified(QString,Analitza::Expression)),
    //             c_results, SLOT(modifyVariable(QString,Analitza::Expression)));
    //     connect(b_funcsModel, SIGNAL(functionRemoved(QString)),
    //             c_results, SLOT(removeVariable(QString)));

    connect(m_tabs, &QTabWidget::currentChanged, this, &KAlgebra::tabChanged);
    tabChanged(0);
}

KAlgebra::~KAlgebra()
{
    KConfig conf(QStringLiteral("kalgebrarc"));
    KConfigGroup config(&conf, QStringLiteral("Default"));
    QStringList urls;
    const auto recentScripts = c_recentScripts->urls();
    for (const QUrl &url : recentScripts)
        urls += url.toDisplayString();

    config.writeEntry("recent", urls);
}

void KAlgebra::initializeRecentScripts()
{
    KConfig conf(QStringLiteral("kalgebrarc"));
    KConfigGroup config(&conf, QStringLiteral("Default"));

    const QStringList urls = config.readEntry("recent", QStringList());
    for (const QString &url : urls) {
        c_recentScripts->addUrl(QUrl(url));
    }
}

void KAlgebra::newInstance()
{
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
}

void KAlgebra::add2D(const Analitza::Expression &exp)
{
    qDebug() << "adding" << exp.toString();

    Analitza::PlotBuilder req = Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim2D, c_results->analitza()->variables());
    Analitza::PlotItem *curve = req.create(randomFunctionColor(), b_funcsModel->freeId());
    b_funcsModel->addPlot(curve);

    m_tabs->setCurrentIndex(1);
}

void KAlgebra::new_func()
{
    Analitza::FunctionGraph *f = b_funced->createFunction();

    if (b_funced->editing()) {
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

void KAlgebra::remove_func()
{
    Q_ASSERT(b_funced->editing());
    b_funcsModel->removeRow(b_funcs->currentIndex().row());

    b_funced->setEditing(false);
    b_funced->clear();
    b_tools->setCurrentIndex(0);
    b_funcs->setCurrentIndex(QModelIndex());
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
    if (i == 0)
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
    if (idx.column() == 0) {
        c_exp->insertText(idx.data().toString());
    } else {
        QModelIndex idxName = idx.sibling(idx.row(), 0);

        QPointer<VarEdit> e(new VarEdit(this, false));
        QString var = c_variables->model()->data(idxName, Qt::DisplayRole).toString();

        e->setAnalitza(c_results->analitza());
        e->setName(var);

        if (e->exec() == QDialog::Accepted) {
            QString str = var + u" := "_s + e->val().toString();
            c_results->addOperation(Analitza::Expression(str, false), str);
        }
        delete e;
    }
}

void KAlgebra::operate()
{
    if (!c_exp->text().isEmpty()) {
        c_exp->setCorrect(c_results->addOperation(c_exp->expression(), c_exp->toPlainText()));
        c_exp->selectAll();
    }
}

void KAlgebra::changeStatusBar(const QString &msg)
{
    //     statusBar()->showMessage(msg);
    m_status->setText(msg);
}

void KAlgebra::loadScript()
{
    QUrl path = QFileDialog::getOpenFileUrl(this, i18n("Choose a script"), QUrl(), i18n("Script (*.kal)"));

    if (!path.isEmpty())
        loadScript(path);
}

void KAlgebra::loadScript(const QUrl &path)
{
    bool loaded = c_results->loadScript(path);

    if (loaded)
        c_recentScripts->addUrl(path);
}

void KAlgebra::saveScript()
{
    QUrl path = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), i18n("Script (*.kal)"));
    bool loaded = false;
    if (!path.isEmpty())
        loaded = c_results->saveScript(path);

    if (loaded)
        c_recentScripts->addUrl(path);
}

void KAlgebra::saveLog()
{
    QUrl path = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), i18n("HTML File (*.html)"));
    if (!path.isEmpty())
        c_results->saveLog(path);
}

void KAlgebra::insertAns()
{
    c_exp->insertText(QStringLiteral("ans"));
}

void KAlgebra::set_res_low()
{
    b_funcsModel->setResolution(416);
}
void KAlgebra::set_res_std()
{
    b_funcsModel->setResolution(832);
}
void KAlgebra::set_res_fine()
{
    b_funcsModel->setResolution(1664);
}
void KAlgebra::set_res_vfine()
{
    b_funcsModel->setResolution(3328);
}

void KAlgebra::new_func3d()
{
    Analitza::Expression exp = t_exp->expression();
    Analitza::PlotBuilder plot = Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim3D, c_results->analitza()->variables());
    if (plot.canDraw()) {
        t_model3d->clear();
        t_model3d->addPlot(plot.create(Qt::yellow, QStringLiteral("func3d")));
    } else
        changeStatusBar(i18n("Errors: %1", plot.errors().join(i18n(", "))));
}

void KAlgebra::set_dots()
{
    m_graph3d->setPlotStyle(Analitza::Dots);
}

void KAlgebra::set_lines()
{
    m_graph3d->setPlotStyle(Analitza::Wired);
}

void KAlgebra::set_solid()
{
    m_graph3d->setPlotStyle(Analitza::Solid);
}

void KAlgebra::save3DGraph()
{
    QString path = QFileDialog::getSaveFileName(this, QString(), QString(), m_graph3d->filters().join(QStringLiteral(";;")));
    if (!path.isEmpty()) {
        m_graph3d->save(QUrl::fromLocalFile(path));
    }
}

void KAlgebra::toggleSquares()
{
    m_graph2d->setTicksShown(m_graph2d->ticksShown() ? (Qt::Horizontal | Qt::Vertical) : ~(Qt::Horizontal | Qt::Vertical));
}

void KAlgebra::toggleKeepAspect()
{
    m_graph2d->setKeepAspectRatio(!m_graph2d->keepAspectRatio());
}

void KAlgebra::saveGraph()
{
    QPointer<QFileDialog> dialog =
        new QFileDialog(this, i18n("Select where to put the rendered plot"), QString(), i18n("Image File (*.png);;SVG File (*.svg)"));
    dialog->setOption(QFileDialog::DontConfirmOverwrite, false);
    if (dialog->exec() && !dialog->selectedFiles().isEmpty()) {
        QString filter = dialog->selectedNameFilter();
        QString filename = dialog->selectedFiles().first();

        bool isSvg = filename.endsWith(QLatin1String(".svg")) || (!filename.endsWith(QLatin1String(".png")) && filter.mid(2, 3) == QLatin1String("svg"));
        Analitza::PlotsView2D::Format f = isSvg ? Analitza::PlotsView2D::SVG : Analitza::PlotsView2D::PNG;
        m_graph2d->toImage(filename, f);
    }
    delete dialog;
}

void menuEnabledHelper(QMenu *m, bool enabled)
{
    m->setEnabled(enabled);
    const auto actions = m->actions();
    for (QAction *action : actions) {
        action->setEnabled(enabled);
    }
}

void KAlgebra::tabChanged(int n)
{
    c_dock_vars->hide();
    b_dock_funcs->hide();
    d_dock->hide();

    menuEnabledHelper(c_menu, n == 0);
    menuEnabledHelper(b_menu, n == 1);
    menuEnabledHelper(t_menu, n == 2);

    m_status->clear();

    switch (n) {
    case 0:
        c_dock_vars->show();
        c_dock_vars->raise();
        c_exp->setFocus();
        break;
    case 1:
        b_dock_funcs->show();
        b_dock_funcs->raise();

        if (b_funcsModel->rowCount() == 0)
            b_tools->setCurrentIndex(1); // We set the Add tab
        //             b_add->setFocus();
        break;
    case 2:
        t_exp->setFocus();
        break;
    case 3:
        d_filter->setFocus();
        d_dock->show();
        d_dock->raise();
    default:
        break;
    }
    changeStatusBar(i18nc("@info:status", "Ready"));
}

void KAlgebra::select(const QModelIndex &idx)
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
    c_results->setMode(ConsoleModel::Calculation);
}

void KAlgebra::consoleEvaluate()
{
    c_results->setMode(ConsoleModel::Evaluation);
}

void KAlgebra::valueChanged()
{
    // FIXME: Should only repaint the affected ones.
    if (b_funcsModel->rowCount() > 0)
        m_graph2d->updateFunctions(QModelIndex(), 0, b_funcsModel->rowCount() - 1);
}

void KAlgebra::varsContextMenu(const QPoint &p)
{
    QPointer<QMenu> m = new QMenu;
    m->addAction(i18n("Add variable"));
    QAction *ac = m->exec(b_dock_funcs->widget()->mapToGlobal(p));

    if (ac) {
        QPointer<AskName> a = new AskName(i18n("Enter a name for the new variable"), nullptr);

        if (a->exec() == QDialog::Accepted)
            b_varsModel->insertVariable(a->name(), Analitza::Expression(Analitza::Cn(0)));
        delete a;
    }
    delete m;
}

void KAlgebra::add3D(const Analitza::Expression &exp)
{
    t_model3d->clear();
    Analitza::PlotBuilder plot = Analitza::PlotsFactory::self()->requestPlot(exp, Analitza::Dim3D, c_results->analitza()->variables());
    Q_ASSERT(plot.canDraw());
    t_model3d->addPlot(plot.create(Qt::yellow, QStringLiteral("func3d_console")));
    m_tabs->setCurrentIndex(2);
}

void KAlgebra::dictionaryFilterChanged(const QString &)
{
    if (d_list->model()->rowCount() == 1)
        d_list->setCurrentIndex(d_list->model()->index(0, 0));
}

void KAlgebra::fullScreen(bool isFull)
{
    m_tabs->setDocumentMode(isFull);
    m_tabs->setTabEnabled(3, !isFull);
    if (isFull) {
        hide();
        m_tabs->setParent(nullptr);
        setCentralWidget(nullptr);
        m_tabs->showFullScreen();
    } else {
        setCentralWidget(m_tabs);
        show();
    }
}

#include "moc_kalgebra.cpp"
