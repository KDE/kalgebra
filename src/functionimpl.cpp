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

#include "functionimpl.h"
#include "variables.h"

#include <KLocale>
#include <KDebug>

FunctionImpl::FunctionImpl(const Expression& newFunc)
	: points(0), m_deriv(0), m_last_viewport(QRect()), m_last_resolution(0), m_last_max_res(0)
{
	func.setExpression(newFunc);
	Expression deriv=func.derivative();
	if(func.isCorrect())
		m_deriv = new Expression(deriv);
	else
		func.flushErrors();
}

FunctionImpl::FunctionImpl(const FunctionImpl& fi)
	: points(0), m_deriv(0), m_last_viewport(QRect()), m_last_resolution(0), m_last_max_res(0)
{
	if(fi.isCorrect()) {
		func.setExpression(fi.func.expression());
		if(fi.m_deriv)
			m_deriv = new Expression(*fi.m_deriv);
	}
}

FunctionImpl::~FunctionImpl()
{
	if(points)
		delete [] points;
	
	if(m_deriv)
		delete m_deriv;
}

// FunctionImpl FunctionImpl::operator=(const FunctionImpl& f)
// {
// 	if(&f!=this) {
// 		func=f.func;
// 		m_last_viewport=QRect();
// 		m_last_resolution=0;
// 		m_last_max_res=0;
// 		m_deriv = new Expression(f.m_deriv);
// 		points=0;
// 	}
// 	return *this;
// }

void FunctionX::updatePoints(const QRect& viewport, unsigned int max_res)
{
	if(viewport.top()==m_last_viewport.top() && viewport.bottom()==m_last_viewport.bottom() && max_res==m_last_max_res || max_res<=static_cast<unsigned int>(-viewport.height()))
		return;
	
	if(max_res!=m_last_max_res) {
		if(points!=0)
			delete [] points;
		points = new QPointF[max_res];
	}
	double t_lim=viewport.top()+1, b_lim=viewport.bottom()-1;
	
	unsigned int resolucio=0, width=static_cast<unsigned int>(-b_lim+t_lim);
	
	while(resolucio<max_res)
		resolucio+= width;
	resolucio -= width;
	
	double inv_res= (double) ( -b_lim+t_lim)/resolucio;
	register unsigned int i=0;
	
	/*if(viewport.height() == m_last_viewport.height()) { //Should port y=f(x) optimizations here
	int cacho = round(resolucio/(-b_lim+t_lim));
// 		qDebug("<%d>", cacho);
		
	if(viewport.top()>m_last_viewport.top()) { //if its gone down
	b_lim= m_last_viewport.top()-1;
	for(i=0;i<(viewport.height()-(viewport.height()+m_last_viewport.top()))*cacho;i++){
// 				qDebug("%d>=%d", resolucio, i+(m_last_viewport.top()-viewport.top())*cacho);
	points[i]=points[i+(m_last_viewport.top()-viewport.top())*cacho];
}
	i=0;
}
}*/
	
	double x, y=0.0;
	for(y=t_lim; y>=b_lim; y-=inv_res) {
		func.m_vars->modify("y", y);
		x = func.calculate().value();
		points[i++]=QPointF(x, y);
	}
	
	m_last_viewport=viewport;
	m_last_resolution=resolucio;
	m_last_max_res = max_res;
}

void FunctionPolar::updatePoints(const QRect& viewport, unsigned int max_res)
{
	Q_ASSERT(func.expression()->isCorrect());
	if(max_res==m_last_max_res && !m_last_viewport.isNull())
		return;
	unsigned int resolucio=max_res;
	double pi=2.*acos(0.);
	double r=0., th=0.;
	Expression *e = func.expression();
	Cn ulimit = e->uplimit(), dlimit=e->downlimit();
	
	if(!ulimit.isCorrect())
		ulimit = 2.*pi;
	
	if(!dlimit.isCorrect())
		dlimit = 0.;
	
	if(ulimit<dlimit) {
		qDebug() << "can't have uplimit<downlimit";
		return;
	}
	
	register unsigned int i=0;
	
	if(max_res!=m_last_max_res) {
		if(points!=0)
			delete [] points;
		points = new QPointF[max_res];
	}
	
	double inv_res= (double) (ulimit.value()-dlimit.value())/resolucio;
	func.m_vars->modify("q", 0.);
	Cn *varth = (Cn*) func.m_vars->value("q");
	
	for(th=dlimit.value(); th<ulimit.value() && i<max_res; th+=inv_res) {
		varth->setValue(th);
		r = func.calculate().value();
		
		points[i++]=fromPolar(r, th);
	}
	
	m_last_viewport=viewport;
	m_last_resolution=resolucio;
	m_last_max_res = max_res;
}

