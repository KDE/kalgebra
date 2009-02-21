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
#include <QDomElement>

#include "explexer.h"
#include "expressionparser.h"
#include "container.h"
#include "value.h"
#include "vector.h"
#include "expressionwriter.h"
#include "stringexpressionwriter.h"
#include "htmlexpressionwriter.h"
#include "mathmlexpressionwriter.h"
#include "mathmlpresentationexpressionwriter.h"

class Expression::ExpressionPrivate
{
public:
	ExpressionPrivate(Object* t) : m_tree(t) {}
	
	static Object* extractType(const Container& c, Container::ContainerType t);
	
	Object* m_tree;
	QStringList m_err;
};

Expression::Expression()
{
	d=new ExpressionPrivate(0);
}

Expression::Expression(Object * o)
{
	d=new ExpressionPrivate(o);
}

Expression::Expression(const Cn & e)
{
	d=new ExpressionPrivate(0);
	if(e.isCorrect())
		d->m_tree = new Cn(e);
}

Expression::Expression(const Expression & e)
{
	d=new ExpressionPrivate(0);
	d->m_err=e.d->m_err;
	if(e.isCorrect())
		d->m_tree = objectCopy(e.d->m_tree);
}

Expression::Expression(const QString & exp, bool mathml)
{
	d=new ExpressionPrivate(0);
	if(mathml)
		setMathML(exp);
	else
		setText(exp);
}

Expression::~Expression()
{
	if(d && d->m_tree)
		delete d->m_tree;
	delete d;
}

Expression Expression::operator=(const Expression & e)
{
	if(this != &e) {
		if(d->m_tree)
			delete d->m_tree;
		d->m_tree = objectCopy(e.d->m_tree);
		d->m_err = e.d->m_err;
	}
	return *this;
}

bool Expression::setText(const QString & exp)
{
	d->m_err.clear();
	ExpLexer lex(exp);
	ExpressionParser parser;
	bool corr=parser.parse(&lex);
	
	if(corr)
		setMathML(parser.mathML());
	else
		d->m_err << parser.error();
	
	return corr;
}

bool Expression::setMathML(const QString & s)
{
	d->m_err.clear();
	
	if(d->m_tree)
		delete d->m_tree;
	
	QDomDocument doc;
	
	if (!doc.setContent(s)) {
		d->m_err << i18n("Error while parsing: %1", s);
		return false;
	}
	
	d->m_tree = branch(doc.documentElement());
	return d->m_tree;
}

