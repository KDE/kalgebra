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

#include "expressionedit.h"

#include <QMenu>
#include <QLabel>
#include <QKeyEvent>
#include <QTreeView>
#include <QHeaderView>
#include <QApplication>
#include <QTimer>
#include <QPropertyAnimation>

#include <KLocale>

#include <analitzagui/operatorsmodel.h>
#include <analitza/explexer.h>
#include <analitza/expressionparser.h>
#include <analitza/operator.h>
#include <analitza/analyzer.h>
#include <analitza/variables.h>

class HelpTip : public QLabel
{
	public:
		HelpTip(QWidget* parent)
			: QLabel(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool | Qt::X11BypassWindowManagerHint)
		{
			setFrameShape(QFrame::Box);
			setFocusPolicy(Qt::NoFocus);
			setAutoFillBackground(false);
			
			QPalette p=palette();
			p.setColor(backgroundRole(), p.color(QPalette::Active, QPalette::ToolTipBase));
			setPalette(p);
		}
		
		void mousePressEvent(QMouseEvent*)
		{
			hide();
		}
};

ExpressionEdit::ExpressionEdit(QWidget *parent, AlgebraHighlighter::Mode inimode)
	: QPlainTextEdit(parent), m_histPos(0), help(true), m_auto(true), a(0), m_correct(true), m_ans("ans")
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setTabChangesFocus(true);
	m_history.append(QString());
	
	m_helptip = new HelpTip(this);
	m_helptip->hide();
	m_hideHelpTip = new QTimer(this);
	m_hideHelpTip->setInterval(500);
	connect(m_hideHelpTip, SIGNAL(timeout()), m_helptip, SLOT(hide()));
	
	m_highlight= new AlgebraHighlighter(this->document(), a);
	
	m_completer = new QCompleter(this);
	m_completer->setWidget(this);
	m_completer->setCompletionColumn(0);
	m_completer->setCompletionRole(Qt::DisplayRole);
	QTreeView* completionView = new QTreeView;
	m_completer->setPopup(completionView);
	completionView->setRootIsDecorated(false);
	completionView->header()->hide();
// 	completionView->resizeColumnToContents(1);
	completionView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	completionView->setMinimumWidth(300);
	m_ops = new OperatorsModel(m_completer);
	m_completer->setModel(m_ops);
	
	updateCompleter();
	
	completionView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
	completionView->showColumn(0);
	completionView->showColumn(1);
	completionView->hideColumn(2);
	completionView->hideColumn(3);
	
	connect(this, SIGNAL(returnPressed()), this, SLOT(returnP()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMov()));
	connect(this, SIGNAL(signalHelper(const QString&)), this, SLOT(helper(const QString&)));
	connect(m_completer, SIGNAL(activated(const QString&)), this, SLOT(completed(const QString&)));
// 	connect(m_completer, SIGNAL(activated(const QModelIndex&)), this, SLOT(completed(const QModelIndex&)));
	
	setMode(inimode);
	m_lineHeight = QFontMetrics(currentCharFormat().font()).height();
	setFixedHeight(m_lineHeight+15);
}

ExpressionEdit::~ExpressionEdit()
{
	delete m_highlight;
}

void ExpressionEdit::setExpression(const Analitza::Expression& e)
{
	if(!e.isCorrect())
		setText(QString());
	else if(isMathML())
		setText(e.toMathML());
	else
		setText(e.toString());
	
	setCorrect(e.isCorrect());
}

void ExpressionEdit::updateCompleter()
{
	m_ops->updateInformation();
}
 
void ExpressionEdit::completed(const QString& newText)
{
	int c = newText.length() - lastWord(textCursor().selectionStart()).length();
	QString toInsert=newText.right(c);
	if(Analitza::Expression::whatType(newText) == Analitza::Object::oper && !isMathML())
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
			return Analitza::Expression::isMathML(this->toPlainText());
	}
}

