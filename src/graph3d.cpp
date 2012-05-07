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

#include <analitza/analyzer.h>
#include <analitza/value.h>
#include <analitza/variable.h>

using std::fabs;
using std::log10;

using namespace Analitza;

enum ActionsEnum {
    KEYRIGHT=1<<0, KEYLEFT=1<<1, KEYUP=1<<2, KEYDOWN=1<<3, KEYAVPAG=1<<4, KEYREPAG=1<<5, KEYS=1<<6, KEYW=1<<7,
    KEYQ=1<<8, KEYE=1<<9, LCLICK=1<<10, RCLICK=1<<11, MCLICK=1<<12 };

Graph3D::Graph3D(QWidget *parent) : QGLWidget(parent),
		default_step(0.15f), default_size(10.0f), zoom(1.0f), points(0), alpha(60.),
		method(Solid), trans(false), keyspressed(0), m_n(4)
{
	setFocusPolicy(Qt::ClickFocus);
	rotation[0] = 90.0;
	rotation[1] = 0.0;
	rotation[2] = 0.0;
}


Graph3D::~Graph3D()
{
	if(points) {
		const int j= 2*static_cast<int>(default_size/default_step);
		
		for(int i=0; i<j; i++)
			delete [] points[i];
		delete [] points;
	}
}

void Graph3D::initializeGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
// 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	
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
		rotation[0] += rel.y();
		rotation[2] -= rel.x();
		
		press = e->pos();
		repaint();
	}
}

