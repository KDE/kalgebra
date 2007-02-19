#include "function.h"
#include "exp.h"

function::function()
	: func(0), m_show(true), m_selected(false),m_last_max_res(0), points(0)
{}

function::function(const QString &name, const Expression& newFunc, const QColor& color=Qt::red, bool selec)
	: func(0), m_show(true), m_selected(selec), m_last_max_res(0), points(0)
{
	setFunction(name, newFunc, color, selec);
}


function::function(const function& f)
	: func(0), m_show(true), m_selected(false),m_last_max_res(0), points(0)
{
	setFunction(f.m_name, *f.func->expression(), f.color(), f.selected());
}

function function::operator=(const function& f)
{
	m_show=true; func=0; points=0; m_selected=f.selected(); m_last_max_res=0;
	
	setFunction(f.m_name, *f.func->expression(), f.color(), f.selected());
	return *this;
}

function::~function()
{
	if(func!=0)
		delete func;
	if(points!=0)
		delete [] points;
}

int function::setFunction(const QString& name, const Expression& newFunc, const QColor& color=Qt::red, bool selec)
{
	int ret;
	m_last_max_res=0;
	m_last_resolution=0;
	m_color = color;
	m_selected = selec;
	m_show=true;
	m_last_viewport=QRect();
	m_name=name;
	if(!func)
		func = new Analitza;
	func->setExpression(newFunc);
	
	if(points!=0) {
		delete [] points;
		points=0;
	}
	return ret;
}

void function::update_points(QRect viewport, unsigned int max_res)
{
	Q_ASSERT(func!=0);
	if(!m_show)
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
			err << i18n("Don't know how to represent f(%1)").arg(m_firstlambda);
	} else
		err << i18n("Too much lambda for a 2D Graph");
}

void function::update_pointsY(QRect viewport, unsigned int max_res)
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
	
	if(viewport.width() == m_last_viewport.width()) { //FIXME: resolucio diferent
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

void function::update_pointsX(QRect viewport, unsigned int max_res)
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


void function::update_pointsPolar(QRect viewport, unsigned int max_res)
{
	Q_ASSERT(func->expression()->isCorrect());
	if(max_res==m_last_max_res && !m_last_viewport.isNull())
		return;
	unsigned int resolucio=max_res;
	double pi=2.*acos(0.);
	double r=0., th=0.;
	Cn ulimit = func->expression()->uplimit(), dlimit=func->expression()->downlimit();
	
// 	objectWalker(func->tree());
	
	if(!ulimit.isCorrect())
		ulimit = 2.*pi;
	
	if(!dlimit.isCorrect())
		dlimit = 0.;
	
// 	qDebug() << "polar u:" << u25limit.value() << " d: " << dlimit.value();
	
	if(ulimit<dlimit) {
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
