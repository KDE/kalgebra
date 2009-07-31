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
	
	m_name = new KLineEdit(this);
	
	m_func = new ExpressionEdit(this);
	m_func->setAns("x");
	connect(m_func, SIGNAL(textChanged()), this, SLOT(edit()));
	connect(m_func, SIGNAL(returnPressed()), this, SLOT(ok()));
	
	m_valid = new QLabel(this);
	m_valid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_validIcon = new QLabel(this);
	m_validIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QLayout* validLayout=new QHBoxLayout;
	validLayout->addWidget(m_validIcon);
	validLayout->addWidget(m_valid);
	
	m_color = new KColorCombo(this);
	m_color->setColor(QColor(0,150,0));
	connect(m_color, SIGNAL(currentIndexChanged(int)), this, SLOT(colorChange(int)));
	
	m_funcsModel=new FunctionsModel;
//	m_funcsModel->addFunction(function(m_name->text(), m_func->expression(), m_color->color()));
	
	m_graph = new Graph2D(m_funcsModel, this);
	m_graph->setViewport(QRect(QPoint(-5, 7), QPoint(5, -7)));
	m_graph->setResolution(200);
	m_graph->setFocusPolicy(Qt::NoFocus);
	m_graph->setMouseTracking(false);
	m_graph->setFramed(true);
	m_graph->setReadOnly(true);
	m_graph->setSquares(false);
	
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
	topLayout->addWidget(m_graph);
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

void FunctionEdit::setText(const QString &newText)
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

///Let's see if the exp is correct
void FunctionEdit::edit()
{
	if(m_func->text().isEmpty()) {
		m_func->setCorrect(true);
		m_ok->setEnabled(false);
		m_valid->setText(QString());
		m_valid->setToolTip(QString());
		m_validIcon->setPixmap(KIcon("flag-yellow").pixmap(QSize(16,16)));
		
		m_funcsModel->clear();
		m_graph->forceRepaint();
		return;
	}
	
	Analitza a(m_vars);
	a.setExpression(m_func->expression());
	
	Expression res;
	QStringList errors;
	if(a.isCorrect()) {
		QStringList bvl = a.bvarList();
		QString var = bvl.isEmpty() ? "x" : bvl[0];
		
		if(function::supportedBoundedVars().contains(var)) {
			a.insertVariable(var, Cn(0.));
		} else {
			errors.append(i18nc("Error message", "Unknown bounded variable: %1", bvl.join(", ")));
		}
		
		res=a.calculate();
	}
	
	m_correct= a.isCorrect() && res.isValue();
	function f;
	if(m_correct) {
		f=function(m_name->text(), m_func->expression(), m_vars, m_color->color());
		
		if(f.isCorrect())
			f.update_points(QRect(-10, 10, 10, -10), 100);
	}
	m_correct=m_correct && f.isCorrect();
	
	if(m_correct) {
		m_funcsModel->clear();
		m_funcsModel->addFunction(f);
		m_valid->setToolTip(QString());
		m_valid->setText(QString("<b style='color:#090'>%1:=%2</b>")
			.arg(m_name->text()).arg(a.expression().toString()));
		m_validIcon->setPixmap(KIcon("flag-green").pixmap(QSize(16,16)));
	} else {
		errors += a.errors();
		errors += f.errors();
		if(a.isCorrect() && !res.isValue())
			errors.append(i18n("We can only draw Real results."));
		
		m_funcsModel->clear();
		m_graph->forceRepaint();
// 		m_valid->setText(i18n("<b style='color:red'>WRONG</b>"));
		Q_ASSERT(!errors.isEmpty());
		
		QString errorm=errors.first(), error=errors.first();
		QFontMetrics fm(m_valid->font());
		int textWidth=fm.width(errorm);
		
		if(textWidth>m_valid->width()) {
			for(int i=3; i<errorm.size(); ++i) {
				QString aux=errorm.mid(0,i)+"...";
				
				int textWidth=fm.width(aux);
				if(textWidth > m_valid->width()) {
					break;
				} else
					error=aux;
			}
		}
		m_valid->setText(i18n("<b style='color:red'>%1</b>", error));
		m_valid->setToolTip(errors.join("\n"));
		m_validIcon->setPixmap(KIcon("flag-red").pixmap(QSize(16,16)));
	}
	m_func->setCorrect(m_correct);
	m_ok->setEnabled(m_correct);
}

void FunctionEdit::ok()
{
	if(m_correct)
		emit accept();
}

void FunctionEdit::focusInEvent(QFocusEvent *)
{
	m_func->setFocus();
}

#if 0
//////////////////////////////////////////
///////////ColorCombo is deprecated.//////
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
#endif

#include "functionedit.moc"
