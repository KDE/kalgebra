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

void print_dom(const QDomNode& in, int ind);

class Expression::ExpressionPrivate : public QSharedData
{
public:
	ExpressionPrivate(Object* t) : m_tree(t) {}
	
	bool canAdd(Object* where, Object* branch);
	bool check(Container* c);
	
	Object* m_tree;
	QStringList m_err;
};

Expression::Expression()
	: d(new ExpressionPrivate(0))
{}

Expression::Expression(Object * o)
	: d(new ExpressionPrivate(o))
{}

Expression::Expression(const Cn & e)
	: d(new ExpressionPrivate(0))
{
	if(e.isCorrect())
		d->m_tree = new Cn(e);
}

Expression::Expression(const Expression & e)
	: d(new ExpressionPrivate(0))
{
	d->m_err=e.d->m_err;
	if(e.isCorrect())
		d->m_tree = e.d->m_tree->copy();
}

Expression::Expression(const QString & exp, bool mathml)
	: d(new ExpressionPrivate(0))
{
	if(mathml)
		setMathML(exp);
	else
		setText(exp);
}

Expression::~Expression()
{
	Q_ASSERT(d);
	delete d->m_tree;
}

Expression Expression::operator=(const Expression & e)
{
	if(this != &e) {
		delete d->m_tree;
		
		if(e.d->m_tree)
			d->m_tree = e.d->m_tree->copy();
		else
			d->m_tree=0;
		
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

bool Expression::ExpressionPrivate::check(Container* c)
{
	bool ret=true;
	switch(c->containerType()) {
		case Container::apply: {
			Operator op=c->firstOperator();
			Operator::OperatorType opt=op.operatorType();
			int cnt=c->countValues();
			
			if(((op.nparams()<0 && cnt<=1) || (op.nparams()>-1 && cnt!=op.nparams())) && opt!=Operator::minus)
			{
				if(op.nparams()<0)
					m_err << i18n("<em>%1</em> needs at least 2 parameters", op.toString());
				else
					m_err << i18n("<em>%1</em> requires %2 parameters", op.toString(), op.nparams());
				ret=false;
			}
		}	break;
	}
	return ret;
}

bool Expression::ExpressionPrivate::canAdd(Object* where, Object* branch)
{
	Q_ASSERT(where && branch);
	bool correct=true;
	
	if(branch->isContainer()) {
		Container* c1=static_cast<Container*>(branch);
		if(c1->containerType()==Container::piece || c1->containerType()==Container::otherwise) {
			bool isPiecewise=where->isContainer() && static_cast<Container*>(where)->containerType()==Container::piecewise;
			if(!isPiecewise) {
				m_err << i18nc("there was a conditional outside a condition structure",
								"We can only have conditionals inside piecewise structures.");
				correct=false;
			}
		}
	}
	
	if(where->isContainer()) {
		Container* cWhere=static_cast<Container*>(where);
		
		if(cWhere->containerType()==Container::piecewise) {
			bool isCondition=branch->isContainer() &&
				(static_cast<Container*>(branch)->containerType()==Container::piece ||
				static_cast<Container*>(branch)->containerType()==Container::otherwise);
			if(!isCondition) {
				m_err << i18nc("there was an element that was not a conditional inside a condition",
									"%1 is not a proper condition inside the piecewise", branch->toString());
				correct=false;
			}
		} else if(cWhere->containerType()==Container::declare && cWhere->isEmpty() && branch->type()!=Object::variable) {
			m_err << i18n("We can only declare variables");
			correct=false;
		}
		
		if(cWhere->containerType()==Container::bvar) {
			if(branch->type()!=Object::variable)
				m_err << i18n("We can only have bounded variables");
		}
	}
	Q_ASSERT(correct || !m_err.isEmpty());
	return correct;
}

bool Expression::setMathML(const QString & s)
{
	d->m_err.clear();
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
	Object* ret=0;
	
	switch(whatType(elem.tagName())) {
		case Object::container: {
			Container::ContainerType t = Container::toContainerType(elem.tagName());
			
			if(t!=Container::none) {
				Container *c=new Container(t);
				
				QDomNode n = elem.firstChild();
				while(!n.isNull()) {
					if(n.isElement()) {
						Object* ob= branch(n.toElement());
						
						if(ob && d->canAdd(c, ob)) {
							c->appendBranch(ob);
						} else {
							delete c;
							return 0;
						}
					}
					n = n.nextSibling();
				}
				if(!d->check(c)) {
					delete c;
					return 0;
				}
				ret = c;
			} else {
				d->m_err << i18nc("An error message", "Container unknown: %1", elem.tagName());
			}
			break;}
		case Object::value:
			num= new Cn(0.);
			if(!num->setValue(elem)) {
				delete num;
				d->m_err<< i18n("Cannot codify the %1 value.", elem.text());
			} else
				ret = num;
			break;
		case Object::oper:
			if(elem.hasChildNodes()) {
				d->m_err << i18n("The %1 operator cannot have child contexts.", elem.tagName());
			} else {
				op= new Operator(Operator::toOperatorType(elem.tagName()));
				ret = op;
			}
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
			Vector* v = new Vector(elem.childNodes().count());
			QDomNode n = elem.firstChild();
			while(!n.isNull()) {
				if(n.isElement()) {
					Object* ob= branch(n.toElement());
					
					if(ob && d->canAdd(v, ob)) {
						v->appendBranch(ob);
					} else {
						delete v;
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
	Q_ASSERT(ret || !d->m_err.isEmpty());
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

bool Expression::operator==(const Expression & e) const
{
	return e.d->m_tree && d->m_tree && Container::equalTree(e.d->m_tree, d->m_tree);
}

Expression Expression::uplimit() const
{
	Expression ret;
	if(d->m_tree->type() == Object::container) {
		Container *c= (Container*) d->m_tree;
		if(!c->m_params.isEmpty())
			ret=Expression(c->ulimit());
	}
	return ret;
}

Expression Expression::downlimit() const
{
	Expression ret;
	if(d->m_tree->isContainer()) {
		Container *c= (Container*) d->m_tree;
		if(!c->m_params.isEmpty())
			ret=Expression(c->dlimit());
	}
	return ret;
}

void Expression::clear()
{
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
	if(c!=0 && c->isContainer()) {
		c = (Container*) c->m_params[0];
		
		if(c->isContainer())
			return c->bvarList();
	}
	return QStringList();
}

Cn Expression::value() const
{
	if(KDE_ISLIKELY(d->m_tree && d->m_tree->type()==Object::value))
		return *static_cast<Cn*>(d->m_tree);
	else {
// 		qDebug() << "trying to return an invalid value" << d->m_tree ? d->m_tree->toString() : QString();
		return Cn(0.);
	}
}

bool Expression::isLambda() const
{
	Container* c = (Container*) d->m_tree;
	if(d->m_tree && d->m_tree->isContainer() && c->containerType()==Container::math && !c->m_params.isEmpty()) {
		Container *c1 = (Container*) c->m_params.first();
		return c1->isContainer() && c1->containerType()==Container::lambda;
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

void print_dom(const QDomNode& in, int ind)
{
	QString a;
	
	if(ind >100){
		qDebug("...");
		return;
	}
	
	for(int i=0; i<ind; i++)
		a.append("______|");
	
	if(in.hasChildNodes())
		qDebug("%s%s(%s) -- %d", qPrintable(a), qPrintable(in.toElement().tagName()), qPrintable(in.toElement().text()), in.childNodes().length());
	else
		qDebug("%s%s", qPrintable(a), qPrintable(in.toElement().tagName()));
	
	for(unsigned int i=0 ; i<in.childNodes().length(); i++){
		if(in.childNodes().item(i).isElement())
			print_dom(in.childNodes().item(i), ind+1);
	}
}
