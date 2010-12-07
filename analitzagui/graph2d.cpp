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

#include "graph2d.h"

#include <QSvgGenerator>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QList>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFile>
#include <QApplication>
#include <cmath>

#include <KLocale>
#include <KDebug>

#include <analitza/analyzer.h>
#include "functionsmodel.h"

#if defined(HAVE_IEEEFP_H)
#include <ieeefp.h>
bool isinf(double x) { return !finite(x) && x==x; }
#endif

// #define DEBUG_GRAPH

QColor const Graph2D::m_axeColor(100,100,255);
QColor const Graph2D::m_axe2Color(235,235,235);
QColor const Graph2D::m_derivativeColor(90,90,160);

Graph2D::Graph2D(FunctionsModel* fm, QWidget *parent) :
	QWidget(parent), m_model(fm),
	valid(false), mode(None), m_squares(true), m_keepRatio(true),
	m_framed(false), m_readonly(false), m_posText()
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setCursor(Qt::CrossCursor);
	
	this->setMouseTracking(!m_readonly);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	setViewport(QRectF(QPointF(-12., 10.), QSizeF(24., -20.)));
	defViewport = userViewport;
	this->setAutoFillBackground(false);
	
	connect(m_model, SIGNAL(dataChanged( const QModelIndex&, const QModelIndex& )),
		this, SLOT(update(const QModelIndex&, const QModelIndex)));
	connect(m_model, SIGNAL( rowsInserted ( const QModelIndex &, int, int ) ),
		this, SLOT(addFuncs(const QModelIndex&, int, int)));
	connect(m_model, SIGNAL( rowsRemoved ( const QModelIndex &, int, int ) ),
		this, SLOT(removeFuncs(const QModelIndex&, int, int)));
}

Graph2D::~Graph2D() {}

void Graph2D::drawAxes(QPainter *f, Function::Axe a)
{
	f->setRenderHint(QPainter::Antialiasing, false);
	
	switch(a) {
		case Function::Polar:
			drawPolarAxes(f);
			break;
		default:
			drawCartesianAxes(f);
	}
	
	//write coords
	QString rightBound=QString::number(viewport.right());
	int width=f->fontMetrics().width(rightBound);
	
	f->drawText(QPointF(3.+this->width()/2., 13.                 ), QString::number(viewport.top()));
	f->drawText(QPointF(3.+this->width()/2., this->height()-5.   ), QString::number(viewport.bottom()));
	f->drawText(QPointF(8.                 , this->height()/2.-5.), QString::number(viewport.left()));
	f->drawText(QPointF(this->width()-width, this->height()/2.-5.), rightBound);
	//EO write coords
}

void Graph2D::drawPolarAxes(QPainter *w)
{
	QPen ceixos;
	ceixos.setColor(m_axeColor);
	w->setPen(ceixos);
	const QPointF center = toWidget(QPointF(0.,0.));
	bool zero=false;
	zero= center.x()>0. && center.y()>0. && center.x()<width() && center.y()<height();
	double thmin = zero ? 0. : fmin(floor(fmin(viewport.left(), viewport.bottom())), ceil(fmax(viewport.right(), viewport.top())));
	double thmax = ceil(fmax(
				sqrt(pow(viewport.topRight().x(), 2.) + pow(viewport.topRight().y(), 2.)),
				sqrt(pow(viewport.bottomLeft().x(), 2.)+ pow(viewport.bottomLeft().y(), 2.))
			    )
			);
		
	ceixos.setColor(m_axe2Color);
	ceixos.setStyle(Qt::SolidLine);
	w->setPen(ceixos);
	
	w->setRenderHint(QPainter::Antialiasing, true);
	for(double i=thmin; i<thmax; i++) { //i is +
		QPointF p(toWidget(QPointF(i,i)));
		QPointF p2(toWidget(QPointF(-i,-i)));
		w->drawEllipse(QRectF(p.x(),p.y(), p2.x()-p.x(),p2.y()-p.y()));
	}
	w->setRenderHint(QPainter::Antialiasing, false);
	
	ceixos.setColor(m_axeColor);
	ceixos.setStyle(Qt::SolidLine);
	w->setPen(ceixos);
	w->drawLine(QPointF(0., center.y()), QPointF(this->width(), center.y()));
	w->drawLine(QPointF(center.x(), 0.), QPointF(center.x(),this->height()));
}

