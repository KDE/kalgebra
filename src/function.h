#ifndef FUNCTION_H
#define FUNCTION_H

#include <QColor>
#include "analitza.h"
#include "expression.h"

/**
@author Aleix Pol i Gonzalez
*/

enum Axe { Cartesian=0, Polar};
class function
{
	friend class Graph2D;
public:
	function();
	function(const function& f);
	function(const QString& name, const Expression& newExp, const QColor& color, bool selec=false);
	~function();
	
	Analitza *func;
	
	int setFunction(const QString& name, const Expression& newExp, const QColor&, bool selec=false);
	void update_points(QRect viewport, unsigned int resolucio);
	
	QColor color() const { return m_color; }
	void setColor(const QColor& newColor) { m_color=newColor; }
	unsigned int npoints() const { return m_last_resolution; }
	void setSelected(bool newSelec) { m_selected=newSelec; }
	bool selected() const { return m_selected; }
	void setShown(bool newShow) { m_show=newShow; }
	bool isShown() const { return m_show && func->isCorrect(); }
	QPair<QPointF, QString> calc(const QPointF& dp);
	bool operator==(const function& f) const { return f.m_name==m_name; }
	QString name() const { return m_name; }
	
	function operator=(const function& f);
	Analitza* analitza() const { return func; }
	
	Axe axeType() const;
private:
	bool m_show;
	bool m_selected;
	QString m_firstlambda;
	QColor m_color;
	QRect m_last_viewport;
	unsigned int m_last_resolution;
	unsigned int m_last_max_res;
	QStringList err; //function errors
	void update_pointsY(QRect viewport, unsigned int resolucio); //for functions such as y=f(x)
	void update_pointsX(QRect viewport, unsigned int resolucio); //for functions such as x=f(y)
	void update_pointsPolar(QRect viewport, unsigned int resolucio); //for functions such as r=f(sigma)
	
	inline QPointF fromPolar(double r, double th) { return QPointF(r*cos(th), r*sin(th)); }
	QString m_name;
protected:
	QPointF *points;
};

#endif
