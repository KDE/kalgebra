/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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
#include "variable.h"
#include "expressionwriter.h"
#include "stringexpressionwriter.h"
#include "htmlexpressionwriter.h"
#include "mathmlexpressionwriter.h"
#include "mathmlpresentationexpressionwriter.h"
#include "list.h"
#include "analitzautils.h"
#include "apply.h"
#include "customobject.h"

static void print_dom(const QDomNode& in, int ind);

using namespace Analitza;

class Expression::ExpressionPrivate : public QSharedData
{
public:
	ExpressionPrivate(Object* t) : m_tree(t) {}
	
	bool canAdd(const Object* where, const Object* branch);
	bool check(const Container* c);
	bool check(const Apply* c);
	
	Object* branch(const QDomElement& elem);
	
	template <class T>
	T* addElements(T* v, const QDomElement* elem)
	{
		QDomNode n = elem->firstChild();
		while(!n.isNull()) {
			if(n.isElement()) {
				Object* ob= branch(n.toElement()); //TODO: We should do that better
				
				if(ob && canAdd(v, ob)) {
					v->appendBranch(ob);
				} else {
// 					delete ob;
					delete v;
					return 0;
				}
			}
			n = n.nextSibling();
		}
		return v;
	}
	
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

bool Expression::ExpressionPrivate::check(const Apply* c)
{
	bool ret=true;
	Operator op=c->firstOperator();
	Operator::OperatorType opt=op.operatorType();
	int cnt=c->countValues();
	
	if(((op.nparams()<0 && cnt<=1) || (op.nparams()>-1 && cnt!=op.nparams())) && (opt!=Operator::minus || cnt==0))
	{
		if(op.nparams()<0)
			m_err << i18n("<em>%1</em> needs at least 2 parameters", op.toString());
		else
			m_err << i18n("<em>%1</em> requires %2 parameters", op.toString(), op.nparams());
		ret=false;
	}
	
	if(op.isBounded() && !c->hasBVars()) {
		m_err << i18n("Missing boundary for '%1'", op.toString());
	} else if(!op.isBounded() && c->hasBVars()) {
		m_err << i18n("Unexpected bounding for '%1'", op.toString());
	}
	
	if(op.isBounded() && op!=Operator::diff) {
		if(!(c->ulimit() && c->dlimit()) && !c->domain()) {
			m_err << i18n("<em>%1</em> missing bounds on '%2'", c->bvarStrings().join(" "), op.toString());
		}
	}
	
	return ret;
}

bool Expression::ExpressionPrivate::check(const Container* c)
{
	bool ret=true;
	
	switch(c->containerType()) {
		case Container::declare:
			if(c->m_params.size()!=2) {
				m_err << i18n("Wrong declare");
				ret=false;
			}
			break;
		default:
			//should never do anything here,
			break;
	}
	
	if(c->isEmpty() && c->containerType()!=Container::math) {
		m_err << i18n("Empty container: %1", c->tagName());
		ret=false;
	}
	
	return ret;
}

bool Expression::ExpressionPrivate::canAdd(const Object* where, const Object* branch)
{
	Q_ASSERT(where && branch);
	bool correct=true;
	
	if(branch->type()==Object::container) {
		const Container* cBranch=static_cast<const Container*>(branch);
		if(cBranch->containerType()==Container::piece || cBranch->containerType()==Container::otherwise) {
			bool isPiecewise=where->type()==Object::container
				&& static_cast<const Container*>(where)->containerType()==Container::piecewise;
			if(!isPiecewise) {
				m_err << i18nc("there was a conditional outside a condition structure",
								"We can only have conditionals inside piecewise structures.");
				correct=false;
			}
		} else if(cBranch->containerType()==Container::bvar && (where->isContainer() || where->isApply())) {
			//we don't want 2 bvar in the same element
			QStringList bvars;
			if(where->isContainer())
				bvars=static_cast<const Container*>(where)->bvarStrings();
			else
				bvars=static_cast<const Apply*>(where)->bvarStrings();
			
			const Ci* var= static_cast<const Ci*>(cBranch->m_params.first());
			
			if(bvars.contains(var->name())) {
				correct=false;
				m_err << i18n("Cannot have two parameters with the same name like '%1'.", var->name());
			}
		}
	}
	
	if(where->type()==Object::container) {
		const Container* cWhere=static_cast<const Container*>(where);
		
		if(cWhere->containerType()==Container::piecewise) {
			bool isCondition=false;
			if(branch->type()==Object::container) {
				Container::ContainerType ct=static_cast<const Container*>(branch)->containerType();
				isCondition=ct==Container::piece || ct==Container::otherwise;
				
				if(cWhere->extractType(Container::otherwise)) {
					m_err << i18nc("this is an error message. otherwise is the else in a mathml condition",
						"The <em>otherwise</em> parameter should be the last one");
					correct=false;
				}
				
			}
			
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
			if(branch->type()!=Object::variable) {
				m_err << i18n("We can only have bounded variables");
				correct=false;
			}
		}
	}
	
	Q_ASSERT(correct || !m_err.isEmpty());
	
	return correct;
}

static void variableDepth(Object* o, int& next, const QMap<QString, int>& scope)
{
	Q_ASSERT(o);
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			QMap<QString, int>::const_iterator it = scope.constFind(i->name());
			i->setBVarDepth(it==scope.constEnd() 
							? -1
							: *it);
			
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it)
				variableDepth(*it, next, scope);
			
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it)
				variableDepth(*it, next, scope);
			
		}	break;
		case Object::container: {
			const Container *c = (const Container*) o;
			
			QMap<QString, int> newScope=scope;
			
			Container::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
			if(c->containerType()==Container::declare)
				++it;
			
			foreach(Ci* bvar, c->bvarCi()) {
				newScope.insert(bvar->name(), next);
				bvar->setBVarDepth(next++);
			}
			
			for(; it!=itEnd; ++it)
				variableDepth(*it, next, newScope);
			
		}	break;
		case Object::apply: {
			const Apply *c = (const Apply*) o;
			
			Object* ul=c->ulimit(), *dl=c->dlimit(), *dn=c->domain();
			
			//uplimit and downlimit are in the parent scope
			if(ul) variableDepth(ul, next, scope);
			if(dl) variableDepth(dl, next, scope);
			if(dn) variableDepth(dn, next, scope);
			
			QMap<QString, int> newScope=scope;
			
			foreach(Ci* bvar, c->bvarCi()) {
				newScope.insert(bvar->name(), next);
				bvar->setBVarDepth(next++);
			}
			
			Container::const_iterator it = c->firstValue();
			for(; it!=c->constEnd(); ++it) {
				variableDepth(*it, next, newScope);
			}
		}	break;
		case Object::none:
		case Object::value:
		case Object::custom:
		case Object::oper:
			break;
	}
}

