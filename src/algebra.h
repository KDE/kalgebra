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

#include <QPushButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QToolBox>
#include <QTabWidget>
#include "config-kalgebra.h"

class ExpressionEdit;
class ConsoleHtml;
class VariableView;
class FunctionEdit;
class Graph2D;
class Graph3D;

class KAlgebra : public KMainWindow
{
	Q_OBJECT
	public:
		KAlgebra ( QWidget *parent=0 );
		~KAlgebra() {}
	private:
		//console
		ExpressionEdit *c_exp;
		ConsoleHtml *c_results;
		VariableView *c_variables;
		int outs;
		QDockWidget *c_dock_vars;
		
		//graf 2d
		QTreeWidget *b_funcs;
		QTabWidget *b_tools;
		Graph2D *grafic;
		QDockWidget *b_dock_funcs;
		FunctionEdit *b_funced;

#ifdef HAVE_OPENGL
		//graph 3d
		ExpressionEdit *t_exp;
		Graph3D *grafic3d;
#endif
	public slots:
		void operate();
		void insert ( QListWidgetItem * item );
		void loadScript();
		void saveScript();
		void saveLog();
		
		void new_func();
		void edit_func ( const QModelIndex & );
		void edit_var ( const QModelIndex & );
		void change ( QTreeWidgetItem * item, QTreeWidgetItem * ant );
		void toggleSquares();
		void set_res_low();
		void set_res_std();
		void set_res_fine();
		void set_res_vfine();
		
#ifdef HAVE_OPENGL
		void new_func3d();
		
		void set_dots();
		void set_lines();
		void set_solid();
		void toggleTransparency();
		void save3DGraph();
#endif
		void different ( QTreeWidgetItem * item, int column );
		void saveGraph();
		void functools ( int );
		
		void changeStatusBar ( const QString & );
		void tabChanged ( int );
};

#endif
