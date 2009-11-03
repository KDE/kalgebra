/*************************************************************************************
 *  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
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
#include "functionfactory.h"
#include "value.h"
#include "vector.h"

#include <KDebug>
#include <KLocale>
#include <container.h>

struct FunctionParametric : public FunctionImpl
{
	explicit FunctionParametric(const Expression &e, Variables* v) : FunctionImpl(e, v, 0,2*M_PI) {}
	FunctionParametric(const FunctionParametric &fx) : FunctionImpl(fx) {}
	
	void updatePoints(const QRect& viewport);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionParametric(*this); }
	
	static QStringList supportedBVars() { return QStringList("t"); }
	QStringList boundings() const { return supportedBVars(); }
};
REGISTER_FUNCTION(FunctionParametric)

void FunctionParametric::updatePoints(const QRect& viewport)
{
	Q_UNUSED(viewport);
	Q_ASSERT(func.expression().isCorrect());
	if(int(resolution())==points.capacity())
		return;
	
	double ulimit=uplimit();
	double dlimit=downlimit();
	
	if(ulimit<=dlimit) {
		kDebug() << "Cannot have downlimit ≥ uplimit";
		return;
	}
	
	points.clear();
	points.reserve(resolution());
	
	Cn *theT = func.insertValueVariable("t", 0.);
	
	double inv_res= double((ulimit-dlimit)/resolution());
	double final=ulimit-inv_res;
	for(double t=dlimit; t<final; t+=inv_res) {
		theT->setValue(t);
		Expression res=func.calculate();
		
		Object* vo=res.tree();
// 		objectWalker(vo);
		if(vo->type()==Object::vector) {
			Vector* v=static_cast<Vector*>(vo);
			bool valid=v->size()==2
				&& v->at(0)->type()==Object::value
				&& v->at(1)->type()==Object::value;
			
			if(valid) {
				Cn* x=static_cast<Cn*>(v->at(0));
				Cn* y=static_cast<Cn*>(v->at(1));
				addValue(QPointF(x->value(), y->value()));
			}
		}
	}
}

QPair<QPointF, QString> FunctionParametric::calc(const QPointF& p)
{
	func.insertValueVariable("t", 0.);
	Expression res=func.calculate();
	Object* vo=res.tree();
	
	if(func.isCorrect())
	{
		if(vo->type()!=Object::vector) {
			m_err += i18nc("if the specified function is not a vector",
						"The parametric function does not return a vector");
		} else {
			Vector* v=static_cast<Vector*>(vo);
			if(v->size()!=2)
				m_err += i18nc("If it is a vector but the wrong size. We work in R2 here",
							"A two-dimensional vector is needed");
			else if(v->at(0)->type()!=Object::value || v->at(1)->type()!=Object::value)
				m_err += i18nc("The vector has to be composed by integer members",
							"The parametric function items should be scalars");
		}
	}
	return QPair<QPointF, QString>(p, QString());
}

QLineF FunctionParametric::derivative(const QPointF& p) const
{
	return QLineF(p,p);
}
