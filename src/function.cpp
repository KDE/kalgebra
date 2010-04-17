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
#include "analyzer.h"
#include "expression.h"
#include "functionimpl.h"
#include "functionfactory.h"

#include <KLocale>
#include <cmath>

function::function()
	: m_function(0), m_show(true), m_color(Qt::black)
{}

function::function(const QString &name, const Analitza::Expression& newFunc, Analitza::Variables* v,
				   const QColor& color, double uplimit, double downlimit)
	: m_function(0), m_expression(newFunc), m_show(true), m_color(color), m_name(name)
{
	if(newFunc.isCorrect()) {
		Analitza::Analyzer a(v);
		a.setExpression(newFunc);
		
		m_expression=a.dependenciesToLambda();
		
		QStringList bvars=m_expression.bvarList();
		
		//TODO: turn into assertion
		if(!FunctionFactory::self()->contains(bvars))                                        
			m_err << i18n("Function type not recognized");
		else {
			Analitza::ExpressionType t=a.type().returnValue();
			
			if(t.canReduceTo(FunctionFactory::self()->type(bvars))) {
				m_function=FunctionFactory::self()->item(bvars, m_expression, v);
				if(downlimit!=uplimit)
					m_function->setLimits(downlimit, uplimit);
			} else
				m_err << i18n("Function type not correct for functions depending on %1", bvars.join(i18n(", ")));
		}
	} else {
		m_err << i18n("The expression is not correct");
	}
}

function::function(const function& f)
	: m_function(0), m_expression(f.expression()), m_show(f.m_show), m_color(f.m_color)
	, m_name(f.m_name), m_err(f.m_err)
{
	if(f.m_function)
		m_function=f.m_function->copy();
}

function::~function()
{
	delete m_function;
}

function function::operator=(const function& f)
{
	if(&f!=this) {
		delete m_function;
		
		if(f.m_function) {
			m_function=f.m_function->copy();
//	 		m_function=copy(f.m_function);
			Q_ASSERT(m_function);
		} else
			m_function=0;
		m_expression=f.m_expression;
		m_show=f.m_show;
		m_color=f.m_color;
		m_name=f.m_name;
		m_err=f.m_err;
	}
	return *this;
}

void function::update_points(const QRect& viewport)
{
	Q_ASSERT(m_function);
	Q_ASSERT(resolution()>2);
	
	m_function->updatePoints(viewport);
	Q_ASSERT(m_function->points.size()>=2);
}

void function::setResolution(unsigned int resolution)
{
	Q_ASSERT(m_function);
	m_function->setResolution(resolution);
}

uint function::resolution() const
{
	return m_function->resolution();
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

const QVector<QPointF>& function::points() const
{
	Q_ASSERT(m_function);
	Q_ASSERT(m_function->points.size()>1);
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
	if(m_function) {
		err += m_function->m_err;
		err += m_function->func.errors();
	}
	return err;
}

const Analitza::Expression& function::expression() const
{
	return m_expression;
}

QList<int> function::jumps() const
{
	return m_function->m_jumps;
}

