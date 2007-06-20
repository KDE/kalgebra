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

#include "graph2d.h"

#include <QSvgGenerator>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QList>
#include <QPixmap>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFrame>
#include <QFile>

#include <klocale.h>
#include <cmath>

#include "analitza.h"

using namespace std;

QColor const Graph2D::m_axeColor(100,100,255);
QColor const Graph2D::m_axe2Color(235,235,235);

Graph2D::Graph2D(QWidget *parent) :
	QWidget(parent),
	mode(None), m_squares(true), resolucio(800), m_framed(false), m_readonly(false), m_posText("")
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setCursor(QCursor(Qt::CrossCursor));
	this->setMinimumHeight(20);
	this->setMinimumWidth(10);
	this->setMouseTracking(!m_readonly);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	valid=false;
	
	setViewport(QRectF(QPointF(-12., 10.), QSizeF(24., -20.)));
	defViewport = viewport;
	this->setAutoFillBackground(false);
}

Graph2D::~Graph2D() {
// 	funclist.clear();
}

QSizePolicy Graph2D::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void Graph2D::drawAxes(QPainter *f, Axe a)
{
	finestra.setRenderHint(QPainter::Antialiasing, false);
	
	switch(a) {
		case Polar:
			drawPolarAxes(f);
			break;
		default:
			drawCartesianAxes(f);
	}
	
	//write coords
	f->drawText(QPointF(3.+this->width()/2., 13.               ), QString::number(viewport.top()));
	f->drawText(QPointF(3.+this->width()/2., this->height()-5. ), QString::number(viewport.bottom()));
	f->drawText(QPointF(8.                 , this->height()/2.-5.), QString::number(viewport.left()));
	f->drawText(QPointF(this->width()-30.  , this->height()/2.-5.),QString::number(viewport.right()));
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
	
	finestra.setRenderHint(QPainter::Antialiasing, true);
	for(double i=thmin; i<thmax; i++) { //i is +
		QPointF p(toWidget(QPointF(i,i)));
		QPointF p2(toWidget(QPointF(-i,-i)));
		w->drawEllipse(QRectF(p.x(),p.y(), p2.x()-p.x(),p2.y()-p.y()));
	}
	finestra.setRenderHint(QPainter::Antialiasing, false);
	
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
	double x;
	
	ceixos.setColor(m_axe2Color);
	ceixos.setStyle(Qt::SolidLine);
	finestra->setPen(ceixos);

	for(x=ceil(viewport.left()); x<=viewport.right(); x+=1.0) {	// ralletes X
		p = toWidget(QPointF(x, 0.));
		if(m_squares)
			finestra->drawLine(QPointF(p.x(), this->height()), QPointF(p.x(), 0));
		else
			finestra->drawLine(p, p+QPointF(0.,-3.));
	}
	
	for(x=ceil(viewport.top()); x>=viewport.bottom(); x-=1.0) {		// ralletes y
		p = toWidget(QPointF(0., x));
		if(m_squares)
			finestra->drawLine(QPointF(0., p.y()), QPointF(width(), p.y()));
		else
			finestra->drawLine(p, p+QPointF(3.,0.));
	}
	
	ceixos.setColor(m_axeColor);
	ceixos.setStyle(Qt::SolidLine);
	finestra->setPen(ceixos);
	
	//dibuixa eixos viewport
	finestra->drawLine(QPointF(0., center.y()), QPointF(this->width(), center.y()));
	finestra->drawLine(QPointF(center.x(), 0.), QPointF(center.x(),this->height()));
	//EO dibuixa eixos viewport
	finestra->setRenderHint(QPainter::Antialiasing, true);
}

