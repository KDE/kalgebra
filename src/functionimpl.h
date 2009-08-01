/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
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

#ifndef FUNCTIONIMPL_H
#define FUNCTIONIMPL_H

#include "function.h"
#include "analitza.h"
#include <cmath>

class Expression;

struct FunctionImpl
{
	explicit FunctionImpl(const Expression& e, Variables* v);
	FunctionImpl(const FunctionImpl& fi);
	virtual ~FunctionImpl();
	QStringList bvarList() { return func.bvarList(); }
	bool isCorrect() const { return m_err.isEmpty() && func.isCorrect(); }
	QString toString() const { return func.expression().toString(); }
	bool addValue(const QPointF& p);
	
	virtual function::Axe axeType() const { return function::Cartesian; }
	virtual QPair<QPointF, QString> calc(const QPointF& dp)=0;
	virtual void updatePoints(const QRect& viewport, unsigned int resolution)=0;
	virtual QLineF derivative(const QPointF& p) const=0;
	virtual FunctionImpl* copy()=0;
	
	double uplimit(double defaultValue) const;
	double downlimit(double defaultValue) const;
	
	QVector<QPointF> points;
	Analitza func;
	Expression *m_deriv;
	QStringList m_err;
	QList<int> m_jumps;
};

#endif
