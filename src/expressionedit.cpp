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

#include "expressionedit.h"

#include <QLabel>
#include <QKeyEvent>
#include <QTreeView>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMenu>

#include <KLocale>

#include "operatorsmodel.h"
#include "operator.h"
#include "exp.h"
#include "analitza.h"
#include "variables.h"
#include "container.h"

ExpressionEdit::ExpressionEdit(QWidget *parent, AlgebraHighlighter::Mode inimode)
	: QTextEdit(parent), m_histPos(0), help(true), m_auto(true), a(0), m_correct(true), m_ans("ans")
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setFixedHeight(QFontMetrics(this->currentFont()).height()+12);
	this->setTabChangesFocus(true);
	this->setAcceptRichText(false);
	this->setAutoFormatting(AutoNone);
	m_history.append("");
	
	m_helptip = new QLabel(this, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool | Qt::X11BypassWindowManagerHint);
	m_helptip->setFrameShape(QFrame::Box);
	m_helptip->setFocusPolicy(Qt::NoFocus);
	m_helptip->setAutoFillBackground(false);
	m_helptip->hide();
	QPalette palette;
	palette.setColor(m_helptip->backgroundRole(), QColor(255,230,255));
	m_helptip->setPalette(palette);
	
	m_highlight= new AlgebraHighlighter(this->document(), a);
	
	m_completer = new QCompleter(this);
	m_completer->setWidget(this);
	m_completer->setCompletionColumn(0);
	m_completer->setCompletionRole(Qt::DisplayRole);
	treeView = new QTreeView;
	m_completer->setPopup(treeView);
	treeView->setRootIsDecorated(false);
	treeView->header()->hide();
// 	treeView->resizeColumnToContents(1);
	treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	treeView->setMinimumWidth(300);
	m_ops = new OperatorsModel(m_completer);
	m_completer->setModel(m_ops);
	
	updateCompleter();
	
	treeView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	treeView->showColumn(0);
	treeView->showColumn(1);
	treeView->hideColumn(2);
	treeView->hideColumn(3);
	
	connect(this, SIGNAL(returnPressed()), this, SLOT(returnP()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMov()));
	connect(this, SIGNAL(signalHelper(const QString&)), this, SLOT(ajudant(const QString&)));
	connect(m_completer, SIGNAL(activated(const QString&)), this, SLOT(completed(const QString&)));
// 	connect(m_completer, SIGNAL(activated(const QModelIndex&)), this, SLOT(completed(const QModelIndex&)));
	
	setMode(inimode);
}

ExpressionEdit::~ExpressionEdit() {}

void ExpressionEdit::updateCompleter()
{
	m_ops->updateInformation();
}

void ExpressionEdit::completed(const QString& newText)
{
	int c = newText.length() - lastWord(textCursor().selectionStart()).length();
	QString toInsert=newText.right(c);
	if(Expression::whatType(newText) == Object::oper && !isMathML())
		toInsert += '(';
	insertPlainText(toInsert);
}

bool ExpressionEdit::isMathML() const
{
	switch(m_highlight->mode()) {
		case AlgebraHighlighter::MathML:
			return true;
		case AlgebraHighlighter::Expression:
			return false;
		default:
			return Expression::isMathML(this->toPlainText());
	}
}

void ExpressionEdit::setMode(AlgebraHighlighter::Mode en)
{
	if(!text().isEmpty()) {
		if(isMathML() && en==AlgebraHighlighter::Expression) { //We convert it into MathML
			Expression e(toPlainText(), true);
			this->setPlainText(e.toString());
		} else if(!isMathML() && en==AlgebraHighlighter::MathML) {
			/*Expression e(toPlainText(), false);
			this->setPlainText(e.toMathML());*/
			
			Exp e(toPlainText());
			e.parse();
			this->setPlainText(e.mathML());
		}
	}
	m_highlight->setMode(en);
	setCorrect(true);
}

void ExpressionEdit::returnP()
{
// 	removenl();
	if(!this->toPlainText().isEmpty()) {
		m_history.last() = this->toPlainText();
		m_history.append(QString());
		m_histPos=m_history.count()-1;
	}
}