void Graph2D::pintafunc(QPaintDevice *qpd)
{
	QPointF ultim(0.,0.), act(0.,0.);
	QPen pfunc, ccursor;
	if(buffer.isNull() || buffer.width()!=width() || buffer.height()!=height())
		buffer = QPixmap(this->width(), this->height());
	buffer.fill(Qt::white);
	
	pfunc.setColor(QColor(0,150,0));
	pfunc.setWidth(2);
	
	finestra.begin(qpd);
//	finestra.initFrom(this);
	
	QRectF panorama(QPoint(0,0), size());
	finestra.setPen(pfunc);
	
	finestra.setRenderHint(QPainter::Antialiasing, true);
	
	Axe t = Cartesian;
	for (QList<function>::iterator it=funclist.begin(); it!=funclist.end(); ++it ){
		if(it->selected()) {
			t = it->axeType();
			break;
		}
	}
	
	drawAxes(&finestra, t);
	finestra.setRenderHint(QPainter::Antialiasing, true);
	
	if(!funclist.isEmpty()) {
		for (QList<function>::iterator it=funclist.begin(); it!=funclist.end(); ++it ){
			if(it->isShown()) {
				pfunc.setColor(it->color());
				pfunc.setWidth(it->selected()+1);
				finestra.setPen(pfunc);
				int i = it->npoints(), j;
				
				ultim=toWidget(it->points[0]);
				
				QPointF *vect=it->points;
				for(j=0; j<i; j++){
					act=toWidget(vect[j]);
					
					if(!isnan(act.y()) && !isnan(ultim.y()) && (panorama.contains(act) || panorama.contains(ultim)))
						finestra.drawLine(ultim, act);
					
					ultim=act;
				}
			}
		}
	}
	
	finestra.end();
	valid=true;
}

void Graph2D::paintEvent( QPaintEvent * )
{
	if(!valid) 
		pintafunc(&buffer);
	
	front = buffer;
	finestra.begin(&front);
	finestra.initFrom(this);
	QPen ccursor;
	QPointF ultim;
	
// 	finestra.setRenderHint(QPainter::Antialiasing, true);
	
	if(!m_readonly && mode==None) {
		ultim = toWidget(mark);
		
		//Draw derivative
		finestra.setRenderHint(QPainter::Antialiasing, true);
		QPointF from, to;
		ccursor.setColor(QColor(90,90,160));
		ccursor.setStyle(Qt::SolidLine);
		finestra.setPen(ccursor);
		double der = pendent(fromWidget(ultim)), arcder = atan(der);
		const double len=3.*der;
		from.setX(mark.x()+len*cos(arcder));
		from.setY(mark.y()+len*sin(arcder));
		
		to.setX(mark.x()-len*cos(arcder));
		to.setY(mark.y()-len*sin(arcder));
		if(!isnan(from.x()) && !isnan(from.y()) && !isnan(to.x()) && !isnan(to.y()))
			finestra.drawLine(toWidget(from), toWidget(to));
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
		
		finestra.setPen(QPen(QColor(0,0,0)));
		finestra.drawText(QPointF(ultim.x()+15., ultim.y()+15.), m_posText);
	} else if(!m_readonly && mode==Selection) {
		ccursor.setColor(QColor(0xc0,0,0));
		ccursor.setStyle(Qt::SolidLine);
		finestra.setPen(ccursor);
		finestra.setBrush(QColor(0xff,0xff, 0,0x90));
		finestra.drawRect(QRect(press,last));
	}
	
	if(m_framed) {
		QPen bord(Qt::black);
		finestra.setPen(bord);
		QPoint p2=QPoint(this->width(), this->height());
		finestra.drawRect(QRect(QPoint(0,0), p2-QPoint(2,2)));
	}
	finestra.end();
	
	///////////////////////////////
	QPainter win(this);
	win.drawPixmap(QPoint(0,0), front);
}

