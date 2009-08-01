#include "functionimpl.h"
#include "functionfactory.h"
#include "value.h"
#include "variables.h"

#include <KDebug>
#include <KLocale>

struct FunctionX : public FunctionImpl
{
	explicit FunctionX(const Expression &e, Variables* v) : FunctionImpl(e, v) {}
	FunctionX(const FunctionX &fx) : FunctionImpl(fx) {}
	
	void updatePoints(const QRect& viewport, unsigned int resolution);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionX(*this); }
	
	static QStringList supportedBVars() { return QStringList("y"); }
};

struct FunctionY : public FunctionImpl
{
	explicit FunctionY(const Expression &e, Variables* v) : FunctionImpl(e, v) {}
	FunctionY(const FunctionY &fy) : FunctionImpl(fy) {}
	
	void updatePoints(const QRect& viewport, unsigned int resolution);
	QPair<QPointF, QString> calc(const QPointF& dp);
	QLineF derivative(const QPointF& p) const;
	virtual FunctionImpl* copy() { return new FunctionY(*this); }
	static QStringList supportedBVars() { return QStringList("x"); }
};

REGISTER_FUNCTION(FunctionY)
REGISTER_FUNCTION(FunctionX)

bool isSimilar(const double &a, const double &b, const double& diff=0.0001);

void FunctionX::updatePoints(const QRect& viewport, unsigned int max_res)
{
	double t_lim=viewport.top()+.1, b_lim=viewport.bottom()-.1;
	if(!points.isEmpty() && isSimilar(points.first().y(), t_lim) && isSimilar(points.last().y(), b_lim) &&
			int(max_res)==points.capacity())
		return;
	
	points.clear();
	points.reserve( max_res );
	func.variables()->modify("y", 0.);
	Cn *yval=(Cn*) func.variables()->value("y");
	
	double inv_res=double((-b_lim+t_lim)/max_res);
	for(double y=t_lim; y>b_lim; y-=inv_res) {
		yval->setValue(y);
		double x = func.calculate().toReal().value();
		
		addValue(QPointF(x, y));
	}
}


///	If there is a big difference between @p v1 and @p v2 and the sign is different,
///	@returns true

bool traverse(double v1, double v2)
{
// 	if(fabs(v1)>10 || fabs(v2)>10) qDebug() << "lolololo" << fabs(v1) << fabs(v2);
	return ((v1<0. && v2>0.) || (v2<0. && v1>0.)) && fabs(v1)>10 && fabs(v2)>10;
}

void FunctionY::updatePoints(const QRect& viewport, unsigned int max_res)
{
	double l_lim=viewport.left()-.1, r_lim=viewport.right()+.1;
	
	/*if(!points.isEmpty()) {
		qDebug() << isSimilar(points.first().x(), l_lim) << isSimilar(points.last().x(), r_lim) <<
			(int(max_res)==points.capacity());
		qDebug() << points.last().x() << r_lim;
	}*/
	
	//TODO: Check that the last value is the end of the viewport
	if(!func.isCorrect()) {
		qDebug() << "func error." << func.errors() << func.expression().toString();
		return;
	}
		
	if(!points.isEmpty() && isSimilar(points.first().x(), l_lim) && isSimilar(points.last().x(), r_lim) &&
			int(max_res)==points.capacity()) {
		return;
	}
	
	m_jumps.clear();
	points.clear();
	points.reserve(max_res);
	
	double step= double((-l_lim+r_lim)/max_res);
	
	func.variables()->modify("x", 0.);
	Cn *vx = (Cn*) func.variables()->value("x");
	
	for(double x=l_lim; x<r_lim-step; x+=step) {
		vx->setValue(x);
		Cn y = func.calculate().toReal();
		QPointF p(x, y.value());
		bool wasempty=points.isEmpty();
		bool ch=addValue(p);
		
		if(!wasempty && ch && (m_jumps.isEmpty() || m_jumps.last()!=points.count())) {
			double prevY=points[points.count()-2].y();
			if(y.format()!=Cn::Real && prevY!=y.value()) {
				m_jumps.append(points.count()-1);
			} else if(traverse(prevY, y.value())) {
				m_jumps.append(points.count()-1);
			}
		}
	}
// 	qDebug() << "end." << m_jumps;
}

QPair<QPointF, QString> FunctionX::calc(const QPointF& p)
{
	QPointF dp=p;
	func.variables()->modify("y", dp.y());
	if(!func.calculate().isReal())
		m_err += i18n("We can only draw Real results.");
	dp.setX(func.calculate().toReal().value());
	QString pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
	return QPair<QPointF, QString>(dp, pos);
}

QPair<QPointF, QString> FunctionY::calc(const QPointF& p)
{
	QPointF dp=p;
	QString pos;
	func.variables()->modify(QString("x"), dp.x());
	if(!func.calculate().isReal())
		m_err += i18n("We can only draw Real results.");
	
	dp.setY(func.calculate().toReal().value());
	pos = QString("x=%1 y=%2").arg(dp.x(),3,'f',2).arg(dp.y(),3,'f',2);
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
	Analitza a(func.variables());
	double ret;
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.variables()->modify("y", p.y());
		
		if(a.isCorrect())
			ret = a.calculate().toReal().value();
	
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
	Analitza a(func.variables());
	double ret;
	if(m_deriv) {
		a.setExpression(*m_deriv);
		a.variables()->modify("x", p.x());
		
		if(a.isCorrect())
			ret = a.calculate().toReal().value();
		
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
