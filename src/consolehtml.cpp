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

#include "consolehtml.h"
#include <QFile>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QTimer>
#include <QTextDocument>
#include <qevent.h>
#include <QDir>
#include <QWebFrame>

#include <KLocale>
#include <KStandardAction>
#include <KMenu>
#include <KHTMLView>
#include <KAction>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kio/job.h>

#include <analitza/variables.h>
#include <analitza/expression.h>

ConsoleHtml::ConsoleHtml(QWidget *parent)
	: QWebView(parent), m_mode(Evaluation)
{
	page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	setRenderHint(QPainter::TextAntialiasing);
	
	connect(this, SIGNAL(linkClicked(QUrl)), SLOT(openClickedUrl(QUrl)));
	
	QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

ConsoleHtml::~ConsoleHtml()
{
	qDeleteAll(m_options);
}

void ConsoleHtml::initialize()
{
	QPalette p=qApp->palette();
	
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
	m_css +="\t.keyword { color: #000064; }\n";
	m_css +="\t.func { color: #008600; }\n";
	m_css +="\t.result { padding-left: 10%; }\n";
	m_css +="\t.options { font-size: small; text-align:right }\n";
	m_css +="\t.string { color: #bb0000 }\n";
	m_css +="\tli { padding-left: 12px; padding-bottom: 4px; list-style-position: inside; }";
	m_css +="</style>\n";
}

void ConsoleHtml::openClickedUrl(const QUrl& url)
{
	QString id =url.queryItemValue("id");
	QString exp=url.queryItemValue("func");
	
	foreach(InlineOptions* opt, m_options) {
		if(opt->id() == id) {
			opt->triggerOption(Analitza::Expression(exp, false));
		}
	}
}

bool ConsoleHtml::addOperation(const Analitza::Expression& e, const QString& input)
{
	QString result, newEntry;
	Analitza::Expression res;
	
	a.setExpression(e);
	if(a.isCorrect()) {
		if(m_mode==Evaluation) {
			res=a.evaluate();
		} else {
			res=a.calculate();
		}
	}
	
	QString options;
	if(a.isCorrect()) {
		result = res.toHtml();
		
		Analitza::Analyzer lambdifier(a.variables());
		lambdifier.setExpression(res);
		Analitza::Expression lambdaexp = lambdifier.dependenciesToLambda();
		lambdifier.setExpression(lambdaexp);
		
		Analitza::ExpressionType functype = lambdifier.type();
		
		foreach(InlineOptions* opt, m_options) {
			if(opt->matchesExpression(lambdaexp, functype)) {
				KUrl url("/query");
				url.addQueryItem("id", opt->id());
				url.addQueryItem("func", lambdaexp.toString());
				
				options += i18n(" <a href='%1'>%2</a>", url.url(), opt->caption());
			}
		}
		
		if(!options.isEmpty())
			options = "<div class='options'>"+i18n("Options: %1", options)+"</div>";
		
		a.insertVariable("ans", res);
		m_script += e; //Script won't have the errors
		newEntry = QString("%1<br />=<span class='result'>%2</span>").arg(e.toHtml()).arg(result);
	} else {
		m_htmlLog += i18n("<ul class='error'>Error: <b>%1</b><li>%2</li></ul>", Qt::escape(input), a.errors().join("</li>\n<li>"));
	}
	
	updateView(newEntry, options);
	
	return a.isCorrect();
}

QString temporaryPath()
{
	QTemporaryFile temp("consolelog");
	temp.open();
	temp.close();
	temp.setAutoRemove(false);
	return QDir::tempPath()+'/'+temp.fileName();
}

QString ConsoleHtml::retrieve(const KUrl& remoteUrl)
{
	QString path=temporaryPath();
	
	KIO::CopyJob* job=KIO::copyAs(remoteUrl, KUrl(path));
	
	bool ret = KIO::NetAccess::synchronousRun(job, 0);
	if(!ret)
		path.clear();
	
	return path;
}

bool ConsoleHtml::loadScript(const KUrl& path)
{
	Q_ASSERT(!path.isEmpty());
	
	//FIXME: We have expression-only script support
	bool correct=false;
	QFile file(path.isLocalFile() ? path.toLocalFile() : retrieve(path));
	
	if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream stream(&file);
		
		a.importScript(&stream);
		correct=a.isCorrect();
	}
	
	if(!correct) {
		m_htmlLog += i18n("<ul class='error'>Error: Could not load %1. <br /> %2</ul>", path.pathOrUrl(), a.errors().join("<br/>"));
		updateView(QString(), QString());
	}
	else
		updateView(i18n("Imported: %1", path.pathOrUrl()), QString());
	
	return correct;
}

