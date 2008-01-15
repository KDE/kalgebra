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

#include "graph3d.h"

#if defined(Q_WS_MAC)
	#include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif

#include <KLocale>

#include <QImage>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QTime>

#include "analitza.h"
#include "exp.h"
#include "variables.h"

Graph3D::Graph3D(QWidget *parent) : QGLWidget(parent),
		default_step(0.15f), default_size(8.0f), zoom(1.0f), punts(NULL), alpha(60.),
		method(Solid), trans(false), keyspressed(0), m_n(4)
{
	this->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
	this->setFocusPolicy(Qt::ClickFocus);
	graus[0] = 90.0;
	graus[1] = 0.0;
	graus[2] = 0.0;
}


Graph3D::~Graph3D()
{
	if(punts) {
		const int j= 2*static_cast<int>(default_size/default_step);
		
		for(int i=0; i<j; i++)
			delete [] punts[i];
		delete [] punts;
	}
}

void Graph3D::initializeGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	QImage t, b;
	GLuint texture[1];
	b = QImage(8, 8, QImage::Format_Mono);
	b.fill(QColor(0xff, 0xff, 0xff).rgb());
	for(int i=0; i<b.height(); i++)
		b.setPixel(4, i, 0);
	
	for(int i=0; i<b.width(); i++)
		b.setPixel(i, 4, 0);
		
	t = QGLWidget::convertToGLFormat( b );
	glGenTextures( 1, &texture[0] );
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void Graph3D::resizeGL( int width, int height )
{
	height = height ? height : 1;
	
	glViewport( 0, 0, width, height );
}

void Graph3D::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton) {
		press = e->pos();
		keyspressed |= LCLICK;
	}
}

void Graph3D::mouseReleaseEvent(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton)
		keyspressed &= ~LCLICK;
}

void Graph3D::mouseMoveEvent(QMouseEvent *e)
{
	if(keyspressed & LCLICK){
		QPoint rel = e->pos() - press;
		graus[0] += rel.y();
		graus[2] -= rel.x();
		
		press = e->pos();
		this->repaint();
	}
}