void Graph2D::drawCartesianAxes(QPainter *finestra)
{
	QPen ceixos;
	const QPointF center = toWidget(QPointF(0.,0.));
	QPointF p;
	
	ceixos.setColor(m_axe2Color);
	ceixos.setStyle(Qt::SolidLine);
	finestra->setPen(ceixos);
	
	double xini=ceil(viewport.left()), inc=1.;
	double yini=ceil(viewport.top());
	
	if(viewport.width()>100.) { //Draw less lines on large viewports
		inc=10.;
		xini=floor(xini/10.)*10.;
		yini=floor(yini/10.)*10.;
	}
	
	for(double x=xini; x<=viewport.right(); x+=inc) {	// ticks X
		p = toWidget(QPointF(x, 0.));
		if(m_squares)
			finestra->drawLine(QPointF(p.x(), this->height()), QPointF(p.x(), 0.));
		else
			finestra->drawLine(p, p+QPointF(0.,-3.));
	}
	
	for(double y=yini; y>=viewport.bottom(); y-=inc) {		// ticks y
		p = toWidget(QPointF(0., y));
		if(m_squares)
			finestra->drawLine(QPointF(0., p.y()), QPointF(width(), p.y()));
		else
			finestra->drawLine(p, p+QPointF(3.,0.));
	}
	
	ceixos.setColor(m_axeColor);
	ceixos.setStyle(Qt::SolidLine);
	finestra->setPen(ceixos);
	
	QPointF Xright(this->width(), center.y());
	QPointF Ytop(center.x(), 0.);
	
	//draw viewport axes
	finestra->drawLine(QPointF(0., center.y()), Xright);
	finestra->drawLine(Ytop, QPointF(center.x(),this->height()));
	//EO draw viewport axes
	finestra->setRenderHint(QPainter::Antialiasing, true);
	finestra->setBrush(m_axeColor);
	
	const double width=15., height=4.;
	QPointF dpx(width, height);
	QPointF dpy(height, width);
	QRectF rectX(Xright+dpx, Xright-dpx);
	QRectF rectY(Ytop+dpy, Ytop-dpy);
	
	int startAngleX = 150*16;
	int startAngleY = 240*16;
	int spanAngle = 60*16;
	finestra->drawPie(rectX, startAngleX, spanAngle);
	finestra->drawPie(rectY, startAngleY, spanAngle);
}

void Graph2D::drawFunctions(QPaintDevice *qpd)
{
	QPalette p=qApp->palette();
	QPen pfunc;
	if(buffer.isNull() || buffer.width()!=width() || buffer.height()!=height())
		buffer = QPixmap(this->width(), this->height());
	buffer.fill(p.color(QPalette::Active, QPalette::Base));
	
	pfunc.setColor(QColor(0,150,0));
	pfunc.setWidth(2);
	
	QPainter finestra;
	finestra.begin(qpd);
//	finestra.initFrom(this);
	finestra.setPen(pfunc);
	
	Function::Axe t=Function::Cartesian;
	if(m_model->hasSelection())
		t=m_model->currentFunction().axeType();
	drawAxes(&finestra, t);
	finestra.setRenderHint(QPainter::Antialiasing, true);
	
	int k=0;
	FunctionsModel::const_iterator it=m_model->constBegin(), itEnd=m_model->constEnd();
	for (; it!=itEnd; ++it, ++k ) {
		if(!it->isShown())
			continue;
		pfunc.setColor(it->color());
		pfunc.setWidth(m_model->isSelected(k)+1);
		finestra.setPen(pfunc);
		
		const QVector<QPointF> &vect=it->points();
		QList<int> jumps=it->jumps();
		
		unsigned int pointsCount = vect.count();
		QPointF ultim(toWidget(vect[0]));
		
		bool allJumps = it->allDisconnected();
		int nextjump;
		if(allJumps)
			nextjump = 0;
		else
			nextjump = jumps.isEmpty() ? -1 : jumps.takeFirst();
        
#ifdef DEBUG_GRAPH
		qDebug() << "---------" << jumps.count()+1;
#endif
		for(unsigned int j=0; j<pointsCount; j++) {
			QPointF act=toWidget(vect.at(j));
			
// 			qDebug() << "xxxxx" << act << ultim << isnan(act.y()) << isnan(ultim.y());
			if(isinf(act.y()) && !isnan(act.y())) qDebug() << "trying to plot from a NaN value" << act << ultim;
			else if(isinf(act.y()) && isnan(act.y())) qDebug() << "trying to plot to a NaN value";
			
			bool bothinf=(isinf(ultim.y()) && isinf(act.y())) || (isinf(ultim.x()) && isinf(act.x()));
			if(!bothinf && !isnan(act.y()) && !isnan(ultim.y()) && nextjump!=int(j)) {
				if(isinf(ultim.y())) {
					if(act.y()<0) ultim.setY(0);
					if(act.y()>0) ultim.setY(qpd->height());
				}
// 				
				QPointF act2(act);
				if(isinf(act2.y())) {
					if(ultim.y()<0) act2.setY(0);
					if(ultim.y()>0) act2.setY(qpd->height());
				}
				
// 				qDebug() << "xxxxx" << act2 << ultim << isnan(act2.y()) << isnan(ultim.y());
				finestra.drawLine(ultim, act2);
				
#ifdef DEBUG_GRAPH
				QPen p(Qt::red);
				p.setWidth(3);
				finestra.setPen(p);
				finestra.drawPoint(ultim);
				finestra.setPen(pfunc);
#endif
            } else if(nextjump==int(j)) {
				do {
					if(nextjump!=int(j))
						finestra.drawPoint(act);
					
					if(allJumps)
						nextjump += 2;
					else
						nextjump = jumps.isEmpty() ? -1 : jumps.takeFirst();
					
				} while(!jumps.isEmpty() && jumps.first()==nextjump+1);

#ifdef DEBUG_GRAPH
				qDebug() << "jumpiiiiiing" << ultim << toWidget(vect.at(j));
				QPen p(Qt::blue);
				p.setWidth(2);
				finestra.setPen(p);
				finestra.drawLine(QLineF(QPointF(act.x(), height()/2-10), QPointF(act.x(), height()/2+10)));
				finestra.setPen(pfunc);
#endif
			}
			
			ultim=act;
		}
	}
	
	finestra.end();
	valid=true;
}

