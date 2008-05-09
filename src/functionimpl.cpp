/*************************************************************************************
 *  Copyright (C) 2007-2008 by Aleix Pol <aleixpol@gmail.com>                        *
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
#include "expression.h"

#include <KLocale>
#include <KDebug>

FunctionImpl::FunctionImpl(const Expression& newFunc)
	: points(), m_deriv(0), m_last_viewport(QRect())
{
	func.setExpression(newFunc);
	if(func.isCorrect()) {
		Expression deriv = func.derivative();
		if(func.isCorrect())
			m_deriv = new Expression(deriv);
		else
			func.flushErrors();
		Q_ASSERT(func.isCorrect());
	} else
		func.flushErrors();
}

FunctionImpl::FunctionImpl(const FunctionImpl& fi)
	: points(), m_deriv(0), m_last_viewport(QRect())
{
	if(fi.isCorrect()) {
		func.setExpression(fi.func.expression());
		if(fi.m_deriv)
			m_deriv = new Expression(*fi.m_deriv);
	}
}

FunctionImpl::~FunctionImpl()
{
	points.clear();
	
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
	if(viewport.top()==m_last_viewport.top() && viewport.bottom()==m_last_viewport.bottom() && int(max_res)==points.capacity() 
		|| max_res<=static_cast<unsigned int>(-viewport.height()))
		return;
	
	points.reserve( max_res );
	func.variables()->modify("y", 0.);
	Cn *yval=(Cn*) func.variables()->value("y");
	
	double t_lim=viewport.top()+.1, b_lim=viewport.bottom()-.1;
	double inv_res=double((-b_lim+t_lim)/max_res);
	for(double y=t_lim; y>=b_lim; y-=inv_res) {
		yval->setValue(y);
		double x = func.calculate().value();
		points.append(QPointF(x, y));
	}
	
	npoints=points.count();
	m_last_viewport=viewport;
}

void FunctionPolar::updatePoints(const QRect& viewport, unsigned int max_res)
{
	Q_ASSERT(func.expression()->isCorrect());
	if(int(max_res)==points.capacity() && m_last_viewport==viewport)
		return;
	unsigned int resolucio=max_res;
	double pi=2.*acos(0.);
	const Expression *e = func.expression();
	Expression ulimitexp = e->uplimit(), dlimitexp=e->downlimit();
	double ulimit, dlimit;
	
	if(ulimitexp.isCorrect()) {
		Analitza u;
		u.setExpression(ulimitexp);
		ulimitexp=u.calculate();
	}
	if(ulimitexp.isCorrect() && ulimitexp.isValue())
		ulimit=ulimitexp.value();
	else
		ulimit = 2.*pi;
	
	if(dlimitexp.isCorrect()) {
		Analitza d;
		d.setExpression(dlimitexp);
		dlimitexp=d.calculate();
	}
	if(dlimitexp.isCorrect() && dlimitexp.isValue())
		dlimit=dlimitexp.value();
	else
		dlimit = 0.;
	
	if(ulimit<=dlimit) {
		qDebug() << i18n("Can't have uplimit <= downlimit");
		return;
	}
	points.reserve(max_res);
	
	func.variables()->modify("q", 0.);
	Cn *varth = (Cn*) func.variables()->value("q");
	
	double inv_res= double((ulimit-dlimit)/resolucio);
	double final=ulimit-inv_res;
	for(double th=dlimit; th<final; th+=inv_res) {
		varth->setValue(th);
		double r = func.calculate().value();
		
		points.append(fromPolar(r, th));
	}
	
	npoints=points.size();
	m_last_viewport=viewport;
}

void FunctionY::updatePoints(const QRect& viewport, unsigned int max_res)
{
	if(viewport.left()==m_last_viewport.left() && viewport.right()==m_last_viewport.right() && int(max_res)==points.capacity()/* || max_res<=viewport.width()*/)
		return;
	
	points.reserve(max_res);
	double l_lim=viewport.left()-1., r_lim=viewport.right()+1., x=0.;
	
	unsigned int width=static_cast<unsigned int>(-l_lim+r_lim);
	unsigned int resolucio=((max_res-1)/width)*width;
	double inv_res= (double) (-l_lim+r_lim)/resolucio;
	
	/*if(viewport.width() == m_last_viewport.width()) {
		//Perhaps these optimizations could be removed now, calculator is fast enough
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
	}*/
	
	func.variables()->modify("x", 0.);
	Cn *vx = (Cn*) func.variables()->value("x");
	int i=0;
	double middleX=viewport.left()+viewport.width()/2;
	for(x=l_lim; x<=r_lim; x+=inv_res) {
		vx->setValue(x);
		double y = func.calculate().value();
		
		if(i>=2 && !(i<3 && x>middleX) && points[i-1].y()==y && points[i-2].y()==y) {
			points.last().setX(x);
		} else {
			points.append(QPointF(x, y));
			i++;
		}
	}
	
	npoints=i;
	m_last_viewport=viewport;
}

