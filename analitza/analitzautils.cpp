/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
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

#include "analitzautils.h"

#include "expressionwriter.h"
#include "vector.h"
#include "value.h"
#include "list.h"
#include "variable.h"
#include "container.h"
#include "variables.h"
#include "expression.h"
#include "apply.h"
#include <QVariant>

using namespace Analitza;
namespace AnalitzaUtils
{
	
QStringList dependencies(const Object* o, const QStringList& scope)
{
	Q_ASSERT(o);
	
	QSet<QString> ret;
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			if(!scope.contains(i->name()))
				ret += i->name();
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				ret += dependencies(*it, scope).toSet();
			}
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				ret += dependencies(*it, scope).toSet();
			}
		}	break;
		case Object::container: {
			Container *c = (Container*) o;
			
			foreach(const Object* o, c->m_params) {
				ret += dependencies(o, scope).toSet();
			}
		} break;
		case Object::apply: {
			Apply* c = (Apply*) o;
			Apply::iterator it = c->firstValue(), first = c->firstValue();
			
			QStringList newScope=scope+c->bvarStrings();
			Object* ul=c->ulimit(), *dl=c->dlimit();
			
			//uplimit and downlimit are in the parent scope
			if(ul) ret += dependencies(ul, scope).toSet();
			if(dl) ret += dependencies(dl, scope).toSet();
			for(; it!=c->m_params.end(); ++it) {
				ret += dependencies(*it, newScope).toSet();
			}
		}	break;
		case Object::none:
		case Object::value:
		case Object::oper:
			break;
	}
	
	return ret.toList();
}

bool hasTheVar(const QSet<QString> & vars, const Object * o)
{
	if(!o)
		return false;
	
	bool found=false;
	const Ci* cand;
	switch(o->type()) {
		case Object::vector: {
			const Vector *v=static_cast<const Vector*>(o);
			Vector::const_iterator it, itEnd=v->constEnd();
			for(it=v->constBegin(); it!=itEnd; ++it) {
				found |= hasTheVar(vars, *it);
			}
		}	break;
		case Object::list: {
			const List *v=static_cast<const List*>(o);
			List::const_iterator it, itEnd=v->constEnd();
			for(it=v->constBegin(); it!=itEnd; ++it) {
				found |= hasTheVar(vars, *it);
			}
		}	break;
		case Object::container: {
			const Container *c=static_cast<const Container*>(o);
			QSet<QString> bvars=c->bvarStrings().toSet(), varsCopy=vars;
			foreach(const QString &var, bvars) {
				varsCopy.remove(var);
			}
			found=hasTheVar(varsCopy, c);
		}	break;
		case Object::apply: {
			const Apply *c=static_cast<const Apply*>(o);
			QSet<QString> bvars=c->bvarStrings().toSet(), varsCopy=vars;
			foreach(const QString &var, bvars) {
				varsCopy.remove(var);
			}
			found=hasTheVar(varsCopy, c);
		}	break;
		case Object::variable:
			cand=static_cast<const Ci*>(o);
			found=vars.contains(cand->name());
			break;
		case Object::value:
		case Object::oper:
		case Object::none:
			found=false;
			break;
	}
	return found;
}

bool hasTheVar(const QSet<QString> & vars, const Container* c)
{
	bool found=false;
	if(c->containerType()!=Container::bvar) {
		Container::const_iterator it=c->constBegin(), itEnd=c->constEnd();
		for(; !found && it!=itEnd; ++it) {
			found=hasTheVar(vars, *it);
		}
	}
	return found;
}

bool hasTheVar(const QSet<QString> & vars, const Apply* a)
{
	bool found=hasTheVar(vars, a->ulimit()) || hasTheVar(vars, a->dlimit()) || hasTheVar(vars, a->domain());
	Apply::const_iterator it=a->firstValue(), itEnd=a->constEnd();
	for(; !found && it!=itEnd; ++it) {
		found=hasTheVar(vars, *it);
	}
	return found;
}

bool isLambda(const Object* o)
{
	return o->isContainer() && static_cast<const Container*>(o)->containerType()==Container::lambda;
}

bool hasVars(const Analitza::Object* o, const QStringList& bvars)
{
	Q_ASSERT(o);
	
	bool r=false;
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			r=!bvars.contains(i->name());
			
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				r |= hasVars(*it, bvars);
			}
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				r |= hasVars(*it, bvars);
			}
		}	break;
		case Object::container: {
			const Container *c = (const Container*) o;
			
			QStringList newScope=bvars+c->bvarStrings();
			Container::const_iterator it=c->m_params.constBegin(), itEnd=c->m_params.constEnd();
			if(c->containerType()==Container::declare) {
				newScope += static_cast<const Ci*>(*c->constBegin())->name();
				++it;
			}
			
			for(; it!=itEnd; ++it) {
				r |= hasVars(*it, newScope);
			}
		}	break;
		case Object::apply: {
			const Apply *c = (const Apply*) o;
			
			const QStringList scope=bvars+c->bvarStrings();
			Object* ul=c->ulimit(), *dl=c->dlimit(), *dn=c->domain();
			
			//uplimit and downlimit are in the parent scope
			if(ul) r |= hasVars(ul, bvars);
			if(dl) r |= hasVars(dl, bvars);
			if(dn) r |= hasVars(dn, bvars);
			
			Apply::const_iterator it = c->firstValue();
			for(; !r && it!=c->constEnd(); ++it) {
				r |= hasVars(*it, scope);
			}
		}	break;
		case Object::none:
		case Object::value:
		case Object::oper:
			r=false;
	}
	return r;
}

