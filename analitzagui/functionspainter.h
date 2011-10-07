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


#ifndef FUNCTIONSPAINTER_H
#define FUNCTIONSPAINTER_H
#include "function.h"
#include "analitzaguiexport.h"

class QModelIndex;
class FunctionsModel;

class ANALITZAGUI_EXPORT FunctionsPainter
{
	public:
		FunctionsPainter(FunctionsModel* model, const QSizeF& size);
		virtual ~FunctionsPainter();
		
		virtual void drawFunctions(QPaintDevice *qpd);
		virtual void forceRepaint() = 0;
		virtual void viewportChanged() = 0;
		virtual int currentFunction() const = 0;
		virtual void modelChanged() {}
		
		/** Sets whether we will see a grid or only the axes. */
		void setSquares(bool newSquare) { m_squares=newSquare; forceRepaint(); }
		
		/** Returns whether we have chosen to see the grid. */
		bool squares() const {return m_squares;}
		
		/** Sets whether it has to keep the aspect ratio (1:1 grid). */
		void setKeepAspectRatio(bool ar);
		
		/** Sets whether it is keeping the aspect ratio (1:1 grid). */
		bool keepAspectRatio() const { return m_keepRatio; }
		
		/** Force the functions from @p start to @p end to be recalculated. */
		void updateFunctions( const QModelIndex & start, const QModelIndex& end );
		
		void setModel(FunctionsModel* f);
		FunctionsModel* model() const { return m_model; }
		
		int width() const { return m_size.width(); }
		int height() const { return m_size.height(); }
		
		static QRect toBiggerRect(const QRectF&);
		
		/** Sets the graph's viewport to @p v. */
		void setViewport(const QRectF& vp, bool repaint=true);
		
		/** Moves the viewport @p delta */
		void moveViewport(const QPoint& delta);
		
	protected:
		QRectF lastViewport() const { return viewport; }
		QRectF lastUserViewport() const { return userViewport; }
		void updateScale(bool repaint);
		
		QPointF toWidget(const QPointF &) const;
		QPointF fromWidget(const QPoint& p) const;
		QPointF toViewport(const QPoint& mv) const;
		QPointF calcImage(const QPointF& ndp) const;
		QLineF slope(const QPointF& dp) const;
		
		QLineF toWidget(const QLineF &) const;
		void setPaintedSize(const QSize& size);
		void scaleViewport(qreal s, const QPoint& center);

	private:
		void drawAxes(QPainter *f, Function::Axe a);
		void drawPolarAxes(QPainter *f);
		void drawCartesianAxes(QPainter *f);
		
		double rang_x, rang_y;
		bool m_squares;
		bool m_keepRatio;
		bool m_dirty;
		QRectF viewport;
		QRectF userViewport;
		QSizeF m_size;
		FunctionsModel* m_model;
		
		static const QColor m_axeColor;
		static const QColor m_axe2Color;
		static const QColor m_derivativeColor;
};

#endif // FUNCTIONSPAINTER_H
