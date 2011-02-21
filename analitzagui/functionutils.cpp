/*************************************************************************************
 *  Copyright (C) 2011 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "functionutils.h"
#include <QLine>
#include <cmath>

using namespace std;

namespace FunctionUtils
{

bool traverse(double p1, double p2, double next)
{
	static const double delta=3;
	double diff=p2-p1, diff2=next-p2;
	bool ret = (diff>0 && diff2<-delta) || (diff<0 && diff2>delta);
	
	return ret;
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

}