Object* Expression::branch(const QDomElement& elem)
{
	Cn *num; Operator *op;
	QDomNode n;
	Object* ret=0;
	
	switch(whatType(elem.tagName())) {
		case Object::container: {
			Container::ContainerType t = Container::toContainerType(elem.tagName());
			if(t!=Container::none) {
				Container *c=new Container(t);
				
				n = elem.firstChild();
				while(!n.isNull()) {
					if(n.isElement()) {
						Object* ob= branch(n.toElement());
						
						if(ob && ob->isContainer() && c->containerType()!=Container::piecewise)
						{
							Container* c1=static_cast<Container*>(ob);
							if(c1->containerType()==Container::piece || c1->containerType()==Container::otherwise) {
								d->m_err << i18nc("there was a conditional outside a condition structure",
												"We can only have conditionals inside piecewise structures.");
							}
						}
						
						if(ob && d->m_err.isEmpty()) {
							c->appendBranch(ob);
						} else {
							delete c;
							return 0;
						}
					}
					n = n.nextSibling();
				}
				
				//Error collection
				Expression ul=uplimit(*c);
				Expression dl=downlimit(*c);
				if(c->containerType()==Container::apply && ul.isValue() && dl.isValue()) {
					//FIXME: Don't look for it, append it at 2nd position ors
					Cn u=ul.tree(), d=ul.tree();
					bool dGreaterU = (u.isCorrect() && d.isCorrect()) && d.value()>u.value();
					if(dGreaterU)
						this->d->m_err << i18nc("An error message", "The downlimit is greater than the uplimit. "
													"Probably should be %1..%2", u.value(), d.value());
				} else if(c->containerType()==Container::piecewise) {
					bool correct=true;
					foreach(Object *o, c->m_params) {
						if(o->isContainer()) {
							Container *cc = (Container*) o;
							if(cc->containerType()!=Container::piece && cc->containerType()!=Container::otherwise)
								correct=false;
						} else
							correct=false;
						
						if(!correct) {
							d->m_err << i18nc("there was an element that was not a conditional inside a condition",
									"%1 is not a proper condition inside the piecewise", o->toString());
							break;
						}
					}
				} else if(c->containerType()==Container::declare) {
					if(c->m_params.first()->type()!=Object::variable) {
						d->m_err << i18n("We can only declare variables");
					}
				}
				//EOCollect
				ret = c;
			} else {
				d->m_err << i18nc("An error message", "Container unknown: %1", elem.tagName());
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
		case Object::vector:
		{
			Vector* v = new Vector(n.childNodes().count());
			n = elem.firstChild();
			while(!n.isNull()) {
				if(n.isElement()) {
					Object* ob= branch(n.toElement());
					
					if(ob && ob->isContainer())
					{
						Container* c1=static_cast<Container*>(ob);
						if(c1->containerType()==Container::piece || c1->containerType()==Container::otherwise) {
							d->m_err << i18nc("there was a conditional outside a condition structure",
											"We can only have conditionals inside piecewise structures.");
						}
					}
					
					if(ob) {
						v->appendBranch(ob);
					} else {
						return 0;
					}
				}
				n = n.nextSibling();
			}
			ret=v;
		}	break;
		case Object::none:
			d->m_err << i18nc("Error message due to an unrecognized input", "Not supported/unknown: %1", elem.tagName());
			break;
	}
	return ret;
}

QString Expression::toHtml() const
{
	if(isCorrect()) {
		HtmlExpressionWriter s(d->m_tree);
		return s.result();
	} else
		return QString();
}

QString Expression::toMathMLPresentation() const
{
	if(isCorrect()) {
		MathMLPresentationExpressionWriter s(d->m_tree);
		return s.result();
	} else
		return QString();
}

QString Expression::toMathML() const
{
	if(isCorrect()) {
		MathMLExpressionWriter s(d->m_tree);
		return s.result();
	} else
		return QString();
}

QString Expression::toString() const
{
	if(isCorrect()) {
		StringExpressionWriter s(d->m_tree);
		return s.result();
	} else
		return QString();
}

enum Object::ObjectType Expression::whatType(const QString& tag)
{
	Object::ObjectType ret=Object::none;
	if(tag=="cn")
		ret= Object::value;
	else if(tag=="ci")
		ret= Object::variable;
	else if(tag=="vector")
		ret= Object::vector;
	else if(Operator::toOperatorType(tag)!=0)
		ret= Object::oper;
	else if(Container::toContainerType(tag)!=0)
		ret= Object::container;
	
	return ret;
}

Object* Expression::ExpressionPrivate::extractType(const Container& c, Container::ContainerType t)
{
	for(QList<Object*>::const_iterator it=c.m_params.begin(); it!=c.m_params.end(); ++it) {
		Container *c = (Container*) (*it);
		if(c->type()==Object::container && c->containerType()==t)
			return c->m_params.first();
	}
	return 0;
}

Expression Expression::uplimit(const Container& c)
{
	return Expression(objectCopy(ExpressionPrivate::extractType(c, Container::uplimit)));
}

Expression Expression::downlimit(const Container& c)
{
	return Expression(objectCopy(ExpressionPrivate::extractType(c, Container::downlimit)));
}

bool Expression::operator==(const Expression & e) const
{
	return e.d->m_tree && d->m_tree && Container::equalTree(e.d->m_tree, d->m_tree);
}

Expression Expression::uplimit() const
{
	Expression ret;
	if(d->m_tree->type() == Object::container) {
		Container *c= (Container*) d->m_tree;
		ret=uplimit(c->m_params[0]);
	}
	return ret;
}

Expression Expression::downlimit() const
{
	if(d->m_tree->type() == Object::container) {
		Container *c= (Container*) d->m_tree;
		return downlimit(c->m_params[0]);
	}
	return Expression();
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
		case Object::vector:
// 			Q_ASSERT(dynamic_cast<const Vector*>(old));
			o = new Vector(old);
			break;
		case Object::none:
			qFatal("Do not know what are we copying.");
			break;
	}
	return o;
}

void Expression::clear()
{
	if(d->m_tree)
		delete d->m_tree;
	d->m_tree=0;
	d->m_err.clear();
}

bool Expression::isCorrect() const
{
	return d && d->m_tree && d->m_err.isEmpty() && d->m_tree->isCorrect();
}

QStringList Expression::bvarList() const
{
	Container *c = (Container*) d->m_tree;
	if(c!=0 && c->type()==Object::container) {
		c = (Container*) c->m_params[0];
		
		if(c->type()==Object::container)
			return c->bvarList();
	}
	return QStringList();
}

Cn Expression::value() const
{
	if(KDE_ISLIKELY(d->m_tree && d->m_tree->type()==Object::value))
		return Cn((Cn*) d->m_tree);
	else {
// 		qDebug() << "trying to return an invalid value" << d->m_tree ? d->m_tree->toString() : QString();
		return 0.;
	}
}

bool Expression::isLambda() const
{
	Container* c = (Container*) d->m_tree;
	if(d->m_tree && d->m_tree->isContainer() && c->containerType()==Container::math) {
		Container *c1 = (Container*) c->m_params.first();
		return c->m_params[0]->isContainer() && c1->containerType()==Container::lambda;
	} else
		return false;
}

QStringList Expression::error() const
{
	return d->m_err;
}

const Object* Expression::tree() const
{
	return d->m_tree;
}

Object* Expression::tree()
{
	return d->m_tree;
}

void Expression::setTree(Object* o)
{
	d->m_tree=o;
}

bool Expression::isValue() const
{
	return d->m_tree && d->m_tree->type()==Object::value;
}
