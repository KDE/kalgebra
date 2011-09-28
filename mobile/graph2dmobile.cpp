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
#include <analitzagui/functionsmodel.h>

Graph2DMobile::Graph2DMobile(QDeclarativeItem* parent)
	: QDeclarativeItem(parent), FunctionsPainter(0, boundingRect().size())
	, m_dirty(true)
{
	setSize(QSizeF(100,100));
	setFlag(QGraphicsItem::ItemHasNoContents, false);
	
	defViewport = QRectF(QPointF(-12., 10.), QSizeF(24., -20.));
	resetViewport();
}

void Graph2DMobile::paint(QPainter* p, const QStyleOptionGraphicsItem* options, QWidget* )
{
// 	qDebug() << "hellooo" << boundingRect();
	if(boundingRect().size().isEmpty())
		return;
	
	if(m_buffer.size()!=boundingRect().size()) {
		m_buffer = QPixmap(boundingRect().size().toSize());
		setPaintedSize(boundingRect().size().toSize());
	}
	
	Q_ASSERT(!m_buffer.isNull());
	
	m_buffer.fill(Qt::transparent);
	if(m_dirty) {
		drawFunctions(&m_buffer);
		m_dirty=false;
	}
	
	p->drawPixmap(QPoint(0,0), m_buffer);
}

void Graph2DMobile::forceRepaint()
{
	m_dirty=true;
	update();
}

void Graph2DMobile::resetViewport()
{
	setViewport(defViewport);
}

void Graph2DMobile::modelChanged()
{
	connect(model(), SIGNAL(dataChanged( const QModelIndex&, const QModelIndex& )),
		this, SLOT(updateFuncs(const QModelIndex&, const QModelIndex)));
	connect(model(), SIGNAL( rowsInserted ( const QModelIndex &, int, int ) ),
		this, SLOT(addFuncs(const QModelIndex&, int, int)));
	connect(model(), SIGNAL( rowsRemoved ( const QModelIndex &, int, int ) ),
		this, SLOT(removeFuncs(const QModelIndex&, int, int)));
}

void Graph2DMobile::addFuncs(const QModelIndex&, int start, int end) { updateFunctions(model()->index(start,0), model()->index(end,0)); forceRepaint(); }

void Graph2DMobile::removeFuncs(const QModelIndex&, int, int) { forceRepaint(); }
void Graph2DMobile::updateFuncs(const QModelIndex& start, const QModelIndex& end) { updateFunctions(start, end); }
