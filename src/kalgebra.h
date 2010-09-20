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

#include <KMainWindow>
#include <KLineEdit>
#include <QTreeView>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QListView>
#include <config-kalgebra.h>

class ExpressionEdit;
class ConsoleHtml;
class VariablesModel;
class FunctionEdit;
class Graph2D;
class Graph3D;
class FunctionsModel;
class KRecentFilesAction;

namespace Analitza { class Expression; }

class KAlgebra : public KMainWindow
{
	Q_OBJECT
	public:
		KAlgebra ( QWidget *parent=0 );
		~KAlgebra();
		
		void add2D(const Analitza::Expression& exp);
		void add3D(const Analitza::Expression& exp);
	private:
		QLabel *m_status;
		QTabWidget* m_tabs;
		
		//consoleeWidget
		QMenu* c_menu;
		KRecentFilesAction* c_recentScripts;
		ExpressionEdit *c_exp;
		ConsoleHtml *c_results;
		QTreeView *c_variables;
		QDockWidget *c_dock_vars;
		VariablesModel* c_varsModel;
		
		//graf 2d
		QMenu* b_menu;
		FunctionsModel* b_funcsModel;
		QTreeView *b_funcs;
		QTabWidget *b_tools;
		Graph2D *m_graph2d;
		QDockWidget *b_dock_funcs;
		FunctionEdit *b_funced;
		VariablesModel* b_varsModel;

#ifdef HAVE_OPENGL
		//graph 3d
		QMenu* t_menu;
		ExpressionEdit *t_exp;
		Graph3D *m_graph3d;
#endif
		//Dictionary
		QDockWidget *d_dock;
		QListView *d_list;
		KLineEdit *d_filter;
		
	private slots:
		void newInstance();
		
		void initializeRecentScripts();
		void operate();
		void loadScript();
		void loadScript(const KUrl& path);
		void saveScript();
		void saveLog();
		void updateInformation();
		void consoleCalculate();
		void consoleEvaluate();
		
		void select(const QModelIndex& idx);
		void new_func();
		void edit_func ( const QModelIndex & );
		void edit_var ( const QModelIndex & );
		void toggleSquares();
		void toggleKeepAspect();
		void set_res_low();
		void set_res_std();
		void set_res_fine();
		void set_res_vfine();
		void valueChanged();
		void varsContextMenu(const QPoint&);
		
		void new_func3d();
		void set_dots();
		void set_lines();
		void set_solid();
		void toggleTransparency();
		void save3DGraph();
		
		void saveGraph();
		void functools ( int );
		
		void dictionaryFilterChanged(const QString& filter);
		
		void changeStatusBar ( const QString & );
		void tabChanged ( int );
};

#endif