void Expression::computeDepth(Object* o)
{
	if(o) {
		QMap<QString, int> scope;
		int next=0;
		variableDepth(o, next, scope);
	}
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
	
	d->m_tree = d->branch(doc.documentElement());
	
	computeDepth(d->m_tree);
	
	return d->m_tree;
}

Object* Expression::ExpressionPrivate::branch(const QDomElement& elem)
{
	Cn *num; Operator *op;
	Object* ret=0;
	
	switch(whatType(elem.tagName())) {
		case Object::container: {
			Container::ContainerType t = Container::toContainerType(elem.tagName());
			
			if(t!=Container::none) {
				Container* c=addElements<Container>(new Container(t), &elem);
				
				if(c && !check(c)) {
					delete c;
					c=0;
				}
				ret = c;
			} else
				m_err << i18nc("An error message", "Container unknown: %1", elem.tagName());
		}	break;
		case Object::value:
			num= new Cn(0.);
			if(!num->setValue(elem)) {
				delete num;
				m_err<< i18n("Cannot codify the %1 value.", elem.text());
			} else
				ret = num;
			break;
		case Object::oper:
			if(elem.hasChildNodes()) {
				m_err << i18n("The %1 operator cannot have child contexts.", elem.tagName());
			} else {
				Operator::OperatorType type=Operator::toOperatorType(elem.tagName());
				if(type==Operator::none)
					m_err << i18n("The element '%1' is not an operator.", elem.tagName());
				else
					ret = op = new Operator(type);
			}
			break;
		case Object::variable: {
			Ci* var = new Ci(elem.text());
			var->setFunction(elem.attribute("type")=="function");
			ret=var;
		}	break;
		case Object::vector: {
			ret=addElements<Vector>(new Vector(elem.childNodes().count()), &elem);
			if(elem.childNodes().count()==0) {
				m_err << i18n("Do not want empty vectors");
			}
// 			qDebug() << "PEEEEEEU" << v->size();
		}	break;
		case Object::list:
			ret=addElements<List>(new List, &elem);
			break;
		case Object::apply: {
			Apply *a=addElements<Apply>(new Apply, &elem);
			if(a && !check(a)) {
				delete a;
				a=0;
			}
			ret=a;
		}	break;
		case Object::none:
		case Object::custom:
			m_err << i18nc("Error message due to an unrecognized input",
							  "Not supported/unknown: %1", elem.tagName());
			break;
	}
	
	Q_ASSERT(ret || !m_err.isEmpty());
	return ret;
}

