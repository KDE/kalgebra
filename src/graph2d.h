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

#ifndef GRAPH2D_H
#define GRAPH2D_H

#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QToolTip>
#include <QLabel>
#include <QCursor>
#include <QPixmap>

#include "function.h"

/**
 *	This class lets you create a widget that can draw multiple 2D graphs.
 *	@author aleix
 */

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
	Graph2D(QWidget *parent = 0);
	
	/** Destructor. */
	~Graph2D();
	
	/** Adds another function @p f. Returns whether another function like @p f existed. */
	bool addFunction(const function& f);
	
	/** Sets the function selected to @p exp. Returns whether another function like @p exp existed. */
	bool setSelected(const QString& exp);
	
	/** Specifies that the @p exp function is shown. Returns whether another function like @p exp existed. */
	bool setShown(const QString& exp, bool shown);
	
	/** Edits the @p num nth function. The @p num should be less than the number of functions, because you are editing. */
	void editFunction(int num, const function& func);
	
	/** Edits the @p exp function. Returns whether another function like @p exp existed. */
	bool editFunction(const QString& name, const function& func);
	
	/** Returns a pointer to the @p num nth function. */
	function* editFunction(int num);
	
	/** Saves the graphs to a file located at @p path. */
	bool toImage(const QString& path);
	
	/** Specifies the widget size policy. To know more about it, read the Qt documentation */
	virtual QSizePolicy sizePolicy() const;
	
	/** Sets whether we will see a grid or only the axes. */
	void setSquares(bool newSquare) {m_squares=newSquare; valid=false; }
	
	/** Returns whether we have chosen to see the grid. */
	bool squares() const {return m_squares;}
	
	/** Makes that no function is selected. */
	void unselect();
	
	/** Sets the graph's viewport to @p v. */
	void setViewport(const QRectF& v);
	
	/** Sets the desired maximum resolution to @p res. */
	void setResolution(int res);
	
	/** Removes all graphs. */
	void clear();
	
	/** Returns whether it has a little border frame. */
	bool isFramed() const { return m_framed; }
	
	/** Sets whether it has a little border frame. */
	void setFramed(bool fr) { m_framed=fr; }
	
	/** Returns whether it is a read-only widget. */
	bool isReadOnly() const { return m_readonly; }
	
	/** Sets whether it is a read-only widget. */
	void setReadOnly(bool ro) { m_readonly=ro; setMouseTracking(!ro); }
	
	/** Returns how many function there are in the widget. */
	int count() const { return funclist.count(); }
private:
	static const QColor m_axeColor;
	static const QColor m_axe2Color;
	
	//painting
	QPixmap buffer;
	QPixmap front;
	bool valid;
	QPainter finestra;
	QLabel *micepos;
	QList<function> funclist;
	QPointF mark;
	void drawAxes(QPainter*, Axe a);
	void drawPolarAxes(QPainter*);
	void drawCartesianAxes(QPainter*);
	void update_points();
	QPointF toWidget(const QPointF &) const;
	QPointF fromWidget(const QPointF& p) const;
	QPointF toViewport(const QPoint& mv) const;
	QPointF calcImage(const QPointF& dp);
	double pendent(const QPointF& dp) const;
	
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
	
	//representacio
	bool m_squares;
	double resolucio;
	double rang_x, rang_y;
	QPointF ant;
	QRectF viewport;
	QRectF defViewport;
	void pintafunc(QPaintDevice*);
	void update_scale();
		
	void sendStatus(const QString& msg) { emit status(msg); }
	bool m_framed;
	bool m_readonly;
	QString m_posText;
	static QRect toBiggerRect(const QRectF&);
public slots:
	/** Makes the image as dirty and repaints everything. */
	void forceRepaint() { valid=false; repaint(); }
	
	/** Sets the viewport to a default viewport. */
	void resetViewport() { setViewport(defViewport); }
signals:
	/** Emits a status when it changes. */
	void status(const QString &msg);
};

#endif