bool ConsoleHtml::saveScript(const KUrl & path) const
{
	bool correct=false;
	Q_ASSERT(!path.isEmpty());
		
	QString savePath=path.isLocalFile() ?  path.toLocalFile() : temporaryPath();
	QFile file(savePath);
	correct=file.open(QIODevice::WriteOnly | QIODevice::Text);
	
	if(correct) {
		QTextStream out(&file);
		foreach(const Analitza::Expression& exp, m_script)
			out << exp.toString() << endl;
	}
	
	if(!path.isLocalFile()) {
		KIO::CopyJob* job=KIO::move(savePath, path);
		correct=KIO::NetAccess::synchronousRun(job, 0);
	}
	return correct;
}

bool ConsoleHtml::saveLog(const KUrl& path) const
{
	Q_ASSERT(!path.isEmpty());
	//FIXME: We have to choose between txt and html
	bool correct=false;
	QString savePath=path.isLocalFile() ?  path.toLocalFile() : temporaryPath();
	QFile file(savePath);
	correct=file.open(QIODevice::WriteOnly | QIODevice::Text);
	
	if(correct) {
		QTextStream out(&file);
		out << "<html>\n<head>" << m_css << "</head>" << endl;
		out << "<body>" << endl;
		foreach(const QString &entry, m_htmlLog)
			out << "<p>" << entry << "</p>" << endl;
		out << "</body>\n</html>" << endl;
	}
	
	if(!path.isLocalFile()) {
		KIO::CopyJob* job=KIO::move(savePath, path);
		correct=KIO::NetAccess::synchronousRun(job, 0);
	}
	return correct;
}

void ConsoleHtml::updateView(const QString& newEntry, const QString& options)
{
	QByteArray code;
	code += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
	code += "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n<head>\n\t<title> :) </title>\n";
	code += m_css;
	code += "</head>\n<body>";
	foreach(const QString &entry, m_htmlLog)
		code += "<p class='normal'>"+entry+"</p>";

	if(!newEntry.isEmpty()) {
		m_htmlLog += newEntry;
		code += options;
		code += "<p class='last'>"+newEntry+"</p>";
	}
	code += "</body></html>";

	setContent(code);
	
	emit changed();

	QWebFrame* mf = page()->mainFrame();
	mf->setScrollBarValue(Qt::Vertical, mf->scrollBarMaximum(Qt::Vertical));
}

void ConsoleHtml::copy() const
{
	QApplication::clipboard()->setText(selectedText());
}

void ConsoleHtml::contextMenuEvent(QContextMenuEvent* ev)
{
	KMenu popup;
	if(hasSelection()) {
		popup.addAction(KStandardAction::copy(this, SLOT(copy()), &popup));
		KAction *act=new KAction(KIcon("edit-paste"), i18n("Paste \"%1\" to input", selectedText()), &popup);
		connect(act, SIGNAL(triggered()), SLOT(paste()));
		popup.addAction(act);
		popup.addSeparator();
	}
	popup.addAction(KStandardAction::clear(this, SLOT(clear()), &popup));
	
	popup.exec(ev->pos());
}

void ConsoleHtml::clear()
{
	m_script.clear();
	m_htmlLog.clear();
	updateView(QString(), QString());
}

void ConsoleHtml::modifyVariable(const QString& name, const Analitza::Expression& exp)
{
	a.variables()->modify(name, exp);
}

void ConsoleHtml::removeVariable(const QString & name)
{
	a.variables()->remove(name);
}

void ConsoleHtml::paste()
{
	emit paste(selectedText());
}

#include "consolehtml.moc"