void ExpressionEdit::keyPressEvent(QKeyEvent * e)
{
	bool ch=false;
	
	switch(e->key()){
		case Qt::Key_F2:
			simplify();
			break;
		case Qt::Key_F3:
			setMode(isMathML() ? AlgebraHighlighter::Expression : AlgebraHighlighter::MathML);
			break;
		case Qt::Key_Escape:
			if(m_completer->popup()->isVisible())
				m_completer->popup()->hide();
			else
				clear();
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			//FIXME: have to find a way that if the focus is not in the textedit completes, otherwise don't complete.
// 			if(m_completer->popup()->isVisible()) 
// 				completed(m_completer->currentCompletion());
			{
				bool b=returnPress();
				if(b) {
					QTextEdit::keyPressEvent(e);
				}
				m_completer->popup()->hide();
			}
			break;
		case Qt::Key_Up:
			if(!m_completer->popup()->isVisible()) {
				m_histPos--;
				ch=true;
			}
			break;
		case Qt::Key_Down:
			if(!m_completer->popup()->isVisible()) {
				m_histPos++;
				ch=true;
			}
			break;
		case Qt::Key_Plus:
		case Qt::Key_Asterisk:
		case Qt::Key_Slash:
			if(this->toPlainText().length() == (this->textCursor().position()-this->textCursor().anchor()) && m_auto) {
				this->setPlainText(m_ans);
				QTextCursor tc = this->textCursor();
				tc.setPosition(m_ans.length());
				this->setTextCursor(tc);
			}
		default:
			QTextEdit::keyPressEvent(e);
			m_history.last() = this->toPlainText();
			QString last = lastWord(textCursor().selectionStart());
			if(!last.isEmpty()) {
				QFontMetrics fm(font());
				int curPos = this->textCursor().position();
				int pixelsOffset = fm.width(toPlainText(), curPos);
				QPoint pos(pixelsOffset+10, height());
				m_completer->setCompletionPrefix(last);
				m_completer->complete();
			} else {
				m_completer->popup()->hide();
			}
			break;
	}
	
	if(ch) {
		if(m_histPos<0)
			m_histPos=0;
		if(m_histPos>=m_history.count())
			m_histPos=m_history.count()-1;
		this->setPlainText(m_history[m_histPos]);
	}
	
	if(m_completer->completionCount()==1 && m_completer->completionPrefix()==m_completer->currentCompletion()) {
		m_completer->popup()->hide();
	}
	
	int lineCount=toPlainText().count('\n')+1;
	this->setFixedHeight(QFontMetrics(this->currentFont()).height()*lineCount+12);
	setCorrect(m_highlight->isCorrect());
}

QString ExpressionEdit::lastWord(int pos)
{
	QString exp=text();
	int act=pos-1;
	for(; act>=0 && exp[act].isLetter(); act--);
	
	return exp.mid(act+1, pos-act-1);
}

QString ExpressionEdit::findPrec(const QString& exp, int &act, int cur, int &param, const QString& tit)
{
	QString paraula=tit, p;
	int nparams=0, cat=0;
	if(!tit.isNull())
		cat=1;
	
	for(; act<cur; ++act){
		if(exp.at(act).isLetter()) {
			paraula.clear();
			for(; act<exp.length() && exp[act].isLetter(); act++){
				paraula += exp[act];
			}
			
			if(act<exp.length() && exp[act]=='(' && act<cur) {//This is a function
				int param_rec=0;
				
				act++;
				p=findPrec(exp, act, cur, param_rec, paraula);
				
				if(param_rec != -1){
					param = param_rec;
//					qDebug("****out2***<%s> %s %s", p.ascii(), paraula.ascii(), tit.ascii());
					return p;
				}
			} else act--;
		} else if(exp.at(act) == ',') {
			nparams++;
		} else if(exp.at(act) == '(') {
			cat++;
		} else if(exp.at(act) == ')') {
			cat--;
//			qDebug("cat: %d", cat);
			if(cat == 0) { //FIXME: Better to put outside the if?
				param=-1; //Means this is a useless func
				return QString();
			} else if(cat <0){
				param=-3;
				return QString();
			}
		}
//		qDebug("word: %s", exp.mid(act, exp.length()-act).ascii());
	}
	param=nparams;
	
//	qDebug("####out###%s %s <%s>", p.ascii(), paraula.ascii(), tit.ascii());
	return tit;
}

QString ExpressionEdit::editingWord(int pos, int &param)
{ //simplification use only
	int p=0;
	param=0;
	return findPrec(this->toPlainText().mid(0,pos), p, pos, param, "");
}

void ExpressionEdit::cursorMov()
{
	int param=0, pos=this->textCursor().position();
	m_highlight->setPos(pos);
	if(text().isEmpty())
		setCorrect(true);
	QString s = editingWord(pos, param);
	helpShow(s, param);
	m_highlight->rehighlight();
}