void ExpressionEdit::setMode(AlgebraHighlighter::Mode en)
{
	bool correct=true;
	if(!text().isEmpty()) {
		if(isMathML() && en==AlgebraHighlighter::Expression) { //We convert it into MathML
			Analitza::Expression e(toPlainText(), true);
			correct=e.isCorrect();
			if(correct) setPlainText(e.toString());
		} else if(!isMathML() && en==AlgebraHighlighter::MathML) {
			Analitza::Expression e(toPlainText(), false);
			correct=e.isCorrect();
			if(correct) setPlainText(e.toMathML());
		}
	}
	if(correct)
		m_highlight->setMode(en);
	
	setCorrect(correct);
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
	QAbstractItemView* completionView = m_completer->popup();
	
	switch(e->key()){
		case Qt::Key_F2:
			simplify();
			break;
		case Qt::Key_F3:
			setMode(isMathML() ? AlgebraHighlighter::Expression : AlgebraHighlighter::MathML);
			break;
		case Qt::Key_Escape:
			if(!completionView->isVisible())
				selectAll();
			
			completionView->hide();
			m_helptip->hide();
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			if(completionView->isVisible() && !completionView->selectionModel()->selectedRows().isEmpty()) 
				completed(m_completer->currentCompletion());
			else if(returnPress())
					QPlainTextEdit::keyPressEvent(e);
			completionView->hide();
			break;
		case Qt::Key_Up:
			if(!completionView->isVisible()) {
				m_histPos--;
				ch=true;
			}
			break;
		case Qt::Key_Down:
			if(!completionView->isVisible()) {
				m_histPos++;
				ch=true;
			}
			break;
		case Qt::Key_Left:
		case Qt::Key_Right:
			m_highlight->rehighlight();
			QPlainTextEdit::keyPressEvent(e);
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
			QPlainTextEdit::keyPressEvent(e);
			break;
		case Qt::Key_Alt:
			QPlainTextEdit::keyPressEvent(e);
			break;
		default:
			QPlainTextEdit::keyPressEvent(e);
			m_history.last() = this->toPlainText();
			QString last = lastWord(textCursor().selectionStart());
			if(!last.isEmpty()) {
				m_completer->setCompletionPrefix(last);
				m_completer->complete();
			} else {
				completionView->hide();
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
		completionView->hide();
	}
	
	int lineCount=toPlainText().count('\n')+1;
	setFixedHeight(m_lineHeight*lineCount+15);
	setCorrect(m_highlight->isCorrect());
}

QString ExpressionEdit::lastWord(int pos)
{
	QString exp=text();
	int act=pos-1;
	for(; act>=0 && exp[act].isLetter(); act--) {}
	
	return exp.mid(act+1, pos-act-1);
}

void ExpressionEdit::cursorMov()
{
	int pos=this->textCursor().position();
	m_highlight->setPos(pos);
	if(text().isEmpty())
		setCorrect(true);
	
	QString help = helpShow(m_highlight->editingName(),
							m_highlight->editingParameter(),
							m_highlight->editingBounds(),
							a ? a->variables() : 0);
	
	if(help.isEmpty()) {
		if(isCorrect()) {
			QTimer::singleShot(500, this, SLOT(showSimplified()));
		}
	} else
		helper(help);
}


void ExpressionEdit::showSimplified()
{
	Analitza::Analyzer a;
	a.setExpression(expression());
	QString help;
	if(a.isCorrect()) {
		a.simplify();
		help=i18n("Result: %1", a.expression().toString());
	}
	helper(help);
}


QString ExpressionEdit::helpShow(const QString& funcname, int param, bool inbounds, const Analitza::Variables* v)
{
	QString ret;
	Analitza::Operator::OperatorType o=Analitza::Operator::toOperatorType(funcname);
	
	static QString bounds=i18nc("Current parameter is the bounding", " : bounds");
	if(o!=Analitza::Operator::none) {
		Analitza::Operator oper(o);
		const int op=oper.nparams();
		if(op == -1) {
			ret=i18nc("n-ary function prototype", "<em>%1</em>(..., <b>par%2</b>, ...)",
							 funcname, param+1);
		} else {
			QString sample = (param < op || (inbounds && oper.isBounded())) ?
						i18nc("Function name in function prototype", "<em>%1</em>(", funcname) :
						i18nc("Uncorrect function name in function prototype", "<em style='color:red'><b>%1</b></em>(", funcname);
			
			for(int i=0; i<op; ++i) {
				QString current=i18nc("Parameter in function prototype", "par%1", i+1);
				
				if(i==param)
					current=i18nc("Current parameter in function prototype", "<b>%1</b>", current);
				sample += current;
				if(i<op-1)
					sample += i18nc("Function parameter separator", ", ");
			}
			
			if(oper.isBounded()) {
				QString p=bounds;
				if(inbounds)
					p=i18nc("Current parameter in function prototype", "<b>%1</b>", p);
				sample += p;
			}
			
			ret=sample+')';
		}
	} else if(v && v->contains(funcname)) { //if it is a function defined by the user
		Analitza::Expression val=v->valueExpression(funcname);
		if(val.isLambda()) {
			QStringList params = val.bvarList();
			
			QString sample = (param < params.count()) ? //Perhaps we could notify it in a better way
					i18nc("Function name in function prototype", "<em>%1</em>(", funcname) :
					i18nc("Uncorrect function name in function prototype", "<em style='color:red'>%1</em>(",
						  funcname);
			
			for(int i=0; i<params.count(); ++i) {
				if(i==param)
					sample += i18nc("Current parameter in function prototype", "<b>%1</b>", params[i]);
				else
					sample += params[i];
				
				if(i<params.count()-1)
					sample+= i18nc("Function parameter separator", ", ");
			}
			ret=sample+')';
		}
	}
	return ret;
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

void ExpressionEdit::helper(const QString& msg)
{
	QPoint pos = mapToGlobal( QPoint( cursorRect().left(), height() ) );
	
	if(msg.isEmpty()) {
		if(!m_hideHelpTip->isActive())
			m_hideHelpTip->start();
	} else {
		helper(msg, pos-QPoint(0, 50));
		m_hideHelpTip->stop();
	}
}

void ExpressionEdit::helper(const QString& msg, const QPoint& p)
{
	if(isVisible()){
		m_helptip->setText(msg);
		m_helptip->resize(m_helptip->sizeHint());
		if(!m_helptip->isVisible()) {
			m_helptip->move(p);
			
			m_helptip->show();
			m_helptip->raise();
		} else {
			QPropertyAnimation* anim = new QPropertyAnimation(m_helptip, "pos", this);
			anim->setEndValue(p);
			anim->start(QAbstractAnimation::DeleteWhenStopped);
		}
		setFocus();
	}
}

void ExpressionEdit::setCorrect(bool correct)
{
	QPalette p=qApp->palette();
	QColor c;
	m_correct=correct;
	
	if(m_correct && !isMathML())
		c = p.base().color();
	else if(m_correct)
		c = QColor(255,255,200);
	else //if mathml
		c = QColor(255,222,222);
	
	p.setColor(QPalette::Active, QPalette::Base, c);
	setPalette(p);
}

void ExpressionEdit::focusInEvent ( QFocusEvent * event )
{
	QPlainTextEdit::focusInEvent(event);
	switch(event->reason()) {
		case Qt::OtherFocusReason:
		case Qt::TabFocusReason:
			this->selectAll();
		default:
			break;
	}
}

void ExpressionEdit::focusOutEvent ( QFocusEvent * )
{
	m_helptip->hide();
}

void ExpressionEdit::simplify()
{
	Analitza::Analyzer a;
	a.setExpression(expression());
	if(a.isCorrect()) {
		a.simplify();
		setExpression(a.expression());
	}
	selectAll();
}

void ExpressionEdit::contextMenuEvent(QContextMenuEvent * e)
{
	QScopedPointer<QMenu> popup(createStandardContextMenu());
	popup->addSeparator();
	if(isMathML())
		popup->addAction(i18n("To Expression"), this, SLOT(toExpression()));
	else
		popup->addAction(i18n("To MathML"), this, SLOT(toMathML()));
	
	popup->addAction(i18n("Simplify"), this, SLOT(simplify()));
	
	QMenu* examples=popup->addMenu(i18n("Examples"));
	examples->setEnabled(!m_examples.isEmpty());
	foreach(const QString &example, m_examples) {
		QAction* ac=examples->addAction(example);
		ac->setData(example);
	}
	connect(examples, SIGNAL(triggered(QAction*)), SLOT(setActionText(QAction*)));
	
	popup->exec(e->globalPos());

}

void ExpressionEdit::setActionText(QAction* text)
{
	setPlainText(text->data().toString());
}

void ExpressionEdit::setAnalitza(Analitza::Analyzer * in)
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
		ExpLexer lex(toPlainText());
		ExpressionParser ex;
		ex.parse(&lex);
		if(lex.isCompletelyRead()) {
			setCorrect(true);
			emit returnPressed();
		} else {
			setCorrect(false);
			haveToPress=true;
		}
	}
	m_helptip->hide();
	return haveToPress;
}

void ExpressionEdit::insertText(const QString& text)
{
	QTextCursor tc=textCursor();
	tc.insertText(text);
}

Analitza::Expression ExpressionEdit::expression() const
{
	return Analitza::Expression(text(), isMathML());
}

#include "expressionedit.moc"
