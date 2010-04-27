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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWidget>
#include <QTreeWidget>

#include <khtml_part.h>

#include "analyzer.h"

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
		Analitza::Analyzer* analitza() { return &a; }
		
		/** Sets a @p newMode console mode. */
		void setMode(ConsoleMode newMode) { m_mode = newMode; }
		
		/** Retrieves the console mode. */
		ConsoleMode mode() const { return m_mode; }
		
	public slots:
		/** Adds the operation defined by the expression @p e. */
		bool addOperation(const Analitza::Expression& e, const QString& input);
		
		/** Loads a script from @p path. */
		bool loadScript(const KUrl& path);
		
		/** Save a script yo @p path. */
		bool saveScript(const KUrl& path) const;
		
		/** Saves a log to @p path. */
		bool saveLog(const KUrl& path) const;
		
		/** Flushes the contents. */
		void clear();
		
		/** Copies the selected text to the clipboard */
		void copy() const;
		
	signals:
		/** Emits a notification that tells that the widget status. */
		void status(const QString &msg);
		
		/** Emits that something has changed. */
		void changed();
		
		/** Emits the selected code to be pasted somewhere */
		void paste(const QString& code);
		
	private slots:
		void context(const QString&, const QPoint& p);
		
		void modifyVariable(const QString& name, const Analitza::Expression& exp);
		void removeVariable(const QString& name);
		void paste();
		void scrollDown();
		
	private:
		QString retrieve(const KUrl& remoteUrl);
		
		Analitza::Analyzer a;
		void sendStatus(const QString& msg) { emit status(msg); }
		ConsoleMode m_mode;
		QList<Analitza::Expression> m_script;
		QStringList m_htmlLog;
		
		void updateView(const QString& e);
		
		QString m_css;
};

#endif
