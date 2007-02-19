/***************************************************************************
 *   Copyright (C) 2006 by Aleix Pol                                       *
 *   aleixpol@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "expression.h"
#include "exp.h"

Expression::Expression() : m_tree(0)
{
}

Expression::Expression(const Expression & e) : m_tree(0), m_err(e.m_err)
{
	if(e.isCorrect())
		m_tree = objectCopy(e.m_tree);
}

Expression::Expression(const QString & exp, bool mathml) : m_tree(0), m_err()
{
	if(mathml)
		setMathML(exp);
	else
		setText(exp);
}

Expression::~Expression()
{
	if(m_tree)
		delete m_tree;
}

Expression Expression::operator=(const Expression & e)
{
	if(this != &e) {
		m_tree = objectCopy(e.m_tree);
		m_err = e.m_err;
	}
	return *this;
}

bool Expression::setText(const QString & exp)
{
	m_err = QStringList();
	Exp e(exp);
	e.parse();
	bool b;
	if(b=e.error().isEmpty()) {
		setMathML(e.mathML());
	} else {
		m_err << e.error();
	}
	return b;
}

bool Expression::setMathML(const QString & s)
{
	m_err.clear();
	
	if(m_tree)
		delete m_tree;
	
	QDomDocument doc;
	
	if (!doc.setContent(s)) {
		m_err << i18n("Error while parsing: %1").arg(s);
		return false;
	}
	
	m_tree = branch(doc.documentElement());
	return true;
}

Object* Expression::branch(const QDomElement& elem)
{
	Container *c; Cn *num; Operator *op;
	QDomNode n;
	Object* ret=0;
	
	switch(whatType(elem.tagName())) {
		case Object::container: {
			c=new Container(Container::toContainerType(elem.tagName()));
			
			n = elem.firstChild();
			while(!n.isNull()) {
				if(n.isElement())
					c->appendBranch(branch(n.toElement()));
				
				n = n.nextSibling();
			}
			
			//Error collection
			QList<Object*>::iterator i=c->m_params.begin();
			Cn u=uplimit(*c), d=downlimit(*c); //FIXME: Don't look for it, append it at 2nd position ors
			bool dGreaterU = (u.isCorrect() && d.isCorrect()) && d.value()>u.value();
			if(dGreaterU)
				m_err << "The downlimit is greater than the uplimit. Probably should be "+ QString("%1..%2").arg(u.value()).arg(d.value());
			//EOCollect
			ret = c;
			break;}
		case Object::value:
			num= new Cn(0.);
			num->setValue(elem);
			ret = num;
			break;
		case Object::oper:
			op= new Operator(Operator::toOperatorType(elem.tagName()));
			ret = op;
			break;
		case Object::variable:
			{
				Ci* var = new Ci(elem.text());
				var->setFunction(elem.attribute("type")=="function");
				ret=var;
			}
			break;
		case Object::none:
			qDebug() << "Not supported: " << elem.tagName();
	}
	return ret;
}


QString Expression::toMathML() const
{
	if(isCorrect())
		return m_tree->toMathML();
	else
		return QString::null;
}

QString Expression::toString() const
{
	if(isCorrect())
		return m_tree->toString();
	else
		return QString::null;
}

enum Object::ObjectType Expression::whatType(const QString& tag)
{
	Object::ObjectType ret=Object::none;
	if(tag=="cn")
		ret= Object::value;
	else if(tag=="ci")
		ret= Object::variable;
	else if(Operator::toOperatorType(tag)!=0)
		ret= Object::oper;
	else if(Container::toContainerType(tag)!=0)
		ret= Object::container;
	
	return ret;
}

Cn Expression::uplimit(const Container& c)
{
	for(QList<Object*>::const_iterator it=c.m_params.begin(); it!=c.m_params.end(); ++it) {
		Container *c = (Container*) (*it);
		if(c->type()==Object::container && c->containerType()==Object::uplimit && c->m_params[0]->type()==Object::value)
			return Cn(c->m_params[0]);
	}
	Cn r=Cn(0.);
	r.setCorrect(false);
	return r;
}

Cn Expression::downlimit(const Container& c)
{
	for(QList<Object*>::const_iterator it=c.m_params.begin(); it!=c.m_params.end(); ++it) {
		Container *c = (Container*) (*it);
		if(c->type()==Object::container && c->containerType()==Object::downlimit && c->m_params[0]->type()==Object::value)
			return Cn(c->m_params[0]);
	}
	Cn r=Cn(0.);
	r.setCorrect(false);
	return r;
}

bool Expression::operator==(const Expression & e) const
{
	return Container::equalTree(e.m_tree, m_tree);
}

Cn Expression::uplimit() const
{
	if(m_tree->type() == Object::container) {
		Container *c= (Container*) m_tree;
		return uplimit(c->m_params[0]);
	}
	return Cn(0.);
}

Cn Expression::downlimit() const
{
	if(m_tree->type() == Object::container) {
		Container *c= (Container*) m_tree;
		return downlimit(c->m_params[0]);
	}
	return Cn(0.);
}

Object* Expression::objectCopy(const Object * old)
{
	if(old==0)
		return 0;
	Object *o=0;
	switch(old->type()) {
		case Object::oper:
			o = new Operator(old);
			break;
		case Object::value:
			o = new Cn(old);
			break;
		case Object::variable:
			o = new Ci(old);
			break;
		case Object::container:
			o = new Container(old);
			break;
		default:
			qDebug() << "Oops!";
	}
	return o;
}