void FunctionY::updatePoints(const QRect& viewport, unsigned int max_res)
{
	if(viewport.left()==m_last_viewport.left() && viewport.right()==m_last_viewport.right() && max_res==m_last_max_res/* || max_res<=viewport.width()*/)
		return;
	
	if(max_res!=m_last_max_res) {
		if(points==0)
			delete [] points;
		points = new QPointF[max_res];
	}
	double l_lim=viewport.left()-1., r_lim=viewport.right()+1., x=0.;
	
	unsigned int resolucio=0, width=static_cast<unsigned int>(-l_lim+r_lim);
	while(resolucio<max_res)
		resolucio+=width;
	resolucio -= width;
		
	double inv_res= (double) (-l_lim+r_lim)/resolucio;
	register int i=0;
	
	if(viewport.width() == m_last_viewport.width()) { //Perhaps these optimizations could be removed now, calculator is fast enough
		int cacho = static_cast<int>(round(resolucio/(-l_lim+r_lim)));
		
		if(viewport.right()<m_last_viewport.right()) {
			r_lim= m_last_viewport.left();
			for(i=(viewport.width())*cacho; i>(m_last_viewport.right()-viewport.right());i--)
				points[i] = points[i+cacho*(viewport.right()-m_last_viewport.right())];
			i=0;
		}
		
		if(viewport.left()>m_last_viewport.left()) {
			l_lim= m_last_viewport.right();
			for(i=0;i<(viewport.width()-(viewport.left()-m_last_viewport.left()))*cacho;i++)
				points[i]=points[i+(viewport.left()-m_last_viewport.left())*cacho];
		}
	}
	
	func.m_vars->modify("x", 0.);
	Cn *vx = (Cn*) func.m_vars->value("x");
	
	for(x=l_lim; x<=r_lim; x+=inv_res) {
		vx->setValue(x);
		double y = func.calculate().value();
		points[i++]=QPointF(x, y);
	}
	
	m_last_viewport=viewport;
	m_last_resolution=resolucio;
	m_last_max_res = max_res;
}

QPair<QPointF, QString> FunctionX::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	func.m_vars->modify("y", dp.y());
	dp.setX(func.calculate().value());
	pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QPair<QPointF, QString> FunctionY::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	func.m_vars->modify(QString("x"), dp.x());
	dp.setY(func.calculate().value());
	pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

// static const double pi=2.*acos(0.);
static const double pi=acos(-1.);
QPair<QPointF, QString> FunctionPolar::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	if(p==QPointF(0., 0.))
		return QPair<QPointF, QString>(dp, i18n("center"));
	double th=atan(p.y()/p.x()), r=1., d, d2;
	if(p.x()<0.)	th += pi;
	else if(th<0.)	th += 2.*pi;
			
	Cn ulimit = func.expression()->uplimit(), dlimit=func.expression()->downlimit();
	if(!ulimit.isCorrect()) ulimit = 2.*pi;
	if(!dlimit.isCorrect()) dlimit = 0.;
			
			/*while(th<dlimit.value())
	th += 2.*pi;
			
	while(th>ulimit.value())
	th -= 2.*pi;*/
	QPointF dist;
	for(;;) {
		func.m_vars->modify("q", th);
		r = func.calculate().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d = sqrt(pow(dist.x(), 2.)+pow(dist.y(), 2.));
				
		func.m_vars->modify("q", th+2.*pi);
		r = func.calculate().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d2 = sqrt(pow(dist.x(), 2.)+pow(dist.y(), 2.));
		
		if(d<=d2)
			break;
		th += 2.*pi;
	}
	
	func.m_vars->modify("q", th);
	r = func.calculate().value();
	dp = fromPolar(r, th);
	pos = QString("r=%1 th=%2").arg(r,3,'f',2).arg(th,3,'f',2);
	
	dp = fromPolar(r, th);
	pos = QString("r=%1 th=%2").arg(r,3,'f',2).arg(th,3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QLineF slopeToLine(const double &der)
{
	double arcder = atan(der);
	const double len=3.*der;
	QPointF from, to;
	from.setX(len*cos(arcder));
	from.setY(len*sin(arcder));

	to.setX(-len*cos(arcder));
	to.setY(-len*sin(arcder));
	return QLineF(from, to);
}

QLineF mirrorXY(const QLineF& l)
{
	return QLineF(l.y1(), l.x1(), l.y2(), l.x2());
}

QLineF FunctionX::derivative(const QPointF& p) const
{
	Analitza a;
	Cn ret=0.;
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.m_vars->modify("y", p.y());
		
		if(a.isCorrect())
			ret = a.calculate();
	
		if(!a.isCorrect()) {
			kDebug(0) << "Derivative error: " <<  a.errors();
			return QLineF();
		}
	} else {
		QList<QPair<QString, double> > vars;
		vars.append(QPair<QString, double>("y", p.y()));
		a.setExpression(func.expression());
		ret=a.derivative(vars);
	}
	return mirrorXY(slopeToLine(ret.value()));
}

QLineF FunctionY::derivative(const QPointF& p) const
{
	Analitza a;
	Cn ret=0.;
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.m_vars->modify("x", p.x());
		
		if(a.isCorrect())
			ret = a.calculate();
	
		if(!a.isCorrect()) {
			kDebug(0) << "Derivative error: " <<  a.errors();
			return QLineF();
		}
	} else {
		QList<QPair<QString, double> > vars;
		vars.append(QPair<QString, double>("x", p.x()));
		a.setExpression(func.expression());
		ret=a.derivative(vars);
	}
	
	return slopeToLine(ret.value());
}

QLineF FunctionPolar::derivative(const QPointF& p) const
{
	Q_UNUSED(p);
	return QLineF();
}