QString Expression::toHtml() const
{
	Q_ASSERT(isCorrect());
	return HtmlExpressionWriter(d->m_tree).result();
}

QString Expression::toMathMLPresentation() const
{
	Q_ASSERT(isCorrect());
	return MathMLPresentationExpressionWriter(d->m_tree).result();
}

QString Expression::toMathML() const
{
	Q_ASSERT(isCorrect());
	return MathMLExpressionWriter(d->m_tree).result();
}

QString Expression::toString() const
{
	Q_ASSERT(isCorrect());
	StringExpressionWriter s(d->m_tree);
	return s.result();
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
	else if(tag=="list")
		ret= Object::list;
	else if(tag=="apply")
		ret= Object::apply;
	else if(Operator::toOperatorType(tag)!=0)
		ret= Object::oper;
	else if(Container::toContainerType(tag)!=0)
		ret= Object::container;
	
	return ret;
}

bool Expression::operator==(const Expression & e) const
{
	return e.d->m_tree && d->m_tree && AnalitzaUtils::equalTree(e.d->m_tree, d->m_tree);
}

void Expression::clear()
{
	delete d->m_tree;
	d->m_tree=0;
	d->m_err.clear();
}

bool Expression::isCorrect() const
{
	return d && d->m_tree && d->m_err.isEmpty();
}

QStringList Expression::bvarList() const
{
	Q_ASSERT(d->m_tree);
	const Object *o = d->m_tree;
	
	if(o->isContainer() && static_cast<const Container*>(o)->containerType()==Container::math) {
		o = (Container*) static_cast<const Container*>(o)->m_params[0];
	}
	
	if(o->isApply())
		return static_cast<const Apply*>(o)->bvarStrings();
	else if(o->isContainer())
		return static_cast<const Container*>(o)->bvarStrings();
	
	return QStringList();
}

Cn Expression::toReal() const
{
	Object* tree=d->m_tree;
	if(KDE_ISLIKELY(tree && tree->type()==Object::value))
		return *static_cast<Cn*>(tree);
	else {
		qDebug() << "trying to return an invalid value:" << (tree ? tree->toString() : "null");
		return Cn(0.);
	}
}

bool Expression::isLambda() const
{
	if(d->m_tree && d->m_tree->type()==Object::container) {
		Container* c = (Container*) d->m_tree;
		if(c->containerType()==Container::math) {
			Container *c1 = (Container*) c->m_params.first();
			return c1->type()==Object::container && c1->containerType()==Container::lambda;
		}
		return c->containerType()==Container::lambda;
	}
	return false;
}

QList<Ci*> Expression::parameters() const
{
	QList<Ci*> ret;
	if(d->m_tree && d->m_tree->type()==Object::container) {
		Container* c = (Container*) d->m_tree;
		if(c->containerType()==Container::math) {
			Container *c1 = (Container*) c->m_params.first();
			if(c1->type()==Object::container && c1->containerType()==Container::lambda)
				return c1->bvarCi();
		}
		return c->bvarCi();
	}
	return ret;
}

Expression Expression::lambdaBody() const
{
	Q_ASSERT(isLambda());
	Object *ret=0;
	if(d->m_tree) {
		Container* c = (Container*) d->m_tree;
		if(c->containerType()==Container::math) {
			Container *c1 = (Container*) c->m_params.first();
			ret = c1->m_params.last();
		} else
		ret = c->m_params.last();
	}
	
	Object* o = ret->copy();
	computeDepth(o);
	return Expression(o);
}

bool Expression::isVector() const
{
	if(d->m_tree) {
		if(d->m_tree->isContainer()) {
			Container *c = (Container*) d->m_tree;
			return c->containerType()==Container::math && c->m_params.first()->type()==Object::vector;
		} else
			return d->m_tree->type()==Container::vector;
	}
	return false;
}