void Graph3D::drawAxes()
{
	glColor3f(0.8, 0.8, 0.4);
	this->renderText(11.0, 0.0, 0.0, "X");
	this->renderText(0.0, 11.0, 0.0, "Y");
	this->renderText(0.0, 0.0,-11.0, "Z");
	
	glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(-10.0f, 0.0f, 0.0f);
			glVertex3f( 10.0f, 0.0f, 0.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f( 0.0f, 10.0f, 0.0f);
			glVertex3f( 0.0f,-10.0f, 0.0f);
		glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f( 0.0f, 0.0f, 10.0f);
			glVertex3f( 0.0f, 0.0f,-10.0f);
	glEnd();
}

void Graph3D::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(keyspressed & KEYDOWN)	graus[0]+=3.f;
	if(keyspressed & KEYUP)		graus[0]-=3.f;
	if(keyspressed & KEYAVPAG)	graus[1]+=3.f;
	if(keyspressed & KEYREPAG)	graus[1]-=3.f;
	if(keyspressed & KEYLEFT)	graus[2]+=3.f;
	if(keyspressed & KEYRIGHT)	graus[2]-=3.f;
	if(keyspressed & KEYW)		alpha/=2.f;
	if(keyspressed & KEYS && alpha<=90.f)	alpha*=2.f;
	if(keyspressed & KEYQ)		{ zoom/=2.0f; create(); }
	if(keyspressed & KEYE)		{ zoom*=2.0f; create(); }
	
	if(graus[0]>=360.f) graus[0]-=360.f;
	if(graus[1]>=360.f) graus[1]-=360.f;
	if(graus[2]>=360.f) graus[2]-=360.f;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(alpha,(GLfloat)width()/(GLfloat)height(),0.1f,100.0f);
	
	float z=25.f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0,0,-z);
	glRotatef(graus[0], 1.0, 0.0, 0.0);
	glRotatef(graus[1], 0.0, 1.0, 0.0);
	glRotatef(graus[2], 0.0, 0.0, 1.0);
	
	const double mida=default_size*zoom, step=default_step*zoom;
	unsigned int bound=(2*mida/step)-1;
	drawAxes();
	
	if(punts==NULL && !func3d.isCorrect())
		return;
	
	if(method==Dots) {
		glBegin(GL_POINTS);
		for(unsigned int i=0; i<bound; i++) {
			for(unsigned int j=0; j<bound; j++) {
				glColor3d( i*step/mida, j*step/mida, punts[i][j]/5);
				glVertex3d(i*step-mida, j*step-mida, punts[i][j]);
			}
		}
		glEnd();
	} else if(method == Lines) {
		glBegin(GL_LINES);
		
		for(unsigned int i=0; i<bound; i++) {
			for(unsigned int j=0; j<bound; j++) {
				glColor3d( i*step/mida, j*step/mida, punts[i][j]/5);
				
				glVertex3d( i*step-mida, j*step - mida, punts[i][j]);
				glVertex3d( (i?i-1:i)*step-mida, j*step - mida, punts[i?i-1:i][j]);
				
				glVertex3d( i*step-mida, j*step - mida, punts[i][j]);
				glVertex3d( i*step-mida, (j?j-1:j)*step - mida, punts[i][j?j-1:j]);
			}
		}
		glEnd();
	} else if(method == Solid) {
		if(trans){
			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		} else {
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		}
		
		glEnable(GL_TEXTURE_2D);
		double transf=0.8;
		
		glPushMatrix();
		for(unsigned int i=0; i<bound; i++) {
			glPopMatrix();
			
			glPushMatrix();
			glTranslatef(i*step-mida, -mida, 0);
			glBegin(GL_TRIANGLE_STRIP);
			for(unsigned int j=0; j<bound; j++) {
				glTexCoord2f(j%2 ? .0f : 1.f, 0.f);
				glColor4d((i*step-mida)/mida, (j*step-mida)/mida, 1./fabs(log10(5.+punts[i][j])), transf);
				glVertex3d(0., j*step, punts[i][j]);
				
				glTexCoord2f(j%2 ? .0f : 1.f, 1.f);
				glColor4d(((i+1)*step-mida)/mida, (j*step-mida)/mida, 1./fabs(log10(5.+punts[i+1][j])), transf);
				glVertex3d(step, j*step, punts[i+1][j]);
			}
			glEnd();
		}
		glDisable(GL_TEXTURE_2D);
	}
	glPopMatrix();
	glFlush();
}

bool Graph3D::create()
{
	double mida=default_size*zoom, step=default_step*zoom;
	const int k=static_cast<int>(mida/step)*2;
	
	int part = k/m_n;
	QList<Calculate3D*> threads;
	
	Analitza a;
	a.setExpression(func3d);
	
	QTime t;
	t.restart();
	
	for(int i=0; i<m_n; ++i) {
		Calculate3D *r = new Calculate3D(this, Analitza(a), punts, part*i, part*(i+1), mida, step);
		threads << r;
		r->start();
	}
	
	threads.last()->setTo(2*static_cast<int>(default_size/default_step));
	
	bool ret=true;
	QList<Calculate3D*>::iterator it = threads.begin();
	for(; it!=threads.end(); ++it) {
		if(!(*it)->wait(1000)) {
			ret=false;
			(*it)->terminate();
		}
	}
	
// 	qDebug() << "Elapsed time" << t.elapsed();
	
	qDeleteAll(threads);
	return ret;
}

void Calculate3D::run()
{
	Q_ASSERT(punts && a.m_vars);
	
	const int k= static_cast<int>(size/step)*2;
	a.m_vars->modify("x", 0.);
	a.m_vars->modify("y", 0.);
	
	Cn *x=(Cn*)a.m_vars->value("x"), *y=(Cn*)a.m_vars->value("y");
	
	for(int i=from; a.isCorrect() && i<to; i++) {
		x->setValue(i*step-size);
		for(int j=0; j<k; j++) {
			y->setValue(j*step-size);
			punts[i][j] = -a.calculate().value();
		}
	}
}

