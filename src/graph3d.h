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

#ifndef GRAPH3D_H
#define GRAPH3D_H

#include <QGLWidget>
#include <QtCore/QThread>

#include "analitza/analyzer.h"

/**
 *	Used to calculate 3D Graphs in parallel.
 *	@author Aleix Pol i Gonzalez <aleixpol@kde.org>  
 */

class Calculate3D : public QThread
{
public:
	/** Constructor. Creates a Calculate3D thread.
		@param p	parent object.
		@param na	Analitza module used fot the calculations.
		@param poi	memory space where the results will be located.
		@param fr	Beginning of the calculated values.
		@param to	End of the calculated values.
		@param m	Distance of each dimension from the (0, 0) to be calculated.
		@param s	Step between each 2 values.
	*/
	Calculate3D(QObject *p, const Analitza::Analyzer &na, double** poi, int fr, int to, double m, double s) : 
		QThread(p), a(na), points(poi), from(fr), to(to), size(m), step(s) {}
	
	/** Runs the thread. */
	void run();
	
	/** Sets the end of the segment to calculate. */
	void setTo(int nto) { to = nto; }
private:
	Analitza::Analyzer a;
	double **points;
	int from;
	int to;
	
	double size;
	double step;
};

/**
 *	The Graph3D provides a 3D OpenGL graph representation.
 *	@author Aleix Pol i Gonzalez
 */

class Graph3D : public QGLWidget
{
	Q_OBJECT
	public:
		/** Defines how will be the graph representated. */
		enum Type {
			Dots=0,  ///< Dots will be drawn.
			Lines=1, ///< Lines will be drawn.
			Solid=2  ///< A solid graph will be drawn with a line texture.
		};
		
		/** Constructor. Creates a new Graph3D widget. */
		Graph3D(QWidget *parent = 0);
		
		/** Destructor. */
		~Graph3D();
		
		/** Sets @p exp as the function's expression. */
		void setFunc(const Analitza::Expression& exp);
		
		/** Toggles the transparency for Solid graphs. */
		void setTransparency(bool tr) { trans = tr; glDraw(); }
		
		/** Returns whether there is transparency. */
		bool transparency() const { return trans; }
		
		/** Returns the pixmap painting. */
		QPixmap toPixmap();
		
		/** Sets the @p max maximum size. */
		void setSize(double max);
		
		/** Sets the interval between two points. */
		void setStep(double res);
		
		/** Sets the Z coordinate. */
		void setZoom(float alpha);
		
		/** Sets the showed method. */
		void setMethod(Type m);
		
		void wheelEvent(QWheelEvent *e);
		
		bool checkExpression(const Analitza::Expression& exp, const Analitza::ExpressionType& type);
		
	public slots:
		/** Resets the view coordinates. */
		void resetView();
		
	signals:
		/** Emits a status message. */
		void status(const QString &msg);
		
	private:
		void drawAxes();
		
		virtual void initializeGL() ;
		virtual void resizeGL( int width, int height ) ;
		virtual void paintGL() ;
		
		void keyPressEvent(QKeyEvent *e);
		void keyReleaseEvent(QKeyEvent *e);
		void timeOut();
		void mousePressEvent(QMouseEvent *e); QPoint press;
		void mouseReleaseEvent(QMouseEvent *e);
		void mouseMoveEvent(QMouseEvent *e);
		void mem();
		bool create(const Analitza::Expression& func3d);
		void sendStatus(const QString& msg) { emit status(msg); }
		
		Analitza::Analyzer a;
		double default_step;
		double default_size;
		double zoom;
		double **points;
		float rotation[3];
		float alpha;
		Type method;
		bool trans;
		unsigned short keyspressed;
		
		int m_n;
};

#endif
