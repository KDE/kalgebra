#include "functionimpl.h"
#include "functionfactory.h"
#include "value.h"
#include "vector.h"
#include "variables.h"

#include <KDebug>
#include <KLocale>
#include <container.h>

struct FunctionParametric : public FunctionImpl
{
	explicit FunctionParametric(const Expression &e, Variables* v) : FunctionImpl(e, v) {}
	FunctionParametric(const FunctionParametric &fx) : FunctionImpl(fx) {}
	
	void updatePoints(const QRect& viewport, unsigned int resolution);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionParametric(*this); }
	
	static QStringList supportedBVars() { return QStringList("t"); }
};
REGISTER_FUNCTION(FunctionParametric)

void FunctionParametric::updatePoints(const QRect& viewport, unsigned int max_res)
{
	Q_UNUSED(viewport);
	Q_ASSERT(func.expression().isCorrect());
	if(int(max_res)==points.capacity())
		return;
	unsigned int resolution=max_res;
	
	double ulimit=uplimit(2*M_PI);
	double dlimit=downlimit(0);
	
	if(ulimit<=dlimit) {
		m_err += i18n("Cannot have downlimit ≥ uplimit");
		return;
	}
	
	points.clear();
	points.reserve(max_res);
	
	func.variables()->modify("t", 0.);
	Cn *theT = (Cn*) func.variables()->value("t");
	
	double inv_res= double((ulimit-dlimit)/resolution);
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
	func.variables()->modify("t", 0.);
	Expression res=func.calculate();
	Object* vo=res.tree();
	if(vo->type()!=Object::vector) {
		m_err += i18n("The parametric function does not return an vector");
	} else {
		Vector* v=static_cast<Vector*>(vo);
		if(v->size()!=2)
			m_err += i18n("The parametric function does not return an vector");
		else if(v->at(0)->type()!=Object::value || v->at(1)->type()!=Object::value)
			m_err += i18n("The parametric function items should be scalars");
	}
	return QPair<QPointF, QString>(p, QString());
}

QLineF FunctionParametric::derivative(const QPointF& p) const
{
	return QLineF();
}
