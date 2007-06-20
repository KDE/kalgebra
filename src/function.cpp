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

#include "function.h"
#include "exp.h"

#include <klocale.h>
#include "variables.h"
#include "analitza.h"
#include "expression.h"

function::function()
	: points(0), func(0), m_deriv(0), m_show(true), m_selected(false), m_last_max_res(0)
{}

function::function(const QString &name, const Expression& newFunc, const QColor& color=Qt::red, bool selec)
	: points(0), func(new Analitza), m_deriv(0), m_show(true), m_selected(selec), m_color(color), m_last_viewport(QRect()),
	       m_last_resolution(0), m_last_max_res(0), m_name(name)
{
	func->setExpression(newFunc);
}

function::function(const function& f)
	: points(0), func(new Analitza), m_deriv(0), m_show(f.m_show), m_selected(f.m_selected), m_color(f.m_color), m_last_viewport(QRect()),
		m_last_resolution(0), m_last_max_res(0), m_name(f.m_name)
{
	func->setExpression(*f.func->expression());
	m_deriv = new Expression(func->derivative());
}

function function::operator=(const function& f)
{
	if(&f!=this) {
		//Do I need to delete ptrs?
		points=0;
		func=new Analitza;
		func->setExpression(*f.func->expression());
		m_show=f.m_show;
		m_selected=f.m_selected;
		m_color=f.m_color;
		m_last_viewport=QRect();
		m_last_resolution=0;
		m_last_max_res=0;
		m_name=f.m_name;
		m_deriv = new Expression(func->derivative());
	}
	return *this;
}

function::~function()
{
	if(func!=0)
		delete func;
	
	if(points!=0)
		delete [] points;
	
	if(m_deriv!=0)
		delete m_deriv;
}

void function::update_points(const QRect& viewport, unsigned int max_res)
{
	Q_ASSERT(func);
	if(!m_show || !func->isCorrect())
		return;
	
	QStringList lambdas = func->bvarList();
	
	if(lambdas.count() <= 1){ //2D Graph only support 1 lambda, must recheck when add parametric functions
		m_firstlambda = lambdas.isEmpty()? "x" : lambdas[0];
		
		if(m_firstlambda=="x")
			update_pointsY(viewport, max_res);
		else if(m_firstlambda=="y")
			update_pointsX(viewport, max_res);
		else if(m_firstlambda=="q")
			update_pointsPolar(viewport, max_res);
		else
			err << i18n("Do not know how to represent f(%1)", m_firstlambda);
	} else
		err << i18n("Too much lambda for a 2D Graph");
}

void function::update_pointsY(const QRect& viewport, unsigned int max_res)
{
	if(viewport.left()==m_last_viewport.left() && viewport.right()==m_last_viewport.right() && max_res==m_last_max_res/* || max_res<=viewport.width()*/)
		return;
	
	if(max_res!=m_last_max_res) {
		if(points==0)
			delete [] points;
		points = new QPointF[max_res];
	}
	double l_lim=viewport.left()-1., r_lim=viewport.right()+1., x=0.;
	
	unsigned int resolucio=0, ample=static_cast<unsigned int>(-l_lim+r_lim);
	while(resolucio<max_res)
		resolucio+=ample;
	resolucio -= ample;
		
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
	
	func->m_vars->modify("x", new Cn(0.));
	Cn *vx = (Cn*) func->m_vars->value("x");
	
	for(x=l_lim; x<=r_lim; x+=inv_res) {
		vx->setValue(x);
		double y = func->calculate().value();
		points[i++]=QPointF(x, y);
	}
	
	m_last_viewport=viewport;
	m_last_resolution=resolucio;
	m_last_max_res = max_res;
}