bool Expression::isList() const
{
	if(d->m_tree) {
		if(d->m_tree->isContainer()) {
			Container *c = (Container*) d->m_tree;
			return c->containerType()==Container::math && c->m_params.first()->type()==Object::list;
		} else
			return d->m_tree->type()==Container::list;
	}
	return false;
}

QList<Expression> Expression::toExpressionList() const
{
	bool isvector = isVector();
	if(!isvector || !isList() || !d->m_tree)
		return QList<Expression>();
	
	QList<Expression> ret;
	const Object* o=d->m_tree;
	if(d->m_tree->isContainer()) {
		const Container *c = (const Container*) d->m_tree;
		Q_ASSERT(c->containerType()==Container::math);
		o=c->m_params.first();
	}
	
	if(isvector) {
		const Vector* v = (const Vector*) o;
		for(Vector::const_iterator it=v->constBegin(), itEnd=v->constEnd(); it!=itEnd; ++it) {
			Object* newelement = (*it)->copy();
			computeDepth(newelement);
			ret << Expression(newelement);
		}
	} else {
		const List* v = (const List*) o;
		for(List::const_iterator it=v->constBegin(), itEnd=v->constEnd(); it!=itEnd; ++it) {
			Object* newelement = (*it)->copy();
			computeDepth(newelement);
			ret << Expression(newelement);
		}
	}
	
	return ret;
}

Expression Expression::elementAt(int position) const
{
	Q_ASSERT(isVector());
	Vector *v=0;
	if(d->m_tree) {
		if(d->m_tree->isContainer()) {
			Container *c = (Container*) d->m_tree;
			v=static_cast<Vector*>(c->m_params.first());
		} else
			v=static_cast<Vector*>(d->m_tree);
	}
	
	return Expression(v->at(position)->copy());
}
// 
QStringList Expression::error() const
{
	return d->m_err;
}

void Expression::addError(const QString& error)
{
	d->m_err += error;
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

bool Expression::isReal() const
{
	return d->m_tree && d->m_tree->type()==Object::value;
}

Expression Expression::constructList(const QList< Expression >& exps)
{
	List* l = new List;
	foreach(const Expression& e, exps)
		l->appendBranch(e.tree()->copy());
	
	computeDepth(l);
	return Expression(l);
}

#define CHECK_TYPE(type)\
		type* a = (type *) o;\
		\
		type::iterator it=a->begin(), itEnd=a->end();\
		for(; it!=itEnd; ++it) {\
			renameTree(*it, depth, newName);\
		}\

static void renameTree(Object* o, int depth, const QString& newName)
{
	if(!o)
		return;
	
	switch(o->type()) {
		case Object::variable: {
			Ci* var = (Ci*) o;
			if(var->depth()==depth) 
				var->setName(newName);
		}	break;
		case Object::apply: {
			CHECK_TYPE(Apply);
			renameTree(a->ulimit(), depth, newName);
			renameTree(a->dlimit(), depth, newName);
			renameTree(a->domain(), depth, newName);
		}	break;
		case Object::container: { CHECK_TYPE(Container); } break;
		case Object::list: { CHECK_TYPE(List); } break;
		case Object::vector: { CHECK_TYPE(Vector); } break;
		
		case Object::none:
		case Object::value:
		case Object::oper:
		case Object::custom:
			break;
	}
}

void Expression::renameArgument(int depth, const QString& newName)
{
	renameTree(d->m_tree, depth, newName);
	computeDepth(d->m_tree);
}

Expression Expression::constructCustomObject(const QVariant& custom)
{
	Object* obj = new CustomObject(custom);
	return Expression(obj);
}

QVariant Expression::customObjectValue() const
{
	Object* tree=d->m_tree;
	if(KDE_ISLIKELY(tree && tree->type()==Object::custom))
		return static_cast<const CustomObject*>(tree)->value();
	else {
		qDebug() << "trying to return an invalid value:" << (tree ? tree->toString() : "null");
		return QVariant();
	}
}

static void print_dom(const QDomNode& in, int ind)
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

bool Expression::isCompleteExpression(const QString& exp)
{
	ExpLexer lex(exp);
	while(lex.lex()!=0) {}
	
	return lex.isCompletelyRead();
}
