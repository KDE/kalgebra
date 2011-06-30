/*************************************************************************************
 *  Copyright (C) 2011 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "graph2dmobile.h"
#include <QPainter>
#include <qstyleoption.h>
#include <KIcon>

Graph2DMobile::Graph2DMobile(QDeclarativeItem* parent)
	: QDeclarativeItem(parent), FunctionsPainter(0, boundingRect().size())
{
	setSize(QSizeF(100,100));
	setFlag(QGraphicsItem::ItemHasNoContents, false);
	
	defViewport = QRectF(QPointF(-12., 10.), QSizeF(24., -20.));
	resetViewport();
}

void Graph2DMobile::paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* )
{
	qDebug() << "hellooo" << boundingRect();
	if(boundingRect().size().isEmpty())
		return;
	
	if(m_buffer.size()!=boundingRect().size()) {
		m_buffer = QPixmap(boundingRect().size().toSize());
		setPaintedSize(boundingRect().size().toSize());
	}
	
	Q_ASSERT(!m_buffer.isNull());
	
	m_buffer.fill(options->palette.color(QPalette::Active, QPalette::Base));
	drawFunctions(&m_buffer);
	
// 	p->begin();
// 	p->setPen(Qt::red);
// 	p->setBrush(Qt::blue);
// 	p->drawPixmap(QPoint(-5,-5), KIcon("kalgebra").pixmap(boundingRect().size().toSize()));
	p->drawPixmap(QPoint(0,0), m_buffer);
	p->drawRect(0,0, m_buffer.width(), m_buffer.height());
// 	p->end();
}

void Graph2DMobile::forceRepaint()
{
	update();
}

void Graph2DMobile::resetViewport()
{
	setViewport(defViewport);
}
