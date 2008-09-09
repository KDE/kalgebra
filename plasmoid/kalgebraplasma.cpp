/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include "kalgebraplasma.h"

#include <QPainter>
#include <QFontMetrics>
#include <QSizeF>
#include <QLabel>
#include <QGraphicsProxyWidget>
#include <QFont>

#include <plasma/theme.h>
#include <plasma/dataengine.h>
// #include <plasma/widgets/flash.h>
// #include <plasma/layouts/boxlayout.h>
// #include <plasma/widgets/icon.h>

#include "expression.h"

using namespace Plasma;

QColor KAlgebraPlasmoid::correctColor() { return Theme::defaultTheme()->color(Theme::TextColor);}
QColor KAlgebraPlasmoid::errorColor() { return Qt::red; }
int KAlgebraPlasmoid::simplificationSize() {  return Theme::defaultTheme()->font(Theme::DefaultFont).pointSize(); }
int KAlgebraPlasmoid::resultSize() { return simplificationSize()*2; }

KAlgebraPlasmoid::KAlgebraPlasmoid(QObject *parent, const QVariantList &args)
	: Applet(parent, args), m_layout(0)
{
	setBackgroundHints(TranslucentBackground);
}

KAlgebraPlasmoid::~KAlgebraPlasmoid() {}

void KAlgebraPlasmoid::init()
{
	/*
	QGraphicsWidget* graphics = new QGraphicsWidget( this );
	QGraphicsProxyWidget * graphicsWidget = new QGraphicsProxyWidget(graphics);
	m_input = new KLineEdit;
	m_input->setClickMessage(i18n("Enter the expression..."));
	m_input->setAttribute( Qt::WA_NoSystemBackground );
	graphicsWidget->setWidget(m_input);*/
	
	m_input = new Plasma::LineEdit(this);
	
	m_output = new Plasma::Label(this);
	m_output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_output->nativeWidget()->setAlignment(Qt::AlignCenter);
	
	m_layout = new QGraphicsLinearLayout(this);
	m_layout->setOrientation( Qt::Vertical );
	m_layout->addItem(m_input);
	m_layout->addItem(m_output);
	
	connect(m_input, SIGNAL(editingFinished()), this, SLOT(addOperation()));
	connect(m_input->nativeWidget(), SIGNAL(textChanged ( const QString & )), this, SLOT(simplify()));
	
	resize(300,300);
}

void KAlgebraPlasmoid::updateFactor()
{
// 	switch(formFactor()) {
// 		case Horizontal:
// 			m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
// 			m_layout->setOrientation(Qt::Horizontal);
// 			break;
// 		default:
// 			m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
// 			m_layout->setOrientation(Qt::Vertical);
// 			break;
// 	}
}

void KAlgebraPlasmoid::constraintsEvent(Constraints constraints)
{
	Q_ASSERT(m_layout);
	if(constraints & FormFactorConstraint) {
		updateFactor();
	}
}

void KAlgebraPlasmoid::addOperation()
{
	if ( m_input->text().isEmpty() )
		return;
	Expression res;
	a.setExpression(Expression(m_input->text(), false));
	if(a.isCorrect()) {
		res=a.evaluate();
	}
	
	QColor c;
	if(a.isCorrect()) {
		m_output->setText(res.toString());
		c=correctColor();
	} else {
		m_output->setText(a.errors().join("\n"));
		c=errorColor();
	}
	plasmoidFont(true, c, true);
	update();
}

void KAlgebraPlasmoid::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
// 	m_input->selectAll();
	m_input->setFocus(Qt::MouseFocusReason);
}

void KAlgebraPlasmoid::plasmoidFont(bool big, const QColor& c, bool bold)
{
	QFont f=m_output->nativeWidget()->font();
	f.setBold(bold);
	int size;
	if(big) {
		size=(m_output->size().height()*2)/3;
		f.setPointSize(size);
		QFontMetrics fm(f);
		for(; fm.width(m_output->text()) > m_output->nativeWidget()->width(); size--) {
			f.setPointSize(size);
			fm=QFontMetrics(f);
		}
	} else
		size=simplificationSize();
	f.setPointSize(size);
	
	m_output->nativeWidget()->setFont(f);
	
	QPalette palette = m_output->palette();
	palette.setColor(QPalette::WindowText, c);
	m_output->nativeWidget()->setPalette(palette);
}

void KAlgebraPlasmoid::simplify()
{
	Expression res;
	a.setExpression(Expression(m_input->text(), false));

	if(a.isCorrect()) {
		a.simplify();
		res=*a.expression();
		m_output->setText(res.toString());

		plasmoidFont(false, Qt::white, true);
	} else
		m_output->setText(QString());
}

#include "kalgebraplasma.moc"