void ExpressionEdit::helpShow(const QString& funcname, int param)
{
	int op = Operator::nparams(Operator::toOperatorType(funcname));
	if(op) {
		if(op == -1) {
			ajudant(i18nc("n-ary function prototype", "<em>%1</em>(..., <b>par%2</b>, ...)", funcname, param+1));
		} else {
			QString sample = (param < op) ?
						i18nc("Function name in function prototype", "<em>%1</em>(", funcname) :
						i18nc("Uncorrect function name in function prototype", "<em style='color:red'>%1</em>(", funcname);
			for(int i=0; i<op; ++i) {
				if(i==param)
					sample += i18nc("Current parameter in function prototype", "<b>par%1</b>", i+1);
				else
					sample += i18nc("Parameter in function prototype", "par%1", i+1);
				if(i<op-1)
					sample += i18nc("Function parameter separator", ", ");
			}
			ajudant(sample+')');
		}
	} else if(a && a->m_vars->contains(funcname) && a->m_vars->value(funcname)->type()==Object::container) { //if it is a function defined by the user
		Container *c = (Container*) a->m_vars->value(funcname);
		QStringList params = c->bvarList();
		
		QString sample = (param < params.count()) ? //Perhaps we could notify it in a better way
				i18nc("Function name in function prototype", "<em>%1</em>(", funcname) :
                i18nc("Uncorrect function name in function prototype", "<em style='color:red'>%1</em>(", funcname);
		
		for(int i=0; i<params.count(); ++i) {
			if(i==param)
                sample += i18nc("Current parameter in function prototype", "<b>%1</b>", params[i]);
			else
                sample += i18nc("Parameter in function prototype", "%1", params[i]);
			if(i<params.count()-1)
                sample+= i18nc("Function parameter separator", ", ");
		}
		ajudant(sample+')');
	} else
		ajudant(QString());
}

void ExpressionEdit::setAutocomplete(bool a)
{
	m_auto = a;
}

bool ExpressionEdit::autocomplete()
{
	return m_auto;
}

void ExpressionEdit::removenl()
{
	this->setPlainText(this->toPlainText().remove('\n'));
}

void ExpressionEdit::ajudant(const QString& msg)
{
	QFontMetrics fm( font() );
	int curPos = 0;
	curPos=this->textCursor().position();
	int pixelsOffset = fm.width( toPlainText(), curPos );
// 	pixelsOffset -= contentsX();
	QPoint pos = mapToGlobal( QPoint( pixelsOffset, height() ) );
	
	ajudant(msg, pos-QPoint(0, 50));
}

void ExpressionEdit::ajudant(const QString& msg, const QPoint& p)
{
	if(!msg.isEmpty()){
		QFontMetrics fm(m_helptip->font());
		m_helptip->setText(msg);
		m_helptip->setGeometry(QRect(p, p+QPoint(fm.width(msg)+20, 23)));
		
		m_helptip->show();
		m_helptip->raise();
		this->setFocus();
	} else
		m_helptip->hide();
}

void ExpressionEdit::setCorrect(bool correct)
{
	QPalette p;
	QColor c;
	m_correct=correct;
	
	if(m_correct && !isMathML())
		c = QColor(255,255,255);
	else if(m_correct)
		c = QColor(255,255,200);
	else
		c = QColor(255,222,222);
	
	p.setColor(QPalette::Base, c);
	this->setPalette(p);
}

void ExpressionEdit::focusInEvent ( QFocusEvent * event )
{
	QTextEdit::focusInEvent(event);
	switch(event->reason()) {
		case Qt::OtherFocusReason:
		case Qt::TabFocusReason:
			this->selectAll();
		default:
			break;
	}
}

void ExpressionEdit::simplify()
{
	Analitza a;
	a.setExpression(Expression(toPlainText(), isMathML()));
	a.simplify();
	if(isMathML())
		this->setPlainText(a.expression()->toMathML());
	else
		this->setPlainText(a.expression()->toString());
	this->selectAll();
}

void ExpressionEdit::contextMenuEvent(QContextMenuEvent * e)
{
	QMenu *popup = createStandardContextMenu();
	popup->addSeparator();
	if(isMathML())
		popup->addAction(i18n("To Expression"), this, SLOT(toExpression()));
	else
		popup->addAction(i18n("To MathML"), this, SLOT(toMathML()));
	
	popup->addAction(i18n("Simplify"), this, SLOT(simplify()));
	
	popup->exec(e->globalPos());
	delete popup;
}

void ExpressionEdit::setAnalitza(Analitza * in)
{
	m_highlight->setAnalitza(in);
	a=in;
	m_ops->setVariables(a->variables());
	updateCompleter();
}

bool ExpressionEdit::returnPress()
{
	bool haveToPress=false;
	if(isMathML()) {
		emit returnPressed();
	} else {
		Exp ex(toPlainText());
		ex.parse();
		if(ex.isCompletelyRead()) {
			setCorrect(true);
			emit returnPressed();
		} else {
			setCorrect(false);
			haveToPress=true;
		}
	}
	return haveToPress;
}

#include "expressionedit.moc"