void Graph2D::paintEvent(QPaintEvent * )
{
	if(!valid)
		drawFunctions(&buffer);
	
	QPainter finestra(this);
	finestra.drawPixmap(0,0,width(),height(), buffer);
	QPen ccursor;
	QPointF ultim;
	
// 	finestra.setRenderHint(QPainter::Antialiasing, true);
	
	if(!m_readonly && mode==None) {
		ultim = toWidget(mark);
		
		//Draw derivative
		ccursor.setColor(m_derivativeColor);
		ccursor.setStyle(Qt::SolidLine);
		QLineF sl=slope(mark);
		sl.translate(mark);
		
		finestra.setPen(ccursor);
		finestra.setRenderHint(QPainter::Antialiasing, true);
		if(!sl.isNull() && !isnan(sl.length()))
			finestra.drawLine(toWidget(sl));
		finestra.setRenderHint(QPainter::Antialiasing, false);
		//EOderivative
		
		ccursor.setColor(QColor(0xc0,0,0));
		ccursor.setStyle(Qt::SolidLine);
		
		finestra.setPen(ccursor);
		finestra.drawLine(QPointF(0.,ultim.y()), QPointF(this->width(), ultim.y()));
		finestra.drawLine(QPointF(ultim.x(),0.), QPointF(ultim.x(), this->height()));
		
		int w=finestra.fontMetrics().width(m_posText)+15, h=finestra.fontMetrics().height();
		
		if(ultim.x()+w > this->width())
			ultim.setX(this->width()-w);
		if(ultim.y()+h > this->height())
			ultim.setY(this->height()-h);
		if(ultim.y() < 0.)
			ultim.setY(0.);
		
		finestra.setPen(QPen(Qt::black));
		finestra.drawText(QPointF(ultim.x()+15., ultim.y()+15.), m_posText);
	} else if(!m_readonly && mode==Selection) {
		ccursor.setColor(QColor(0xc0,0,0));
		ccursor.setStyle(Qt::SolidLine);
		finestra.setPen(ccursor);
		finestra.setBrush(QColor(0xff,0xff, 0,0x90));
		finestra.drawRect(QRect(press,last));
	}
	
	if(m_framed) {
		QPoint p2=QPoint(this->width(), this->height());
		QPen bord(Qt::black);
		finestra.setPen(bord);
		finestra.drawRect(QRect(QPoint(0,0), p2-QPoint(2,2)));
	}
	finestra.end();
	
// 	qDebug() << "xxx2 " << viewport;
}

void Graph2D::wheelEvent(QWheelEvent *e)
{
	int d = e->delta()>0 ? -1 : 1;
	
	if(d>0 || (viewport.width()+d > 2 && viewport.height()+d < 2)) {
		QPointF p = fromWidget(e->pos());
		QRectF nv;
		nv.setLeft(viewport.left() - d);
		nv.setTop(viewport.top() + d);
		nv.setRight(viewport.right() + d);
		nv.setBottom(viewport.bottom() - d);
		setViewport(nv, false);
		
		QPointF p2 = p-fromWidget(e->pos());
		nv.translate(p2);		
		setViewport(nv);
	}
}

