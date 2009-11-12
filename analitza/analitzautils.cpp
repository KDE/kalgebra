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

using namespace Analitza;
namespace AnalitzaUtils
{
	
QStringList dependencies(const Object* o, const QStringList& scope)
{
	Q_ASSERT(o);
	
	QStringList ret;
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			if(!scope.contains(i->name()))
				ret += i->name();
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				ret += dependencies(*it, scope);
			}
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				ret += dependencies(*it, scope);
			}
		}	break;
		case Object::container: {
			Container *c = (Container*) o;
			Container::iterator it = c->firstValue(), first = c->firstValue();
			
			QStringList newScope=scope+c->bvarStrings();
			Object* ul=c->ulimit(), *dl=c->dlimit();
			
			//uplimit and downlimit are in the parent scope
			if(ul) ret += dependencies(ul, scope);
			if(dl) ret += dependencies(dl, scope);
			
			for(; it!=c->m_params.end(); ++it) {
				ret += dependencies(*it, newScope);
			}
		} break;
		case Object::none:
		case Object::value:
		case Object::oper:
			break;
	}
	
	return ret;
}

bool hasTheVar(const QSet<QString> & vars, const Object * o)
{
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

bool hasTheVar(const QSet<QString> & vars, const Container * c)
{
	bool found=false;
	if(c->containerType()!=Container::bvar) {
		Container::const_iterator it=c->firstValue(), itEnd=c->m_params.constEnd();
		for(; !found && it!=itEnd; ++it) {
			if(hasTheVar(vars, *it))
				found=true;
		}
	}
	return found;
}

bool isLambda(const Object* o)
{
	return o->isContainer() && static_cast<const Container*>(o)->containerType()==Container::lambda;
}

bool hasVars(const Object *o, const QString &var, const QStringList& bvars, const Variables* vars)
{
	Q_ASSERT(o);
	
	bool r=false;
	switch(o->type()) {
		case Object::variable: {
			Ci *i = (Ci*) o;
			
			r=((i->name()==var) || var.isEmpty()) && !bvars.contains(i->name());
			
			if(r && vars && !var.isEmpty() && vars->contains(i->name()))
				r=hasVars(vars->value(i->name()), var, bvars, vars);
		}	break;
		case Object::vector: {
			Vector *v=(Vector*) o;
			for(Vector::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				r |= hasVars(*it, var, bvars, vars);
			}
		}	break;
		case Object::list: {
			List *v=(List*) o;
			for(List::const_iterator it=v->constBegin(); it!=v->constEnd(); ++it) {
				r |= hasVars(*it, var, bvars, vars);
			}
		}	break;
		case Object::container: {
			const Container *c = (const Container*) o;
			
			const QStringList scope=bvars+c->bvarStrings();
			Object* ul=c->ulimit(), *dl=c->dlimit();
			
			//uplimit and downlimit are in the parent scope
			if(ul) r |= hasVars(ul, var, bvars, vars);
			if(dl) r |= hasVars(dl, var, bvars, vars);
			
			Container::const_iterator it = c->firstValue();
			for(; !r && it!=c->m_params.constEnd(); ++it) {
				r |= hasVars(*it, var, scope, vars);
			}
		} break;
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
// 		QString value=(var->isDefined() ? (var->value() ? /*var->value()->toString()*/"def" : "*0" ) : "null");
		qDebug() << prefix().constData() << "| variable: " << var->name() << "Val:" << QString(); return QString(); }
	
	virtual QString accept(const Cn* num)
	{ qDebug() << prefix().constData() << "| num: " << num->value() << " format: " << num->format(); return QString(); }
	
	virtual QString accept(const Container* c)
	{
		qDebug() << prefix().constData() << "| cont: " << c->tagName();// << "=" << c->toString();
		ind++;
		for(Container::const_iterator it=c->m_params.constBegin(); it<c->m_params.constEnd(); ++it)
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
	
	void visitNow(const Object* o) { if(o) o->visit(this); else qDebug() << prefix() << "Null" ;}
	
	QString result() const { return QString(); }
	
	int ind;
	QByteArray m_prefix;
};

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
		case Object::none:
			eq=false;
			Q_ASSERT(false && "Should not get here");
			break;
	}
	return eq;
}
}
