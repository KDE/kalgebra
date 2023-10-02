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

#ifndef KALGEBRA_H
#define KALGEBRA_H

#include <QLabel>
#include <QListView>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <QTreeView>

namespace Analitza
{
class PlotsView2D;
class PlotsView3DES;
class PlotsModel;
class VariablesModel;
class ExpressionEdit;
}

class ConsoleHtml;
class FunctionEdit;
class KRecentFilesAction;

namespace Analitza
{
class Expression;
}

class KAlgebra : public QMainWindow
{
    Q_OBJECT
public:
    KAlgebra(QWidget *parent = nullptr);
    ~KAlgebra();

    void add2D(const Analitza::Expression &exp);
    void add3D(const Analitza::Expression &exp);

private:
    QLabel *m_status;
    QTabWidget *m_tabs;

    // consoleeWidget
    QMenu *c_menu;
    KRecentFilesAction *c_recentScripts;
    Analitza::ExpressionEdit *c_exp;
    ConsoleHtml *c_results;
    QTreeView *c_variables;
    QDockWidget *c_dock_vars;
    Analitza::VariablesModel *c_varsModel;

    // graf 2d
    QMenu *b_menu;
    Analitza::PlotsModel *b_funcsModel;
    QTreeView *b_funcs;
    QTabWidget *b_tools;
    Analitza::PlotsView2D *m_graph2d;
    QDockWidget *b_dock_funcs;
    FunctionEdit *b_funced;
    Analitza::VariablesModel *b_varsModel;

    // graph 3d
    QMenu *t_menu;
    Analitza::ExpressionEdit *t_exp;
    Analitza::PlotsView3DES *m_graph3d;
    Analitza::PlotsModel *t_model3d;

    // Dictionary
    QDockWidget *d_dock;
    QListView *d_list;
    QLineEdit *d_filter;

private Q_SLOTS:
    void newInstance();
    void fullScreen(bool isFull);

    void initializeRecentScripts();
    void operate();
    void loadScript();
    void loadScript(const QUrl &path);
    void saveScript();
    void saveLog();
    void updateInformation();
    void consoleCalculate();
    void consoleEvaluate();
    void insertAns();

    void select(const QModelIndex &idx);
    void new_func();
    void remove_func();
    void edit_func(const QModelIndex &);
    void edit_var(const QModelIndex &);
    void toggleSquares();
    void toggleKeepAspect();
    void set_res_low();
    void set_res_std();
    void set_res_fine();
    void set_res_vfine();
    void valueChanged();
    void varsContextMenu(const QPoint &);

    void new_func3d();
    void set_dots();
    void set_lines();
    void set_solid();
    void save3DGraph();

    void saveGraph();
    void functools(int);

    void dictionaryFilterChanged(const QString &filter);

    void changeStatusBar(const QString &);
    void tabChanged(int);
};

#endif
