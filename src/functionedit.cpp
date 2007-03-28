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

#include "functionedit.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <klocale.h>

#include "expression.h"

FunctionEdit::FunctionEdit(QWidget *parent, Qt::WFlags f) :
		QWidget(parent, f), m_correct(false)
{
	this->setWindowTitle(i18n("Add/Edit a function"));
	/*buttonBox = new QDialogButtonBox(this);
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
	QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));*/

// 	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
// 	this->setFixedWidth(400);
// 	this->setFixedHeight(350);
	
	QVBoxLayout *topLayout = new QVBoxLayout(this);
	topLayout->setMargin(2);
	topLayout->setSpacing(5);
	
	m_name = new QLineEdit(this);
	
	m_func = new ExpressionEdit(this);
	m_func->setAns("x");
	connect(m_func, SIGNAL(textChanged()), this, SLOT(edit()));
	connect(m_func, SIGNAL(returnPressed()), this, SLOT(ok()));
	
	m_valid = new QLabel(this);
	m_valid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	m_color = new ColorCombo(this);
	m_color->setColor(QColor(0,150,0));
	connect(m_color, SIGNAL(currentIndexChanged(int)), this, SLOT(colorChange(int)));
	
	m_graph = new Graph2D(this);
	m_graph->setViewport(QRect(QPoint(-5, 7), QPoint(5, -7)));
	m_graph->setResolution(200);
	m_graph->setFocusPolicy(Qt::NoFocus);
	m_graph->addFunction(function(m_name->text(), Expression(m_func->text(), m_func->isMathML()), m_color->color(), true));
	m_graph->setMouseTracking(false);
	m_graph->setFramed(true);
	m_graph->setReadOnly(true);
	m_graph->setSquares(false);
	
	QHBoxLayout *m_butts = new QHBoxLayout(0);
	m_ok = new QPushButton(i18n("OK"), this);
	QPushButton *m_clear = new QPushButton(i18n("Clear"), this);
	connect(m_ok, SIGNAL(clicked()), this, SLOT(ok()));
	connect(m_clear, SIGNAL(clicked()), this, SLOT(clear()));
	
	topLayout->addWidget(m_name);
	topLayout->addWidget(m_func);
	topLayout->addWidget(m_color);
	topLayout->addWidget(m_valid);
	topLayout->addWidget(m_graph);
	topLayout->addLayout(m_butts);
	
	m_name->hide(); //FIXME: Remove this when the name has any sense
	
	m_butts->addWidget(m_ok);
	m_butts->addWidget(m_clear);
	
	m_func->setFocus();
	m_ok->setEnabled(false);
}

FunctionEdit::~FunctionEdit()
{}


void FunctionEdit::clear()
{
	m_func->setText(QString());
	edit();
}

void FunctionEdit::setText(const QString &newText)
{
	m_func->setText(newText);
	m_func->document()->setModified(true);
}

void FunctionEdit::setColor(const QColor &newColor)
{
	m_color->setColor(newColor);
	m_graph->editFunction(0)->setColor(newColor);
	m_graph->forceRepaint();
}

void FunctionEdit::colorChange(int)
{
	setColor(m_color->color());
}

void FunctionEdit::edit()	//Let's see if the exp is correct
{
	Analitza a;
	QString funct = m_func->text();
	
	if(m_func->text().isEmpty()) {
		m_func->setCorrect(true);
		m_ok->setEnabled(false);
		m_valid->setText(QString());
		return;
	}
	
	if(!m_func->isMathML())
		a.expression()->setText(funct);
	else
		a.expression()->setMathML(funct);
	
	if(a.isCorrect()) {
		QStringList bvl = a.bvarList();
		QString var = bvl.count()==0 ? "x" : bvl[0];
		a.m_vars->modify(var, 0.);
		m_valid->setText(QString("<b style='color:#090'>%1:=%2</b>").arg(m_name->text()).arg(a.expression()->toString()));
		a.calculate();
	} else
		a.errors() << i18n("From parser:") << a.expression()->error();
	
	m_correct=a.isCorrect();
	if(m_correct) {
		m_graph->clear();
		m_graph->addFunction(function(m_name->text(), Expression(m_func->toPlainText(), m_func->isMathML()), m_color->color(), true));
		m_valid->setToolTip(QString());
	} else {
		m_graph->clear();
		m_graph->forceRepaint();
		m_valid->setText(i18n("<b style='color:red'>WRONG</b>"));
		m_valid->setToolTip(a.errors().join("\n"));
	}
	m_func->setCorrect(m_correct);
	m_ok->setEnabled(m_correct);
}

void FunctionEdit::ok()
{
	if(m_correct)
		emit accept();
}

///////////////////////////////////////
ColorCombo::ColorCombo(QWidget* parent) : QComboBox(parent)
{
	setIconSize(QSize(width()*2, QFontMetrics(font()).height()));
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	setColor(Qt::green);
	setColor(Qt::black);
	setColor(Qt::blue);
	setColor(Qt::red);
}

void ColorCombo::resizeEvent(QResizeEvent * e)
{
	QWidget::resizeEvent(e);
	QSize s = QSize(width(), QFontMetrics(font()).height()+2);
// 	setIconSize(s);
}

void ColorCombo::setColor(const QColor &color)
{
	int pos = findData(color.name());
	if(pos==-1) {
		QPixmap p(iconSize());
		p.fill(color);
		addItem(p, QString(), color.name());
		setCurrentIndex(count()-1);
	} else {
		setCurrentIndex(pos);
	}
}

QColor ColorCombo::color() const
{
	return QColor(itemData(currentIndex()).toString());
}

#include "functionedit.moc"
