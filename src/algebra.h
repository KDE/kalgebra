#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QToolBox>
#include <QTabWidget>

#include "functionedit.h"
#include "console.h"
#include "expressionedit.h"
#include "graph2d.h"
#include "graph3d.h"

class QAlgebra : public QMainWindow {
Q_OBJECT
public:
	QAlgebra(QWidget *parent);
	~QAlgebra(){}
	QString calculate(QString exp);
private:
	//console
	ExpressionEdit *c_exp;
	Console *c_results;
	VariableView *c_variables;
	int outs;
	QDockWidget *c_dock_vars;
	
	//graf 2d
	QTreeWidget *b_funcs;
	QTabWidget *b_tools;
	Graph2D *grafic;
	QDockWidget *b_dock_funcs;
	FunctionEdit *b_funced;
	
	//graph 3d
	ExpressionEdit *t_exp;
	Graph3D *grafic3d;
	
public slots:
	void operate();
	void insert(QListWidgetItem * item);
	void loadScript();
	void saveLog();
	
	void afegeix();
	void new_func();
	void edit_func(const QModelIndex &);
	void edit_var(const QModelIndex &);
	void canvi(QTreeWidgetItem * item, QTreeWidgetItem * ant);
	void toggleSquares();
	void set_res_low();
	void set_res_std();
	void set_res_fine();
	void set_res_vfine();
	void different(QTreeWidgetItem * item, int column);
	void saveGraph();
	void functools(int);
	
	void changeStatusBar(const QString &);
	void tabChanged(int);
	void set_dots();
	void set_lines();
	void set_solid();
	void toggleTransparency();
	void save3DGraph();
	
	void about();
};

#endif
