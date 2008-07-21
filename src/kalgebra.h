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

class KAlgebra : public KMainWindow
{
	Q_OBJECT
	public:
		KAlgebra ( QWidget *parent=0 );
		~KAlgebra() {}
	private:
		QLabel *m_status;
		
		//consoleeWidget
		ExpressionEdit *c_exp;
		ConsoleHtml *c_results;
		QTreeView *c_variables;
		int outs;
		QDockWidget *c_dock_vars;
		VariablesModel* c_varsModel;
		
		//graf 2d
		FunctionsModel* b_funcsModel;
		QTreeView *b_funcs;
		QTabWidget *b_tools;
		Graph2D *grafic;
		QDockWidget *b_dock_funcs;
		FunctionEdit *b_funced;

#ifdef HAVE_OPENGL
		//graph 3d
		ExpressionEdit *t_exp;
		Graph3D *grafic3d;
#endif
		//Dictionary
		QDockWidget *d_dock;
		QListView *d_list;
		KLineEdit *d_filter;
		
	private slots:
		void operate();
		void loadScript();
		void saveScript();
		void saveLog();
		void updateInformation();
		
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
		
		void new_func3d();
		
		void set_dots();
		void set_lines();
		void set_solid();
		void toggleTransparency();
		void save3DGraph();
		
		void saveGraph();
		void functools ( int );
		
		void changeStatusBar ( const QString & );
		void tabChanged ( int );
};

#endif
