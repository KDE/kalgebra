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

#include <QWidget>
#include <QListWidget>
#include <QTreeWidget>
#include <QScrollBar>

#include <khtml_part.h>

#include "analitza.h"

/**
 *	The Console widget is able to receive an operation, solve it and show the value.
 *	It also is able to load scripts and save logs.
 *	@author Aleix Pol Gonzalez
 */

class ConsoleHtml : public KHTMLPart
{
	Q_OBJECT
	public:
		/** This enumeration controles the way the console will calculate and show his results. */
		enum ConsoleMode {
			Evaluation, /**< Simplifies the expression, tries to simplify when sees a variable not defined. */
			Calculation /**< Calculates everything, if it finds a not defined variable shows an error. */
		};
		
		/** Constructor. Creates a console widget. */
		ConsoleHtml(QWidget *parent = 0);
		
		/** Destructor. */
		virtual ~ConsoleHtml();
		
		/** Retrieves a pointer to the Analitza calculator associated. */
		Analitza* analitza() { return &a; }
		
		/** Sets a @p newMode console mode. */
		void setMode(ConsoleMode newMode) { m_mode = newMode; }
		
		/** Retrieves the console mode. */
		ConsoleMode mode() const { return m_mode; }
		
	public slots:
		/** Adds an operation @p op with the format @p mathml to the log. */
		bool addOperation(const QString& op, bool mathml);
		
		/** Loads a script from @p path. */
		bool loadScript(const QString& path);
		
		/** Save a script yo @p path. */
		bool saveScript(const QString& path) const;
		
		/** Saves a log to @p path. */
		bool saveLog(const QString& path) const;
		
		/** Flushes the contents. */
		void clear();
	signals:
		/** Emits a notification that tells that the widget status. */
		void status(const QString &msg);
		
		/** Emits that something has changed. */
		void changed();
	private:
		Analitza a;
		void sendStatus(const QString& msg) { emit status(msg); }
		ConsoleMode m_mode;
		QStringList m_script;
		QStringList m_htmlLog;
		
		bool addEvaluation(Expression&);
		bool addCalculation(Expression&);
		
		void updateView(const QString& e=QString());
		
		QString m_css;
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
