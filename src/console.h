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

/**
 *	The Console widget is able to receive an operation, solve it and show the value.
 *	It also is able to load scripts and save logs.
 *	@author Aleix Pol Gonzalez
 */

class Console : public QListWidget
{
	Q_OBJECT
	public:
		/** Constructor. Creates a console widget. */
		Console(QWidget *parent = 0);
		
		/** Destructor. */
		~Console();
		
		/** Retrieves a pointer to the Analitza calculator associated. */
		Analitza* analitza() { return &a; }
		
	public slots:
		/** Adds an operation @p op with the format @p mathml to the log. */
		bool addOperation(const QString& op, bool mathml);
		
		/** Adds an operation @p op and tries to guess the format to the log. */
		bool addOperation(const QString& op) { return addOperation(op, Expression::isMathML(op)); }
		
		/** Loads a script from @p path. */
		bool loadScript(const QString& path);
		
		/** Saves a log to @p path. */
		bool saveLog(const QString& path);
	signals:
		/** Emits a notification that tells that the widget status. */
		void status(const QString &msg);
		
		/** Emits that something has changed. */
		void changed();
		
	private:
		int outs;
		Analitza a;
	
		void sendStatus(const QString& msg) { emit status(msg); }

};

/** The VariableView is a widget that shows all variables with their values. */
class VariableView : public QTreeWidget
{
Q_OBJECT
public:
	/** Constructor. Creates a new VariableView widget. */
	VariableView(QWidget *parent=0);
	
	/** Destructor. */
	~VariableView();
	
	/** Retrieves the used Analitza module. */
	Analitza* analitza() const { return a; }
	
	/** Sets the used Analitza module. */
	void setAnalitza(Analitza *na) { a=na; updateVariables(); }
public slots:
	/** Rechecks the Variables for the view. */
	void updateVariables();
private:
	Analitza *a;
};

#endif