void Graph2D::mousePressEvent(QMouseEvent *e){
// 	qDebug("%d", toViewport(e->pos()).x());
	if(!m_readonly && (e->button()==Qt::LeftButton || e->button()==Qt::MidButton)) {
		last = press = e->pos();
		ant = toViewport(e->pos());
		this->setCursor(QCursor(Qt::PointingHandCursor));
		if(e->button()==Qt::MidButton || (e->button()==Qt::LeftButton && e->modifiers()&Qt::ControlModifier))
			mode=Pan;
		else if(e->button()==Qt::LeftButton)
			mode=Selection;
		
	}
}

void Graph2D::mouseReleaseEvent(QMouseEvent *e)
{
	if(m_readonly)
		this->setCursor(Qt::ArrowCursor);
	else
		this->setCursor(Qt::CrossCursor);
	
	if(!m_readonly && mode==Selection) {
		QPointF pd = toViewport(e->pos())-toViewport(press);
		
		QPoint pd2 = e->pos()-press;
		QRect rr(press, QSize(pd2.x(), pd2.y()));
		
		if(rr.width()>20 && rr.height()>20) { //if selection is not very small
			QRectF r(fromWidget(press), QSizeF(pd.x(), pd.y()));
			
			if(r.top()<r.bottom()) {
				double aux = r.bottom();
				r.setBottom(r.top());
				r.setTop(aux);
			}
			
			if(r.right()<r.left()) {
				double aux = r.left();
				r.setLeft(r.right());
				r.setRight(aux);
			}
			
			setViewport(r);
		} else
			sendStatus(i18n("Selected viewport too small"));
	}
	mode = None;
	this->repaint();
}

void Graph2D::mouseMoveEvent(QMouseEvent *e)
{
	mark=calcImage(fromWidget(e->pos()));
	
	if(!m_readonly && mode==Pan && ant != toViewport(e->pos())){
		QPointF rel = toViewport(e->pos() - press);
		viewport.moveLeft(viewport.left() - rel.x());
		viewport.moveTop(viewport.top() - rel.y());
		setViewport(viewport);
		
		press = e->pos();
	} else if(e->buttons()&Qt::LeftButton) {
		last = e->pos();
	} else if(e->buttons()==0)
		sendStatus(QString("x=%1 y=%2").arg(mark.x(),3,'g',5).arg(mark.y(),3,'g',5));
	
	this->repaint();
}

void Graph2D::keyPressEvent(QKeyEvent * e)
{
	const double xstep=viewport.width()/12., ystep=viewport.height()/10.;
	
	switch(e->key()) {
		case Qt::Key_Right:
			setViewport(userViewport.translated(xstep, 0));
			break;
		case Qt::Key_Left:
			setViewport(userViewport.translated(-xstep, 0));
			break;
		case Qt::Key_Down:
			setViewport(userViewport.translated(0, -ystep));
			break;
		case Qt::Key_Up:
			setViewport(userViewport.translated(0, ystep));
			break;
		case Qt::Key_Minus:
			zoomOut();
			break;
		case Qt::Key_Plus:
			zoomIn();
			break;
		default:
			return;
	}
}

QPointF Graph2D::calcImage(const QPointF& ndp)
{
	return m_model->calcImage(ndp).first;
}

QLineF Graph2D::slope(const QPointF & dp) const
{
	return m_model->slope(dp);
}

void Graph2D::unselect()
{
	m_model->unselect();
}

QLineF Graph2D::toWidget(const QLineF &f) const
{
	return QLineF(toWidget(f.p1()), toWidget(f.p2()));
}

QPointF Graph2D::toWidget(const QPointF& p) const
{
	double left=-viewport.left(), top=-viewport.top();
	return QPointF((left + p.x()) * rang_x,  (top + p.y()) * rang_y);
}

QPointF Graph2D::fromWidget(const QPoint& p) const
{
	double part_negativa_x = -viewport.left();
	double part_negativa_y = -viewport.top();
	return QPointF(p.x()/rang_x-part_negativa_x, p.y()/rang_y-part_negativa_y);
}

QPointF Graph2D::toViewport(const QPoint &mv) const
{
	return QPointF(mv.x()/rang_x, mv.y()/rang_y);
}

