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

#include "function.h"

#include "variables.h"
#include "analitza.h"
#include "expression.h"
#include "functionimpl.h"
#include "functionfactory.h"

#include <KLocale>
#include <cmath>

function::function()
	: m_function(0), m_show(true), m_color(Qt::black)
{}

// #include "container.h"

function::function(const QString &name, const Expression& newFunc, Variables* v, const QColor& color)
	: m_function(0), m_show(true), m_color(color), m_name(name)
{
	if(newFunc.isCorrect()) {
//		objectWalker(newFunc.tree());
		QStringList bvars=newFunc.isLambda() ? newFunc.bvarList() : QStringList("x");
		if(!FunctionFactory::self()->contains(bvars))
			m_err << "Function type not recognized";
		else
			m_function=FunctionFactory::self()->item(bvars, newFunc, v);
	} else {
		m_err << "The expression is not correct";
	}
}

/*FunctionImpl* copy(FunctionImpl* fi)
{
	FunctionX *fx;
	FunctionY *fy;
	FunctionPolar *fp;
	if(fx=dynamic_cast<FunctionX*>(fi))
		return new FunctionX(*fx);
	else if(fy=dynamic_cast<FunctionY*>(fi))
		return new FunctionY(*fy);
	else if(fp=dynamic_cast<FunctionPolar*>(fi))
		return new FunctionPolar(*fp);
	else 
		return 0;
}*/

function::function(const function& f)
	: m_function(0), m_show(f.m_show), m_color(f.m_color), m_name(f.m_name), m_err(f.m_err)
{
	if(f.m_function)
		m_function=f.m_function->copy();
}

function::~function()
{
	if(m_function)
		delete m_function;
}

function function::operator=(const function& f)
{
	if(&f!=this) {
		if(m_function)
			delete m_function;
		
		if(f.m_function) {
			m_function=f.m_function->copy();
//	 		m_function=copy(f.m_function);
			Q_ASSERT(m_function);
		} else
			m_function=0;
		m_show=f.m_show;
		m_color=f.m_color;
		m_name=f.m_name;
		m_err=f.m_err;
	}
	return *this;
}

void function::update_points(const QRect& viewport, unsigned int max_res)
{
	Q_ASSERT(m_function);
	m_function->updatePoints(viewport, max_res);
}

function::Axe function::axeType() const
{
	return m_function->axeType();
}

bool function::isShown() const
{
	return m_show && m_function && m_function->isCorrect();
}

QLineF function::derivative(const QPointF & p) const
{
	Q_ASSERT(m_function);
	return m_function->derivative(p);
}

QStringList function::bvars() const
{
	return m_function->bvarList();
}

Analitza * function::analitza() const
{
	Q_ASSERT(m_function);
	return &m_function->func;
}

const QVector<QPointF>& function::points() const
{
	return m_function->points;
}

QPair< QPointF, QString > function::calc(const QPointF & dp)
{
	Q_ASSERT(m_function);
	return m_function->calc(dp);
}

bool function::isCorrect() const
{
	return m_function && m_err.isEmpty() && m_function->isCorrect();
}

QStringList function::errors() const
{
	QStringList err(m_err);
	if(m_function)
		err += m_function->m_err;
	return err;
}

QString function::toString() const
{
	Q_ASSERT(m_function);
	return m_function->toString();
}

QStringList function::supportedBoundedVars()
{
	QStringList ret;
	ret.append("x");
	ret.append("y");
	ret.append("q");
	return ret;
}

const Expression& function::expression() const
{
	return analitza()->expression();
}


QList<int> function::jumps() const
{
	return m_function->m_jumps;
}