struct ObjectWalker : public ExpressionWriter
{
	ObjectWalker(const QByteArray& pref) : ind(0), m_prefix(pref) {}
	
	virtual QString accept(const Operator* root)
	{ qDebug() << prefix().constData() << "| operator: " << root->toString(); return QString(); }
	
	virtual QString accept(const Ci* var)
 	{
		QString value;
		if(var->depth()>=0)
			value="stack("+QString::number(var->depth())+')';
		else
			value="symbols";
		
		qDebug() << prefix().constData() << "| variable: " << var->name() << var->depth() << "Val:" << value;
		return QString();
	}
	
	virtual QString accept(const Cn* num)
	{ qDebug() << prefix().constData() << "| num: " << num->value() << " format: " << num->format(); return QString(); }
	
	virtual QString accept(const Container* c)
	{
		qDebug() << prefix().constData() << "| cont: " << c->tagName();// << "=" << c->toString();
		ind++;
		for(Container::const_iterator it=c->m_params.constBegin(); it<c->constEnd(); ++it)
			visitNow(*it);
		ind--;
		return QString();
	}
	
	virtual QString accept ( const Apply* c )
	{
		qDebug() << prefix().constData() << "| apply op:" << c->firstOperator().toString();
		ind++;
		if(c->ulimit()) { qDebug() << prefix().constData() << "ul: "; visitNow(c->ulimit()); }
		if(c->dlimit()) { qDebug() << prefix().constData() << "dl: "; visitNow(c->dlimit()); }
		if(!c->bvarCi().isEmpty()) { qDebug() << prefix().constData() << "bvars: " << c->bvarStrings(); }
		
		for(Container::const_iterator it=c->m_params.constBegin(); it<c->constEnd(); ++it)
			visitNow(*it);
		ind--;
		return QString();
	}
	
	virtual QString accept(const Vector* v)
	{
		qDebug() << prefix().constData() << "| vector: " << v->size();
		ind++;
		for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it)
			visitNow(*it);
		ind--;
		return QString();
	}
	
	virtual QString accept(const List* v)
	{
		qDebug() << prefix().constData() << "| list: " << v->size();
		ind++;
		for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it)
			visitNow(*it);
		ind--;
		return QString();
	}
	
	QByteArray prefix()
	{
		QByteArray ret(m_prefix);
		for(int i=0; i<ind; i++)
			ret += " |_____";
		return ret;
	}
	
	void visitNow(const Object* o) { if(o) o->visit(this); else qDebug() << prefix().constData() << "Null" ;}
	
	QString result() const { return QString(); }
	
	int ind;
	QByteArray m_prefix;
};

void objectWalker(const Analitza::Expression& o, const QByteArray& prefix)
{
	objectWalker(o.tree(), prefix);
}

void objectWalker(const Object* root, const QByteArray& prefix)
{
	ObjectWalker o(prefix);
	o.visitNow(root);
	
	qDebug() << prefix.constData() << ';';
}

bool equalTree(const Object * o1, const Object * o2)
{
	Q_ASSERT(o1 && o2);
	if(o1==o2)
		return true;
	else if(o1->type()!=o2->type())
		return false;
	
	bool eq;
	switch(o2->type()) {
		case Object::variable:
			eq = *static_cast<const Ci*>(o1)==*static_cast<const Ci*>(o2);
			break;
		case Object::value:
			eq = *static_cast<const Cn*>(o1)==*static_cast<const Cn*>(o2);
			break;
		case Object::container:
			eq = *static_cast<const Container*>(o1)==*static_cast<const Container*>(o2);
			break;
		case Object::oper:
			eq = *static_cast<const Operator*>(o1)==*static_cast<const Operator*>(o2);
			break;
		case Object::vector:
			eq = *static_cast<const Vector*>(o1)==*static_cast<const Vector*>(o2);
			break;
		case Object::list:
			eq = *static_cast<const List*>(o1)==*static_cast<const List*>(o2);
			break;
		case Object::apply:
			eq = *static_cast<const Apply*>(o1)==*static_cast<const Apply*>(o2);
			break;
		case Object::none:
			eq=false;
			Q_ASSERT(false && "Should not get here");
			break;
	}
	return eq;
}

QVariant expressionToVariant(const Analitza::Expression& res)
{
	QVariant ret;
	if(res.isVector() || res.isList()) {
		QVariantList vals;
		
		QList<Analitza::Expression> expressions = res.toExpressionList();
		foreach(const Analitza::Expression& exp, expressions) {
			vals << expressionToVariant(exp);
		}
		
		ret = vals;
	} else if(res.isReal()) {
		Analitza::Cn val = res.toReal();
		if(val.isBoolean())
			ret = val.isTrue();
		else if(val.isInteger())
			ret = int(val.value());
		else
			ret = val.value();
	} else
		ret = res.toString();
	
	return ret;
}

Analitza::Expression variantToExpression(const QVariant& v)
{
	if(v.canConvert(QVariant::Double))
		return Analitza::Expression(Analitza::Cn(v.toReal()));
	else if(v.canConvert(QVariant::List)) {
		QVariantList list = v.toList();
		QList<Analitza::Expression> expressionList;
		
		foreach(const QVariant& elem, list) {
			expressionList << variantToExpression(elem);
		}
		
		return Analitza::Expression::constructList(expressionList);
	} else if(v.canConvert(QVariant::String))
		return Analitza::Expression(v.toString());
	
	Q_ASSERT(false && "couldn't figure out the type");
	return Analitza::Expression();
}

}