void Graph3D::keyPressEvent( QKeyEvent *e ) {
	switch(e->key()) {
		case Qt::Key_Up:
			keyspressed |= KEYUP;
			break;
		case Qt::Key_Down:
			keyspressed |= KEYDOWN;
			break;
		case Qt::Key_Left:
			keyspressed |= KEYLEFT;
			break;
		case Qt::Key_Right:
			keyspressed |= KEYRIGHT;
			break;
		case Qt::Key_PageUp:
			keyspressed |= KEYREPAG;
			break;
		case Qt::Key_PageDown:
			keyspressed |= KEYAVPAG;
			break;
		case Qt::Key_W:
			keyspressed |= KEYW;
			break;
		case Qt::Key_S:
			keyspressed |= KEYS;
			break;
		case Qt::Key_Q: //Be careful
			keyspressed |= KEYQ;
			break;
		case Qt::Key_E: //Be careful
			keyspressed |= KEYE;
			break;
	}
// 	sendStatus(QString("-%1-").arg(keyspressed, 16));
	this->repaint();
}

void Graph3D::keyReleaseEvent( QKeyEvent *e ){
	switch(e->key()) {
		case Qt::Key_Up:
			keyspressed &= ~KEYUP;
			break;
		case Qt::Key_Down:
			keyspressed &= ~KEYDOWN;
			break;
		case Qt::Key_Left:
			keyspressed &= ~KEYLEFT;
			break;
		case Qt::Key_Right:
			keyspressed &= ~KEYRIGHT;
			break;
		case Qt::Key_PageUp:
			keyspressed &= ~KEYREPAG;
			break;
		case Qt::Key_PageDown:
			keyspressed &= ~KEYAVPAG;
			break;
		case Qt::Key_W:
			keyspressed &= ~KEYW;
			break;
		case Qt::Key_S:
			keyspressed &= ~KEYS;
			break;
		case Qt::Key_Q: //Be careful
			keyspressed &= ~KEYQ;
			break;
		case Qt::Key_E: //Be careful
			keyspressed &= ~KEYE;
			break;
		
	}
// 	sendStatus(QString(".%1.").arg(keyspressed, 16));
	this->repaint();
}

void Graph3D::timeOut(){
	graus[0] += 20.0;
	graus[1] += 20.0;
	graus[2] += 20.0;
	this->repaint();
}

void Graph3D::setFunc(const Expression& exp) //FIXME: Must pass an Expression
{
	if(exp.isCorrect()) {
		func3d = exp;
		load();
	} else
		sendStatus(i18n("Error: %1", exp.error().join(", ")));
}

int Graph3D::load() 
{
	Analitza f3d;
	f3d.setExpression(func3d);
	f3d.m_vars->modify("x", 0.);
	f3d.m_vars->modify("y", 0.);
	f3d.calculate();
	
	if(f3d.isCorrect()) {
		QTime t;
		t.restart();
		sendStatus(i18n("Generating... Please wait"));
		mem();
		create();
		// xgettext: no-c-format
		sendStatus(i18n("Done: %1ms", t.elapsed()));
		this->repaint();
		return 0;
	} else {
		sendStatus(i18n("Error: %1", f3d.m_err.join(", ")));
		this->repaint();
		return -1;
	}
}

void Graph3D::mem()
{
	const int j= 2*static_cast<int>(default_size/default_step);
	if(punts!=0){
		for(int i=0; i<j; i++)
			delete [] punts[i];
		delete [] punts;
	}
	
	int midadelgrafo=0;
	punts = new double* [j];
	Q_CHECK_PTR(punts);
	for(int i=0; i<j; i++){
		midadelgrafo+=sizeof(double)*j;
		punts[i] = new double[j];
		Q_CHECK_PTR(punts[i]);
	}
	qDebug() << "Mida: " << midadelgrafo;
}

void Graph3D::setSize(double newSize)
{
	default_size = newSize;
	this->repaint();
}
void Graph3D::setStep(double newRes)
{
	default_step = newRes;
	this->repaint();
}

void Graph3D::setZoom(float a)
{
	alpha = a;
	this->repaint();
}

void Graph3D::setMethod(enum Type m)
{
	method = m;
	this->repaint();
}

QPixmap Graph3D::toPixmap()
{
	return this->renderPixmap();
}


void Graph3D::resetView()
{
	default_step=0.15f;
	default_size=8.0f;
	if(zoom!=1.0f) {
		zoom=1.0f;
		create();
	}
	alpha=60.;
	
	graus[0] = 90.0;
	graus[1] = 0.0;
	graus[2] = 0.0;
	updateGL();
}

#include "graph3d.moc"
