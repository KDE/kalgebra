/*************************************************************************************
 *  Copyright (C) 2007-2008 by Aleix Pol <aleixpol@kde.org>                          *
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

#ifndef GRAPH2D_H
#define GRAPH2D_H

#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <QPixmap>
#include <QModelIndex>

#include "analitzagui/function.h"

/**
 *	This class lets you create a widget that can draw multiple 2D graphs.
 *	@author Aleix Pol Gonzàlez
 */

class FunctionsModel;

class Graph2D : public QWidget
{
Q_OBJECT
public:
	/** The graph mode will especify the selection mode we are using at the moment */
	enum GraphMode {
		None=0,		/**< Normal behaviour */
		Pan,		/**< Panning, translates the viewport. */
		Selection	/**< There is a rectangle delimiting a region, for zooming. */
	};
	
	/** Constructor. Constructs a new Graph2D. */
	explicit Graph2D(FunctionsModel* fm, QWidget *parent = 0);
	
	/** Destructor. */
	~Graph2D();
	
	void setModel(FunctionsModel* f);
	
	QSize sizeHint() const { return QSize(100, 100); }
	
	/** Saves the graphs to a file located at @p path. */
	bool toImage(const QString& path);
	
	/** Sets whether we will see a grid or only the axes. */
	void setSquares(bool newSquare) { m_squares=newSquare; forceRepaint(); }
	
	/** Returns whether we have chosen to see the grid. */
	bool squares() const {return m_squares;}
	
	/** Makes that no function is selected. */
	void unselect();
	
	/** Returns whether it has a little border frame. */
	bool isFramed() const { return m_framed; }
	
	/** Sets whether it has a little border frame. */
	void setFramed(bool fr) { m_framed=fr; }
	
	/** Returns whether it is a read-only widget. */
	bool isReadOnly() const { return m_readonly; }
	
	/** Sets whether it is a read-only widget. */
	void setReadOnly(bool ro);
	
	/** Sets whether it has to keep the aspect ratio (1:1 grid). */
	void setKeepAspectRatio(bool ar);
	
	/** Sets whether it is keeping the aspect ratio (1:1 grid). */
	bool keepAspectRatio() const { return m_keepRatio; }
	
	/** Returns the viewing port */
	QRectF definedViewport() const;
	
public slots:
	/** Marks the image as dirty and repaints everything. */
	void forceRepaint() { valid=false; repaint(); }

	/** Sets the viewport to a default viewport. */
	void resetViewport() { setViewport(defViewport); }
	
	/** Zooms in to the Viewport center */
	void zoomIn();
	
	/** Zooms out */
	void zoomOut();
	
	/** Force the functions from @p start to @p end to be recalculated. */
	void update( const QModelIndex & start, const QModelIndex& end );
	
	/** Sets the graph's viewport to @p v. */
	void setViewport(const QRectF& v, bool repaint=true);
	
signals:
	/** Emits a status when it changes. */
	void status(const QString &msg);
	
	void viewportChanged(const QRectF&);
	
private slots:
	void addFuncs( const QModelIndex & parent, int start, int end);
	void removeFuncs( const QModelIndex & parent, int start, int end);
	
private:
	static const QColor m_axeColor;
	static const QColor m_axe2Color;
	static const QColor m_derivativeColor;
	
	//painting
	FunctionsModel* m_model;
	QPixmap buffer;
	bool valid;
	QLabel *micepos;
	QPointF mark;
	void drawAxes(QPainter*, function::Axe a);
	void drawPolarAxes(QPainter*);
	void drawCartesianAxes(QPainter*);
	QPointF toWidget(const QPointF &) const;
	QPointF fromWidget(const QPoint& p) const;
	QPointF toViewport(const QPoint& mv) const;
	QPointF calcImage(const QPointF& dp);
	QLineF slope(const QPointF& dp) const;
	
	QLineF toWidget(const QLineF &) const;
	
	//events
	void paintEvent( QPaintEvent * );
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void keyPressEvent(QKeyEvent * e );
	void wheelEvent(QWheelEvent *e);
	void resizeEvent(QResizeEvent *);
	GraphMode mode;
	QPoint press; QPoint last;
	
	//presentation
	bool m_squares;
	bool m_keepRatio;
	double rang_x, rang_y;
    QPointF ant;
    QRectF viewport;
    QRectF userViewport;
	QRectF defViewport;
	void drawFunctions(QPaintDevice*);
	void updateScale(bool repaint=true);
		
	void sendStatus(const QString& msg) { emit status(msg); }
	bool m_framed;
	bool m_readonly;
	QString m_posText;
	static QRect toBiggerRect(const QRectF&);
};

#endif
