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

#include "consolehtml.h"
#include <QFile>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>

#include <KLocale>
#include <KStandardAction>
#include <KMenu>
#include <KHTMLView>
#include <KAction>

#include "exp.h"
#include "variables.h"
#include "expression.h"

ConsoleHtml::ConsoleHtml(QWidget *parent) : KHTMLPart(parent), m_mode(Calculation)
{
	QPalette p=qApp->palette();
	
	setJScriptEnabled(false);
	setJavaEnabled(false);
	setMetaRefreshEnabled(false);
	setPluginsEnabled(false);
	setOnlyLocalReferences(true);
	
	m_css ="<style type=\"text/css\">\n";
	m_css +=QString("\thtml { background-color: %1; }\n").arg(p.color(QPalette::Active, QPalette::Base).name());
	m_css +="\t.error { border-style: solid; border-width: 1px; border-color: #ff3b21; background-color: #ffe9c4; padding:7px;}\n";
	m_css +="\t.last  { border-style: solid; border-width: 1px; border-color: #2020ff; background-color: #e0e0ff; padding:7px;}\n";
	m_css +="\t.before { text-align:right; }\n";
	m_css +="\t.op  { font-weight: bold; }\n";
// 	m_css +="\t.normal:hover  { border-style: solid; border-width: 1px; border-color: #777; }\n";
	m_css +="\t.normal:hover  { background-color: #f7f7f7; }\n";
	m_css +="\t.cont { color: #560000; }\n";
	m_css +="\t.num { color: #0000C4; }\n";
	m_css +="\t.sep { font-weight: bold; color: #0000FF; }\n";
	m_css +="\t.var { color: #640000; }\n";
	m_css +="\t.func { color: #008600; }\n";
	m_css +="\tli { padding-left: 12px; padding-bottom: 4px; list-style-position: inside; }";
	m_css +="</style>\n";
	
	begin();
	write("<html>\n<head>"+m_css+"</head></html>");
	end();
	
	connect(this, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(context(const QString &, const QPoint &)));
}

ConsoleHtml::~ConsoleHtml() {}

/*QString printable(const QString &s)
{
	QString s1(s);
	s1.replace("<", "&lt;");
	s1.replace(">", "&gt;");
	return s1;
}*/

bool ConsoleHtml::addOperation(const QString& op, bool mathml)
{
	QString result, newEntry;
	Expression e(op, mathml);
	a.setExpression(e);
	if(a.isCorrect()) {
		Expression res;
		if(m_mode==Evaluation) {
			res=a.evaluate();
		} else {
			res=a.calculate();
		}
		result = res.toHtml();
		a.insertVariable("ans", res);
	}
	
	if(a.isCorrect()) {
		m_script += op; //Script won't have the errors
		newEntry = QString("%1 <br/><span align='right'>%2</span>").arg(a.expression()->toHtml()).arg(result);
	} else
		m_htmlLog += QString("<ul class='error'>Error: %1<li>%2</li></ul>").arg(op).arg(a.errors().join("</li>\n<li>"));
	
	updateView(newEntry);
	
	return a.isCorrect();
}

bool ConsoleHtml::loadScript(const QString& path)
{
	//FIXME: We have expression-only script support
	bool correct=false;
	if(!path.isEmpty()) {
		QStringList lines;
		QFile file(path);
		QString line;
		if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream stream(&file);
			correct=true;
			while (!stream.atEnd()) {
				line += stream.readLine(); // line of text excluding '\n'
				
				Exp e(line);
				e.parse();
				
				if(!line.isEmpty() && e.isCompletelyRead()) {
					correct &= addOperation(line, Expression::isMathML(line));
					line.clear();
				}
			}
			file.close();
		}
	}
	return correct;
}

bool ConsoleHtml::saveScript(const QString & path) const
{
	bool correct=false;
	if(!path.isEmpty()) {
		QFile file(path);
		correct=file.open(QIODevice::WriteOnly | QIODevice::Text);
		
		if(correct) {
			QTextStream out(&file);
			QStringList::const_iterator it = m_script.begin();
			for(; it!=m_script.end(); it++)
				out << *it << endl;
		}
		file.close();
	}
	return correct;
}

bool ConsoleHtml::saveLog(const QString& path) const
{
	//FIXME: We have to choose between txt and html
	bool correct=false;
	if(!path.isEmpty()) {
		QFile file(path);
		correct=file.open(QIODevice::WriteOnly | QIODevice::Text);
		
		if(correct) {
			QTextStream out(&file);
			out << "<html>\n<head>" << m_css << "</head>" << endl;
			out << "<body>" << endl;
			foreach(const QString &entry, m_htmlLog)
				out << "<p>" << entry << "</p>" << endl;
			out << "</body>\n</html>" << endl;
		}
		file.close();
	}
	return correct;
}

void ConsoleHtml::updateView(const QString& newEntry)
{
	//FIXME: Check that the output html is not correct
	begin();
	write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	write("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n<head>\n\t<title> :) </title>\n");
	write(m_css);
	write("</head>\n<body>");
	write(m_css);
	foreach(const QString &entry, m_htmlLog) {
		write("<p class='normal'>"+entry+"</p>");
	}
	if(!newEntry.isEmpty()) {
		m_htmlLog += newEntry;
		write("<p class='last'>"+newEntry+"</p>");
	}
	write("</body></html>");
	end();
	
// 	qDebug() << "puto!" << m_htmlLog.join("<p />\n");
	
	qApp->processEvents();
// 	view()->verticalScrollBar()->setValue(view()->verticalScrollBar()->maximum());
	view()->scrollBy(0, 200);
	emit changed();
}

void ConsoleHtml::copy() const
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(selectedText());
}

void ConsoleHtml::context(const QString &, const QPoint & p)
{
	KMenu popup;
	if(hasSelection())
		popup.addAction(KStandardAction::copy(this, SLOT(copy()), &popup));
	popup.addAction(KStandardAction::clear(this, SLOT(clear()), &popup));
	
	popup.exec(p);
}

void ConsoleHtml::clear()
{
	m_script.clear();
	m_htmlLog.clear();
	updateView();
}

#include "consolehtml.moc"
