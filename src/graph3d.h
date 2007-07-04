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

#ifndef Q3DGRAPH_H
#define Q3DGRAPH_H

#include <QtOpenGl/QGLWidget>
#include <QtCore/QThread>

#include "analitza.h"

#define KEYRIGHT	1<<0
#define KEYLEFT		1<<1
#define KEYUP		1<<2
#define KEYDOWN		1<<3
#define KEYAVPAG	1<<4
#define KEYREPAG	1<<5
#define KEYS		1<<6
#define KEYW		1<<7
#define KEYQ		1<<8
#define KEYE		1<<9
#define LCLICK		1<<10
#define RCLICK		1<<11
#define MCLICK		1<<12


/**
 *	Used to calculate 3D Graphs in parallel.
 *	@author Aleix Pol i Gonzalez<aleixpol@gmail.com>
 */

class Calculate3D : public QThread
{
public:
	/** Constructor. Creates a Calculate3D thread.
		@param p	parent object.
		@param na	Analitza module used fot the calculations.
		@param poi	memory space where the results will be located.
		@param fr	Begining of the calculated values.
		@param to	End of the calculated values.
		@param m	Distance of each dimension from the (0, 0) to be calculated.
		@param s	Step between each 2 values.
	*/
	Calculate3D(QObject *p, const Analitza &na, double** poi, int fr, int to, double m, double s) : 
		QThread(p), a(na), punts(poi), from(fr), to(to), size(m), step(s) {}
	
	/** Runs the thread. */
	void run();
	
	/** Sets the end of the segment to calculate. */
	void setTo(int nto) { to = nto; }
private:
	Analitza a;
	double **punts;
	int from;
	int to;
	
	double size;
	double step;
};

/**
 *	The Graph3D provides a 3D OpenGL graph representation.
 *	@author Aleix Pol i Gonzalez
 */

class Graph3D : public QGLWidget {
	Q_OBJECT
	public:
		/** Defines how will be the graph representated. */
		enum Type {
			Dots=0,	/**< Dots will be drawn. */
			Lines=1,/**< Lines will be drawn. */
			Solid=2	/**< A solid graph will be drawn with a line texture. */
		};
		
		/** Constructor. Creates a new Graph3D widget. */
		Graph3D(QWidget *parent = 0);
		
		/** Destructor. */
		~Graph3D();
		
		/** Sets @p exp as the function's expression. */
		void setFunc(const Expression& exp);
		
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
		void setZ(float coord_z);
		
		/** Sets the showed method. */
		void setMethod(enum Type m);
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
		int load();
		void mem();
		bool create();
		void sendStatus(const QString& msg) { emit status(msg); }
		
		Expression func3d;
		double default_step;
		double default_size;
		double zoom;
		double **punts;
		float graus[3];
		float z;
		enum Type method;
		bool trans;
		bool tefunc;
		unsigned short keyspressed;
		
		int m_n;
};

#endif
