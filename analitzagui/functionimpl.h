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
#include "analitza/analyzer.h"
#include <cmath>

namespace Analitza { class Expression; }

struct FunctionImpl
{
	explicit FunctionImpl(const Analitza::Expression& e, Analitza::Variables* v, double defDl, double defUl);
	FunctionImpl(const FunctionImpl& fi);
	virtual ~FunctionImpl();
	bool isCorrect() const { return m_err.isEmpty() && func.isCorrect(); }
	
	uint resolution() const { return m_res; }
	void setResolution(uint res);
	
	//helpers
	/** adds a value to the points vector is needed. @returns if it was added. */
	bool addValue(const QPointF& p);
	static bool isSimilar(double a, double b, double diff=0.0001);
	
	//To reimplement
	virtual function::Axe axeType() const { return function::Cartesian; }
	virtual QPair<QPointF, QString> calc(const QPointF& dp)=0;
	virtual void updatePoints(const QRect& viewport)=0;
	virtual FunctionImpl* copy()=0;
    virtual QLineF derivative(const QPointF& p)=0;
	virtual QStringList boundings() const=0;
	virtual QString iconName() const=0;
	
	/** 
	 * @returns whether the points vector contains segments (all disconnected is true)
	 * or continuous lines (all disconnected is false).
	 */
	virtual bool allDisconnected() { return false; }
	
	double uplimit() const;
	double downlimit() const;
	void setLimits(double downlimit, double uplimit);
	
	QVector<QPointF> points;
	QList<int> m_jumps;
	Analitza::Analyzer func;
	Analitza::Expression *m_deriv;
	QStringList m_err;
	uint m_res;
	double mUplimit, mDownlimit;
};

#endif