void Graph3D::drawAxes()
{
	glColor3f(0.8, 0.8, 0.4);
	renderText(11.0, 0.0, 0.0, "Y");
	renderText(0.0, 11.0, 0.0, "X");
	renderText(0.0, 0.0,-11.0, "Z");
	
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
	
	if(keyspressed & KEYDOWN)	rotation[0]+=3.f;
	if(keyspressed & KEYUP)		rotation[0]-=3.f;
	if(keyspressed & KEYAVPAG)	rotation[1]+=3.f;
	if(keyspressed & KEYREPAG)	rotation[1]-=3.f;
	if(keyspressed & KEYLEFT)	rotation[2]+=3.f;
	if(keyspressed & KEYRIGHT)	rotation[2]-=3.f;
	if(keyspressed & KEYW)		alpha/=2.f;
	if(keyspressed & KEYS && alpha<=90.f)	alpha*=2.f;
	if(keyspressed & KEYQ)		{ zoom/=2.0f; create(a.expression()); }
	if(keyspressed & KEYE)		{ zoom*=2.0f; create(a.expression()); }
	
	if(rotation[0]>=360.f) rotation[0]-=360.f;
	if(rotation[1]>=360.f) rotation[1]-=360.f;
	if(rotation[2]>=360.f) rotation[2]-=360.f;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(alpha,(GLfloat)width()/(GLfloat)height(),0.1f,100.0f);
	
	float z=25.f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0,0,-z);
	glRotatef(rotation[0], 1.0, 0.0, 0.0);
	glRotatef(rotation[1], 0.0, 1.0, 0.0);
	glRotatef(rotation[2], 0.0, 0.0, 1.0);
	
	const double mida=default_size*zoom, step=default_step*zoom;
	uint bound=(2*mida/step)-1;
	drawAxes();
	
	if(!points)
		return;
	
	if(method==Dots) {
		glBegin(GL_POINTS);
		for(uint i=0; i<bound; i++) {
			double red=i*step/mida;
			double x=i*step-mida;
			for(uint j=0; j<bound; j++) {
				double curr=points[i][j];
				glColor3d(red, j*step/mida, curr/5);
				glVertex3d(x,  j*step-mida, curr);
			}
		}
		glEnd();
	} else if(method == Lines) {
		glBegin(GL_LINES);
		
		for(uint i=0; i<bound; i++) {
			double x=i*step-mida;
			double red=i*step/mida;
			int ii=i?i-1:i;
			
			for(uint j=0; j<bound; j++) {
				double y=j*step - mida;
				double curr=points[i][j];
				glColor3d( red, j*step/mida, curr/5);
				
				glVertex3d( x, y, curr);
				glVertex3d( ii*step-mida, y, points[ii][j]);
				
				int jj=j?j-1:j;
				glVertex3d( x, y, curr);
				glVertex3d( x, jj*step-mida, points[i][jj]);
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
		for(uint i=0; i<bound-1; i++) {
			glPopMatrix();
			
			glPushMatrix();
			glTranslatef(i*step-mida, -mida, 0);
			glBegin(GL_TRIANGLE_STRIP);
			
			double red=fabs((i*step-mida)/mida);
			double nextRed=fabs(((i+1)*step-mida)/mida);
			for(uint j=0; j<bound; j++) {
				float s=j%2 ? .0f : 1.f;
				double green=fabs((j*step-mida)/mida);
				
				double z=points[i][j];
				double blue=1./fabs(log10(10.+fabs(z)));

				glTexCoord2f(s, 0.f);
				glColor4d(red, green, blue, transf);
				glVertex3d(0., j*step, z);
				
				z=points[i+1][j];
				glTexCoord2f(s, 1.f);
				glColor4d(nextRed, green, blue, transf);
				glVertex3d(step, j*step, z);
			}
			glEnd();
		}
		glDisable(GL_TEXTURE_2D);
	}
	glPopMatrix();
	glFlush();
}

bool Graph3D::create(const Expression& func3d)
{
	double mida=default_size*zoom, step=default_step*zoom;
	const int k=static_cast<int>(mida/step)*2;
	
	int part = k/m_n;
	QList<Calculate3D*> threads;
	
	a.setExpression(func3d);
	
	QTime t;
	t.restart();
	
	for(int i=0; i<m_n; ++i) {
		Calculate3D *r = new Calculate3D(this, a, points, part*i, part*(i+1), mida, step);
		if(i+1==m_n)
			r->setTo(2*static_cast<int>(default_size/default_step));
		
		threads << r;
		r->start();
	}
	
	bool ret=true;
	QList<Calculate3D*>::const_iterator it = threads.constBegin();
	for(; it!=threads.constEnd(); ++it) {
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
	Q_ASSERT(a.isCorrect());
	Q_ASSERT(points);
	
	const int k= static_cast<int>(size/step)*2;
	
	Cn *x=new Cn;
	Cn *y=new Cn;
	
	QVector<Analitza::Object*> runStack;
	runStack.append(x);
	runStack.append(y);
	a.setStack(runStack);
	
	for(int j=from; j<to; j++) {
		y->setValue(j*step-size);
		for(int i=0; i<k; i++) {
			x->setValue(i*step-size);
			points[j][i] = -a.calculateLambda().toReal().value();
		}
	}
	qDeleteAll(runStack);
}

void Graph3D::wheelEvent(QWheelEvent * e)
{
	if(e->delta()>0)
		alpha/=2.f;
	else if(alpha<=90.f)
		alpha*=2.f;
	repaint();
}

void Graph3D::keyPressEvent( QKeyEvent *e )
{
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
	repaint();
}

void Graph3D::keyReleaseEvent( QKeyEvent *e )
{
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
	repaint();
}

void Graph3D::timeOut()
{
	rotation[0] += 20.0;
	rotation[1] += 20.0;
	rotation[2] += 20.0;
	repaint();
}

bool Graph3D::checkExpression(const QStringList& bvars, const Analitza::ExpressionType& actual)
{
	if(bvars!=(QStringList("x") << "y")) {
		return false;
	}
	
	static ExpressionType expected=ExpressionType(ExpressionType::Lambda)
									.addParameter(ExpressionType::Value)
									.addParameter(ExpressionType::Value)
									.addParameter(ExpressionType::Value);
	
	return actual.canReduceTo(expected);
}

void Graph3D::setFunc(const Expression& exp)
{
	if(exp.isCorrect()) {
		Analitza::Analyzer f3d;
		f3d.setExpression(exp);
		f3d.setExpression(f3d.dependenciesToLambda());
		f3d.simplify();
		
		if(!checkExpression(f3d.expression().bvarList(), f3d.type())) {
			sendStatus(i18n("Error: Wrong type of function"));
			return;
		}
		
		if(f3d.isCorrect()) {
			QTime t;
			t.restart();
			mem();
			create(f3d.expression());
			// xgettext: no-c-format
			sendStatus(i18nc("3D graph done in x milliseconds", "Done: %1ms", t.elapsed()));
		}
		else
			sendStatus(i18n("Error: %1", f3d.errors().join(", ")));
	} else
		sendStatus(i18n("Error: %1", exp.error().join(", ")));
}

void Graph3D::mem()
{
	const int j= 2*static_cast<int>(default_size/default_step);
	if(points!=0){
		for(int i=0; i<j; i++)
			delete [] points[i];
		delete [] points;
	}
	
	
	points = new double* [j];
	Q_CHECK_PTR(points);
	for(int i=0; i<j; i++){
		points[i] = new double[j];
		Q_CHECK_PTR(points[i]);
	}
	
	qDebug() << "Size: " << sizeof(double)*j*j;
}

void Graph3D::setSize(double newSize)
{
	default_size = newSize;
	repaint();
}

void Graph3D::setStep(double newRes)
{
	default_step = newRes;
	repaint();
}

void Graph3D::setZoom(float a)
{
	alpha = a;
	repaint();
}

void Graph3D::setMethod(enum Type m)
{
	method = m;
	repaint();
}

QPixmap Graph3D::toPixmap()
{
	return renderPixmap();
}

void Graph3D::resetView()
{
	default_step=0.15f;
	default_size=8.0f;
	if(zoom!=1.0f) {
		zoom=1.0f;
	}
	alpha=60.;
	
	rotation[0] = 90.0;
	rotation[1] = 0.0;
	rotation[2] = 0.0;
	updateGL();
}

#include "graph3d.moc"
