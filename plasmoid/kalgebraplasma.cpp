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

#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/lineedit.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/flash.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>

#include "expression.h"

KAlgebraPlasmoid::KAlgebraPlasmoid(QObject *parent, const QVariantList &args)
	: Plasma::Applet(parent, args)
{
	//this will get us the standard applet background, for free!
	//setDrawStandardBackground(true);
	setSize(200, 200);
}
 
 
KAlgebraPlasmoid::~KAlgebraPlasmoid() {}

void KAlgebraPlasmoid::init()
{
	Plasma::VBoxLayout* m_layout = new Plasma::VBoxLayout( this );
	m_layout->setMargin( 0 );
	m_layout->setSpacing( 0 );
	
	m_input = new Plasma::LineEdit(this);
	m_input->setGeometry(QRectF(QPointF(0,0),size()));
	m_input->setDefaultTextColor(Qt::white);
	m_input->setDefaultText(i18n("Write here the expression..."));
	m_input->setMultiLine(false);
	m_layout->addItem(m_input);
	
	m_output = new Plasma::Label(this);
	m_output->setGeometry(QRectF(QPointF(0,0),size()));
	m_layout->addItem(m_output);
	
	connect(m_input, SIGNAL(editingFinished()), this, SLOT(addOperation()));
}

void KAlgebraPlasmoid::addOperation()
{
// 	m_input->selectAll();
	Expression res;
	a.setExpression(Expression(m_input->toPlainText(), false));
	if(a.isCorrect()) {
		res=a.evaluate();
	}
	
	if(a.isCorrect()) {
		m_output->setPen(QPen(Qt::white));
		m_output->setText(res.toString());
	} else {
		m_output->setPen(QPen(Qt::red));
		m_output->setText(a.errors().join("\n"));
	}
}

#include "kalgebraplasma.moc"
