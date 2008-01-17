/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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
#include "expression.h"

#include <KLocale>

#include "container.h"
#include "exp.h"

Expression::Expression() : m_tree(0), m_err()
{
}

Expression::Expression(Object * o)
	: m_tree(o), m_err()
{
}

Expression::Expression(const Cn & e) : m_tree(0), m_err()
{
	if(e.isCorrect())
		m_tree = new Cn(e);
}

Expression::Expression(const Expression & e) : m_tree(0), m_err(e.m_err)
{
	if(e.isCorrect())
		m_tree = objectCopy(e.m_tree);
	m_err = e.m_err;
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
        if(m_tree)
            delete m_tree;
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
		m_err << i18n("Error while parsing: %1", s);
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
			Container::ContainerType t = Container::toContainerType(elem.tagName());
			if(t!=Container::none) {
				c=new Container(t);
				
				n = elem.firstChild();
				while(!n.isNull()) {
					if(n.isElement()) {
						Object* ob= branch(n.toElement());
						if(ob)
							c->appendBranch(ob);
						else {
							delete c;
							return 0;
						}
					}
					n = n.nextSibling();
				}
				
				//Error collection
				if(c->containerType()==Container::apply) {
					Cn u=uplimit(*c), d=downlimit(*c); //FIXME: Don't look for it, append it at 2nd position ors
					bool dGreaterU = (u.isCorrect() && d.isCorrect()) && d.value()>u.value();
					if(dGreaterU)
						m_err << i18nc("An error message", "The downlimit is greater than the uplimit. Probably should be %1..%2",
										u.value(), d.value());
				}
				
				if(c->containerType()==Container::piecewise) {
					bool correct=true;
					foreach(Object *o, c->m_params) {
						if(o->isContainer()) {
							Container *cc = (Container*) o;
							if(cc->containerType()!=Container::piece && cc->containerType()!=Container::otherwise)
								correct=false;
						} else
							correct=false;
							
						if(!correct) {
							m_err << i18n("%1 is not a piece/otherwise inside the piecewise", o->toString());
							break;
						}
					}
				}
				//EOCollect
				ret = c;
			} else {
				m_err << i18nc("An error message", "Container unknown: %1", elem.tagName());
			}
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
			m_err << i18nc("Error message due to an unrecognized input", "Not supported/unknown: %1", elem.tagName());
			break;
	}
	return ret;
}

QString Expression::toHtml() const
{
	if(isCorrect())
		return m_tree->toHtml();
	else
		return QString();
}

QString Expression::toMathML() const
{
	if(isCorrect())
		return m_tree->toMathML();
	else
		return QString();
}

QString Expression::toString() const
{
	if(isCorrect())
		return m_tree->toString();
	else
		return QString();
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
		if(c->type()==Object::container && c->containerType()==Container::uplimit && c->m_params[0]->type()==Object::value)
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
		if(c->type()==Object::container && c->containerType()==Container::downlimit && c->m_params[0]->type()==Object::value)
			return Cn(c->m_params[0]);
	}
	Cn r=Cn(0.);
	r.setCorrect(false);
	return r;
}

bool Expression::operator==(const Expression & e) const
{
	if(!e.m_tree || !m_tree)
		return false;
	return Container::equalTree(e.m_tree, m_tree);
}

Cn Expression::uplimit() const
{
	Cn ret(0.);
	if(m_tree->type() == Object::container) {
		Container *c= (Container*) m_tree;
		ret=uplimit(c->m_params[0]);
	}
	return ret;
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
	if(!old)
		return 0;
	Object *o=0;
	switch(old->type()) {
		case Object::oper:
// 			Q_ASSERT(dynamic_cast<const Operator*>(old));
			o = new Operator(old);
			break;
		case Object::value:
// 			Q_ASSERT(dynamic_cast<const Cn*>(old));
			o = new Cn(old);
			break;
		case Object::variable:
// 			Q_ASSERT(dynamic_cast<const Ci*>(old));
			o = new Ci(old);
			break;
		case Object::container:
// 			Q_ASSERT(dynamic_cast<const Container*>(old));
			o = new Container(old);
			break;
		case Object::none:
			qFatal("Do not know what are we copying.");
			break;
	}
	return o;
}

void Expression::clear()
{
	if(m_tree)
		delete m_tree;
	m_tree=0;
	m_err.clear();
}

bool Expression::isCorrect() const
{
	return m_tree && m_err.isEmpty() && m_tree->isCorrect();
}

QStringList Expression::bvarList() const
{
	Container *c = (Container*) m_tree;
	if(c!=0 && c->type()==Object::container) {
		c = (Container*) c->m_params[0];
		
		if(c->type()==Object::container)
			return c->bvarList();
	}
	return QStringList();
}

double Expression::value() const
{
	if(m_tree && m_tree->type()==Object::value)
		return ((Cn*) m_tree)->value();
	else {
		qDebug() << "trying to return an invalid value" << m_tree->toString();
		return 0;
	}
}