void function::update_pointsX(const QRect& viewport, unsigned int max_res)
{
	if(viewport.top()==m_last_viewport.top() && viewport.bottom()==m_last_viewport.bottom() && max_res==m_last_max_res || max_res<=static_cast<unsigned int>(-viewport.height()))
		return;
	
	if(max_res!=m_last_max_res) {
		if(points!=0)
			delete [] points;
		points = new QPointF[max_res];
	}
	double t_lim=viewport.top()+1, b_lim=viewport.bottom()-1;
	
	unsigned int resolucio=0, ample=static_cast<unsigned int>(-b_lim+t_lim);
	
	while(resolucio<max_res)
		resolucio+= ample;
	resolucio -= ample;
	
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
		func->m_vars->modify("y", new Cn(y));
		x = func->calculate().value();
		points[i++]=QPointF(x, y);
	}
	
	m_last_viewport=viewport;
	m_last_resolution=resolucio;
	m_last_max_res = max_res;
}


void function::update_pointsPolar(const QRect& viewport, unsigned int max_res)
{
	Q_ASSERT(func->expression()->isCorrect());
	if(max_res==m_last_max_res && !m_last_viewport.isNull())
		return;
	unsigned int resolucio=max_res;
	double pi=2.*acos(0.);
	double r=0., th=0.;
	Expression *e = func->expression();
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
	func->m_vars->modify("q", 0.);
	Cn *varth = (Cn*) func->m_vars->value("q");
	
	for(th=dlimit.value(); th<ulimit.value() && i<max_res; th+=inv_res) {
		varth->setValue(th);
		r = func->calculate().value();
		
		points[i++]=fromPolar(r, th);
	}
	
	m_last_viewport=viewport;
	m_last_resolution=resolucio;
	m_last_max_res = max_res;
}

QPair<QPointF, QString> function::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	if(func->expression()->isCorrect()) {
		if(m_firstlambda=="y") {
			func->m_vars->modify("y", dp.y());
			dp.setX(func->calculate().value());
			pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
		} else if(m_firstlambda=="q") {
			double pi=2.*acos(0.);
			double th=atan(p.y()/p.x()), r=1., d, d2;
			if(p.x()<0.)	th += pi;
			else if(th<0.)	th += 2.*pi;
			
			Cn ulimit = func->expression()->uplimit(), dlimit=func->expression()->downlimit();
			if(!ulimit.isCorrect()) ulimit = 2.*pi;
			if(!dlimit.isCorrect()) dlimit = 0.;
			
			/*while(th<dlimit.value())
				th += 2.*pi;
			
			while(th>ulimit.value())
				th -= 2.*pi;*/
			QPointF dist;
			for(;;) {
				func->m_vars->modify("q", th);
				r = func->calculate().value();
				dp = fromPolar(r, th);
				dist = (dp-p);
				d = sqrt(pow(dist.x(), 2.)+pow(dist.y(), 2.));
				
				func->m_vars->modify("q", th+2.*pi);
				r = func->calculate().value();
				dp = fromPolar(r, th);
				dist = (dp-p);
				d2 = sqrt(pow(dist.x(), 2.)+pow(dist.y(), 2.));
				
				if(d<d2)
					break;
				th += 2.*pi;
			}
			
			func->m_vars->modify("q", th);
			r = func->calculate().value();
			dp = fromPolar(r, th);
			pos = QString("r=%1 th=%2").arg(r,3,'f',2).arg(th,3,'f',2);
		} else {
			func->m_vars->modify(QString("x"), dp.x());
			dp.setY(func->calculate().value());
			pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
		}
	}
	return QPair<QPointF, QString>(dp, pos);
}

Axe function::axeType() const
{
	if(m_firstlambda=="q")
		return Polar;
	else
		return Cartesian;
}

bool function::isShown() const
{
	return m_show && func->isCorrect();
}

double function::derivative(const QPointF & p) const
{
	Analitza a;
	Cn ret=0.;
	a.setExpression(*m_deriv);
	
	if(m_firstlambda=="x") {
		a.m_vars->modify("x", p.x());
	} else if(m_firstlambda=="y") {
		a.m_vars->modify("y", p.y());
	} else if(m_firstlambda=="q") {
		//TODO: Not yet implemented, copy from function::calc
	} else
		qDebug() << "sacre bleu!";
	
	if(a.isCorrect())
		ret = a.calculate().value();
	
	if(a.isCorrect())
		return ret.value();
	else {
		kDebug(0) << "Derivative error: " <<  a.errors() << endl;
		return 0.; //FIXME:Must improve that
	}
}