void Graph2D::setViewport(const QRectF& vp, bool repaint)
{
	userViewport = vp;
	Q_ASSERT(userViewport.top()>userViewport.bottom());
	Q_ASSERT(userViewport.right()>userViewport.left());
	
	sendStatus(QString("(%1, %2)-(%3, %4)")
			.arg(viewport.left()).arg(viewport.top()).arg(viewport.right()).arg(viewport.bottom()));
	updateScale(repaint);
	
	emit viewportChanged(userViewport);
}

void Graph2D::resizeEvent(QResizeEvent *)
{
	buffer=QPixmap(this->size());
	updateScale();
}

QRect Graph2D::toBiggerRect(const QRectF& ent)
{
	QRect ret;
	ret.setTop(static_cast<int>(ceil(ent.top())));
	ret.setBottom(static_cast<int>(floor(ent.bottom())));
	ret.setLeft(static_cast<int>(floor(ent.left())));
	ret.setRight(static_cast<int>(ceil(ent.right())));
	
	return ret;
}

bool Graph2D::toImage(const QString &path)
{
	bool b=false;
	
	if(!path.isEmpty() && path.endsWith(".svg")) {
		QFile f(path);
		QSvgGenerator gen;
		gen.setOutputDevice(&f);
		gen.setSize(this->size());
		drawFunctions(&gen);
		b=true;
		forceRepaint();
	} else if(!path.isEmpty() && path.endsWith(".png")) {
		this->repaint();
		b=buffer.save(path, "PNG");
	}
	
	return b;
}

void Graph2D::updateScale(bool repaint)
{
	viewport=userViewport;
	rang_x= width()/viewport.width();
	rang_y= height()/viewport.height();
	
	if(m_keepRatio && rang_x!=rang_y)
	{
		rang_y=rang_x=qMax(fabs(rang_x), fabs(rang_y));
		if(rang_y>0.) rang_y=-rang_y;
		if(rang_x<0.) rang_x=-rang_x;
		
		double newW=width()/rang_x, newH=height()/rang_x;
		
		double mx=(userViewport.width()-newW)/2.;
		double my=(userViewport.height()-newH)/2.;
		
		viewport.setLeft(userViewport.left()+mx);
		viewport.setTop(userViewport.bottom()-my);
		viewport.setWidth(newW);
		viewport.setHeight(-newH);
		//Commented because precision could make the program crash
// 		Q_ASSERT(userViewport.center() == viewport.center());
	}
	
	if(repaint) {
		if(m_model->rowCount())
			update(m_model->index(0,0), m_model->index(m_model->rowCount()-1,0));
		forceRepaint();
	}
}

void Graph2D::zoomIn()
{
	if(userViewport.height() < -3. && userViewport.width() > 3.){
		setViewport(QRect(userViewport.left() +1., userViewport.top() -1.,
					userViewport.width() -2., userViewport.height() +2.));
	}
}

void Graph2D::zoomOut()
{
	//FIXME:Bad solution
	//resolucio=(resolucio*viewport.width())/(viewport.width()+2.);
	setViewport(QRect(userViewport.left() -1., userViewport.top() +1.,
				userViewport.width() +2., userViewport.height() -2.));
}

void Graph2D::update(const QModelIndex & startIdx, const QModelIndex & endIdx)
{
	Q_ASSERT(startIdx.isValid() && endIdx.isValid());
	int start=startIdx.row(), end=endIdx.row();
	
	for(int i=start; i<=end; i++) {
		m_model->updatePoints(i, toBiggerRect(viewport));
	}
	valid=false;
	repaint();
}

void Graph2D::addFuncs(const QModelIndex & parent, int start, int end)
{
	Q_ASSERT(!parent.isValid());
	
	for(int i=start; i<=end; i++) {
		m_model->updatePoints(i, toBiggerRect(viewport));
	}
	valid=false;
	repaint();
}

void Graph2D::removeFuncs(const QModelIndex & parent, int start, int end)
{
	Q_UNUSED(parent)
	Q_UNUSED(start)
	Q_UNUSED(end)
	
	valid=false;
	repaint();
}

void Graph2D::setKeepAspectRatio(bool ar)
{
	m_keepRatio=ar;
	updateScale();
}

void Graph2D::setReadOnly(bool ro)
{
	m_readonly=ro;
	this->setCursor(ro ? Qt::ArrowCursor : Qt::CrossCursor);
	setMouseTracking(!ro);
}
void Graph2D::setModel(FunctionsModel* f)
{
	m_model=f;
}

QRectF Graph2D::definedViewport() const
{
	return userViewport;
}

#include "graph2d.moc"