void Graph2D::wheelEvent(QWheelEvent *e){
	int d = e->delta()>0 ? -1 : 1;
	if(viewport.left()-d < 1 && viewport.top()+d > 1 && viewport.right()+d > 1 && viewport.bottom()-d < 1) {
		viewport.setLeft(viewport.left() - d);
		viewport.setTop(viewport.top() + d);
		viewport.setRight(viewport.right() + d);
		viewport.setBottom(viewport.bottom() - d);
		update_scale();
		update_points();
	}
	sendStatus(QString("(%1, %2)-(%3, %4)").arg(viewport.left()).arg(viewport.top()).arg(viewport.right()).arg(viewport.bottom()));
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

void Graph2D::mouseReleaseEvent(QMouseEvent *e){
	this->setCursor(QCursor(Qt::CrossCursor));
	if(!m_readonly && mode==Selection) {
		QPointF pd = toViewport(e->pos())-toViewport(press);
		const double mindist = fmin(fabs(pd.x()), fabs(pd.y())), rate=7.;
		const double minrate = fmin(fabs(viewport.width()/rate), fabs(viewport.height()/rate));
		
		if(mindist >= minrate) //if selection is not very small
			setViewport(QRectF(fromWidget(press), QSizeF(pd.x(), pd.y())));
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
		update_points();
	} else if(e->buttons()&Qt::LeftButton) {
		last = e->pos();
	} else if(e->buttons()==0)
		sendStatus(QString("x=%1 y=%2").arg(mark.x(),3,'f',2).arg(mark.y(),3,'f',2));
	
	this->repaint();
}

void Graph2D::keyPressEvent(QKeyEvent * e)
{
	const double xstep=viewport.width()/12., ystep=viewport.height()/10.;
	
	switch(e->key()) {
		case Qt::Key_Right:
			viewport.setLeft(viewport.left() +xstep);
			viewport.setRight(viewport.right() +xstep);
			break;
		case Qt::Key_Left:
			viewport.setLeft(viewport.left() -xstep);
			viewport.setRight(viewport.right() -xstep);
			break;
		case Qt::Key_Down:
			viewport.setTop(viewport.top() -ystep);
			viewport.setBottom(viewport.bottom() -ystep);
			break;
		case Qt::Key_Up:
			viewport.setTop(viewport.top() +ystep);
			viewport.setBottom(viewport.bottom() +ystep);
			break;
		case Qt::Key_Minus:
// 			resolucio=(resolucio*viewport.width())/(viewport.width()+2.);
			viewport.setCoords(viewport.left() -1., viewport.top() +1., viewport.right() + 1., viewport.bottom() -1.);
			update_scale();
			break;
		case Qt::Key_Plus:
			if(viewport.height() < -3. && viewport.width() > 3.){
				//FIXME:Bad solution
// 				resolucio=(resolucio*viewport.width())/(viewport.width()-2.);
				viewport.setCoords(viewport.left() + 1., viewport.top() -1., viewport.right() -1., viewport.bottom() +1.);
				update_scale();
			} else return;
			break;
		default:
			return;
	}
	valid=false;
	update_points();
	this->repaint();
}

QPointF Graph2D::calcImage(const QPointF& ndp)
{
	QPointF dp = ndp;
	m_posText="";
	if(!funclist.isEmpty()){
		for (QList<function>::iterator it = funclist.begin(); it != funclist.end(); ++it ){
			if(it->selected() && it->isShown()) {
				QPair<QPointF, QString> o = it->calc(dp);
				dp=o.first;
				m_posText = o.second;
				break;
			}
		}
	}
	return dp;
}

double Graph2D::pendent(const QPointF & dp) const
{
	double ret=0.;
	if(!funclist.isEmpty()){
		for (QList<function>::const_iterator it = funclist.begin(); it != funclist.end(); ++it ){
			if(it->selected() && it->isShown()) {
				ret = it->derivative(dp);
				break;
			}
		}
	}
	return ret;
}

void Graph2D::unselect(){
	if(!funclist.isEmpty()){
		for (QList<function>::iterator it = funclist.begin(); it != funclist.end(); ++it) {
			(*it).setSelected(false);
		}
	}
}

void Graph2D::update_points(){
	if(!funclist.isEmpty()) {
		for (QList<function>::iterator it = funclist.begin(); it != funclist.end(); ++it )
			(*it).update_points(toBiggerRect(viewport), static_cast<int>(floor(resolucio)));
		
		forceRepaint();
	}
}

bool Graph2D::addFunction(const function& func)
{
	bool exist=false;
	
	for (QList<function>::iterator it = funclist.begin(); !exist && it!=funclist.end(); ++it)
		exist = (it->name() == func.name());
	
	if(!exist) {
		funclist.append(func);
		sendStatus(i18n("%1 function added", func.func->expression()->toString()));
	}
	
	update_points();
	return exist;
}

bool Graph2D::editFunction(const QString& toChange, const function& func)
{
	bool exist=false;
	
	for (QList<function>::iterator it = funclist.begin(); !exist && it != funclist.end(); ++it ){
		if(it->name() == toChange){
			exist=true;
			*it = func;
			it->setName(toChange);
		}
	}
	
	if(exist) {
		update_points();
		this->repaint();
	}
	return exist;
}

function* Graph2D::editFunction(int num)
{
	return &funclist[num];
}

void Graph2D::editFunction(int num, const function& func)
{
	Q_ASSERT(num<funclist.count());
	funclist[num]=func;
	
	update_points();
	this->repaint();
}

bool Graph2D::setSelected(const QString& exp){
	for (QList<function>::iterator it = funclist.begin(); it != funclist.end(); ++it )
		(*it).setSelected((*it).name() == exp);
	
	update_points();
	this->repaint();
	return true;
}

bool Graph2D::setShown(const QString& f, bool shown)
{
	for (QList<function>::iterator it = funclist.begin(); it != funclist.end(); ++it ){
		if(it->name() == f) {
			it->setShown(shown);
			valid=false;
			this->repaint();
			return true;
		}
	}
	return false;
}

QPointF Graph2D::toWidget(const QPointF& p) const
{
	return QPointF((-viewport.left() + p.x()) * rang_x,  (-viewport.top() + p.y()) * rang_y);
}

QPointF Graph2D::fromWidget(const QPointF& p) const
{
	double part_negativa_x = -viewport.left();
	double part_negativa_y = -viewport.top();
	return QPointF(p.x()/rang_x-part_negativa_x, p.y()/rang_y-part_negativa_y);
}

QPointF Graph2D::toViewport(const QPoint &mv) const
{
	return QPointF(mv.x()/rang_x, mv.y()/rang_y);
}

void Graph2D::setResolution(int res)
{
	resolucio = res;
	update_points();
}

void Graph2D::setViewport(const QRectF &vp)
{
	viewport = vp;
	if(viewport.top()<viewport.bottom()) {
		double aux = viewport.bottom();
		viewport.setBottom(viewport.top());
		viewport.setTop(aux);
	}
	
	if(viewport.right()<viewport.left()) {
		double aux = viewport.left();
		viewport.setLeft(viewport.right());
		viewport.setRight(aux);
	}
	
	sendStatus(QString("(%1, %2)-(%3, %4)").arg(viewport.left()).arg(viewport.top()).arg(viewport.right()).arg(viewport.bottom()));
	update_scale();
}

void Graph2D::resizeEvent(QResizeEvent *)
{
	buffer=QPixmap(this->size());
	update_scale();
	repaint();
}

void Graph2D::clear()
{
	if(!funclist.isEmpty()) {
		funclist.clear();
// 		forceRepaint();
	}
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
//		gen.setResolution(100);
		pintafunc(&gen);
		b=true;
	} else if(!path.isEmpty() && path.endsWith(".png")) {
		this->repaint();
		b=buffer.save(path, "PNG");
	}
	
	return b;
}

void Graph2D::update_scale()
{
	rang_x= this->width()/viewport.width();
	rang_y= this->height()/viewport.height();
	valid=false;
	this->repaint();
}

#include "graph2d.moc"
