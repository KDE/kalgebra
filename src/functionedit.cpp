/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
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

#include "functionedit.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <KLocale>
#include <KIcon>

#include "analitza.h"
#include "expression.h"
#include "graph2d.h"
#include "algebrahighlighter.h"
#include "variables.h"
#include "functionsmodel.h"
#include "functionfactory.h"
#include <KTabWidget>

namespace {
	static const int resolution = 200;
}

FunctionEdit::FunctionEdit(QWidget *parent)
	: QWidget(parent), m_calcUplimit(0), m_calcDownlimit(0)
{
	this->setWindowTitle(i18n("Add/Edit a function"));
	
	QVBoxLayout *topLayout = new QVBoxLayout(this);
	topLayout->setMargin(2);
	topLayout->setSpacing(5);
	
	m_name = new KLineEdit(this);
	
	m_func = new ExpressionEdit(this);
	m_func->setAns("x");
	connect(m_func, SIGNAL(textChanged()), this, SLOT(edit()));
	connect(m_func, SIGNAL(returnPressed()), this, SLOT(ok()));
	
	m_valid = new QLabel(this);
	m_valid->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	
	QPalette p=palette();
	p.setColor(QPalette::Active, QPalette::Base, Qt::white);
	m_valid->setPalette(p);
	
	m_validIcon = new QLabel(this);
	m_validIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QLayout* validLayout=new QHBoxLayout;
	validLayout->addWidget(m_validIcon);
	validLayout->addWidget(m_valid);
	
	m_color = new KColorCombo(this);
	m_color->setColor(QColor(0,150,0));
	connect(m_color, SIGNAL(currentIndexChanged(int)), this, SLOT(colorChange(int)));
	
	m_funcsModel=new FunctionsModel;
	m_funcsModel->setResolution(resolution);
//	m_funcsModel->addFunction(function(m_name->text(), m_func->expression(), m_color->color()));
	
	KTabWidget* viewTabs=new KTabWidget(this);
	
	m_graph = new Graph2D(m_funcsModel, viewTabs);
	m_graph->setViewport(QRect(QPoint(-5, 7), QPoint(5, -7)));
	m_graph->setFocusPolicy(Qt::NoFocus);
	m_graph->setMouseTracking(false);
	m_graph->setFramed(true);
	m_graph->setReadOnly(true);
	m_graph->setSquares(false);
	
	viewTabs->addTab(m_graph, i18n("Preview"));
	QWidget *options=new QWidget(viewTabs);
	options->setLayout(new QVBoxLayout);
	m_uplimit=new ExpressionEdit(options);
	m_downlimit=new ExpressionEdit(options);
	m_uplimit->setText("2*pi");
	m_downlimit->setText("0");
	options->layout()->addWidget(new QLabel(i18n("From:"), options));
	options->layout()->addWidget(m_downlimit);
	options->layout()->addWidget(new QLabel(i18n("To:"), options));
	options->layout()->addWidget(m_uplimit);
	options->layout()->addItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	viewTabs->addTab(options, i18n("Options"));
	connect(m_uplimit, SIGNAL(textChanged()), this, SLOT(updateUplimit()));
	connect(m_downlimit, SIGNAL(textChanged()), this, SLOT(updateDownlimit()));
	
	QHBoxLayout *m_butts = new QHBoxLayout;
	m_ok = new QPushButton(i18n("OK"), this);
	m_ok->setIcon(KIcon("dialog-ok"));
	QPushButton *m_clear = new QPushButton(i18nc("@action:button", "Clear"), this);
	m_clear->setIcon(KIcon("dialog-cancel"));
	connect(m_ok, SIGNAL(clicked()), this, SLOT(ok()));
	connect(m_clear, SIGNAL(clicked()), this, SLOT(clear()));
	
	topLayout->addWidget(m_name);
	topLayout->addWidget(m_func);
	topLayout->addWidget(m_color);
	topLayout->addLayout(validLayout);
	topLayout->addWidget(viewTabs);
	topLayout->addLayout(m_butts);
	
	m_name->hide(); //FIXME: Remove this when the name has any sense
	
	m_butts->addWidget(m_ok);
	m_butts->addWidget(m_clear);
	
	m_func->setFocus();
	m_ok->setEnabled(false);
}

FunctionEdit::~FunctionEdit()
{
	delete m_vars;
}

void FunctionEdit::clear()
{
	m_func->setText(QString());
	m_funcsModel->clear();
	edit();
}

void FunctionEdit::setFunction(const QString &newText)
{
	m_func->setText(newText);
	m_func->document()->setModified(true);
}