QPair<QPointF, QString> FunctionX::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	func.variables()->modify("y", dp.y());
	dp.setX(func.calculate().value());
	pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QPair<QPointF, QString> FunctionY::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	func.variables()->modify(QString("x"), dp.x());
	dp.setY(func.calculate().value());
	pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

static const double pi=acos(-1.);
QPair<QPointF, QString> FunctionPolar::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	if(p==QPointF(0., 0.))
		return QPair<QPointF, QString>(dp, i18n("center"));
	double th=atan(p.y()/ p.x()), r=1., d, d2;
	if(p.x()<0.)	th += pi;
	else if(th<0.)	th += 2.*pi;
	
	const Expression *e = func.expression();
	Expression ulimitexp = e->uplimit(), dlimitexp=e->downlimit();
	Cn ulimit, dlimit;
	if(ulimitexp.isCorrect()) {
		Analitza u;
		u.setExpression(ulimitexp);
		ulimitexp=u.calculate();
	}
	if(!ulimitexp.isCorrect())
		ulimit = 2.*pi;
	
	if(dlimitexp.isCorrect()) {
		Analitza d;
		d.setExpression(dlimitexp);
		dlimitexp=d.calculate();
	}
	if(!dlimitexp.isCorrect())
		dlimit = 0.;
	
	if(th<dlimit.value()) th=dlimit.value();
	if(th>ulimit.value()) th=ulimit.value();
	
	QPointF dist;
	do {
		func.variables()->modify("q", th);
		r = func.calculate().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		func.variables()->modify("q", th+2.*pi);
		r = func.calculate().value();
		dp = fromPolar(r, th);
		dist = (dp-p);
		d2 = sqrt(dist.x()*dist.x() + dist.y()*dist.y());
		
		th += 2.*pi;
	} while(d>d2);
	
	func.variables()->modify("q", th);
	r = func.calculate().value();
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
	double ret;
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.variables()->modify("y", p.y());
		
		if(a.isCorrect())
			ret = a.calculate().value();
	
		if(!a.isCorrect()) {
			kDebug() << "Derivative error: " <<  a.errors();
			return QLineF();
		}
	} else {
		QList<QPair<QString, double> > vars;
		vars.append(QPair<QString, double>("y", p.y()));
		a.setExpression(func.expression());
		ret=a.derivative(vars);
	}
	return mirrorXY(slopeToLine(ret));
}

QLineF FunctionY::derivative(const QPointF& p) const
{
	Analitza a;
	double ret;
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.variables()->modify("x", p.x());
		
		if(a.isCorrect())
			ret = a.calculate().value();
		
		if(!a.isCorrect()) {
			kDebug() << "Derivative error: " <<  a.errors();
			return QLineF();
		}
	} else {
		QList<QPair<QString, double> > vars;
		vars.append(QPair<QString, double>("x", p.x()));
		a.setExpression(func.expression());
		ret=a.derivative(vars);
	}
	
	return slopeToLine(ret);
}

QLineF FunctionPolar::derivative(const QPointF& p) const
{
	Q_UNUSED(p);
	return QLineF();
}
