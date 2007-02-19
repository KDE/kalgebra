#ifndef CONSOLE_H
#define CONSOLE_H

//Depends on Qt 4.x

#include <QWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QApplication>
#include <QScrollBar>

#include "expressionedit.h"
#include "analitza.h"

/** @author Aleix Pol Gonzalez */

class Console : public QListWidget
{

Q_OBJECT
public:
	Console(QWidget *parent = 0);
	~Console();
	Analitza* analitza() { return &a; }
	
private:
	int outs;
	Analitza a;
	
	void sendStatus(const QString& msg) { emit status(msg); }

public slots:
	bool addOperation(const QString& op, bool mathml);
	bool addOperation(const QString& op) { return addOperation(op, Expression::isMathML(op)); }
	bool loadScript(const QString& path);
	bool saveLog(const QString& path);
signals:
	void status(const QString &msg);
	void changed();
};

class VariableView : public QTreeWidget
{
Q_OBJECT
public:
	VariableView(QWidget *parent=0);
	~VariableView();
	Analitza* analitza() const { return a; }
	void setAnalitza(Analitza *na) { a=na; updateVariables(); }
	
private:
	Analitza *a;
	
public slots:
	void updateVariables();
};

#endif
