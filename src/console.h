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