void FunctionEdit::setColor(const QColor &newColor)
{
	m_color->setColor(newColor);
	if(m_funcsModel->rowCount()>0)
		  m_funcsModel->editFunction(0)->setColor(newColor);
	m_graph->forceRepaint();
}

void FunctionEdit::colorChange(int)
{
	setColor(m_color->color());
}

static double calcExp(const Expression& exp, Variables* v, bool* corr)
{
	Q_ASSERT(exp.isCorrect());
	Analitza d(v);
	d.setExpression(exp);
	Expression r=d.calculate();
	
	*corr=r.isCorrect() && r.isReal();
	
	if(*corr)
		return r.toReal().value();
	else
		return 0.;
}

void FunctionEdit::updateUplimit()
{
	bool corr;
	Expression e=m_uplimit->expression();
	if(e.isCorrect()) {
		m_calcUplimit=calcExp(e, m_vars, &corr);
		m_uplimit->setCorrect(corr);
		if(corr)
			edit();
	}
}

void FunctionEdit::updateDownlimit()
{
	bool corr;
	Expression e=m_downlimit->expression();
	if(e.isCorrect()) {
		m_calcDownlimit=calcExp(e, m_vars, &corr);
		m_downlimit->setCorrect(corr);
		if(corr)
			edit();
	}
}

void FunctionEdit::setState(const QString& text, const QColor& state)
{
	QFont errorFont=m_valid->font();
	errorFont.setBold(true);
	m_valid->setFont(errorFont);
	
	QString errorm=text, error=text;
	QFontMetrics fm(errorFont);
	
	if(fm.width(errorm)>m_valid->width()) {
		for(int i=3; i<errorm.size(); ++i) {
			QString aux=i18nc("text ellipsis", "%1...", errorm.mid(0,i));
			
			if(fm.width(aux) <= m_valid->width())
				error=aux;
			else
				break;
		}
	}
	m_valid->setText(error);
	
	QPalette p=m_valid->palette();
	p.setColor(foregroundRole(), state);
	m_valid->setPalette(p);
	
	if(state==Qt::red)
		m_validIcon->setPixmap(KIcon("flag-red").pixmap(QSize(16,16)));
	else
		m_validIcon->setPixmap(KIcon("flag-green").pixmap(QSize(16,16)));
}

///Let's see if the exp is correct
void FunctionEdit::edit()
{
	if(m_func->text().isEmpty()) {
		m_func->setCorrect(true);
		m_ok->setEnabled(false);
		m_valid->clear();
		m_valid->setToolTip(QString());
		m_validIcon->setPixmap(KIcon("flag-yellow").pixmap(QSize(16,16)));
		
		m_funcsModel->clear();
		m_graph->forceRepaint();
		return;
	}
	
	if(!m_uplimit->isCorrect() || !m_downlimit->isCorrect()) {
		setState(i18n("The options you specified are not correct"), Qt::red);
		return;
	}
	
	if(m_calcDownlimit>m_calcUplimit) {
		setState(i18n("Downlimit can't be smaller than uplimit"), Qt::red);
		return;
	}
	
	function f=createFunction();
	if(f.isCorrect()) {
		f.setResolution(resolution);
		f.calc(QPointF());
	}
	
	if(f.isCorrect())
		f.update_points(QRect(-10, 10, 20, -20));
	
	if(f.isCorrect()) {
		m_funcsModel->clear();
		m_funcsModel->addFunction(f);
		setState(QString("%1:=%2")
			.arg(m_name->text()).arg(f.expression().toString()), QColor(0, 140, 0));
	} else {
		QStringList errors = f.errors();
		Q_ASSERT(!errors.isEmpty());
		
		m_funcsModel->clear();
		m_graph->forceRepaint();
// 		m_valid->setText(i18n("<b style='color:red'>WRONG</b>"));
		
		setState(errors.first(), Qt::red);
		m_valid->setToolTip(errors.join("<br />"));
		m_validIcon->setPixmap(KIcon("flag-red").pixmap(QSize(16,16)));
	}
	m_func->setCorrect(f.isCorrect());
	m_ok->setEnabled(f.isCorrect());
}

void FunctionEdit::ok()
{
	if(m_ok->isEnabled())
		emit accept();
}

void FunctionEdit::focusInEvent(QFocusEvent *)
{
	m_func->setFocus();
}

function FunctionEdit::createFunction() const
{
	return function(name(), expression(), m_vars, color(), m_calcUplimit, m_calcDownlimit);
}

#include "functionedit.moc"
