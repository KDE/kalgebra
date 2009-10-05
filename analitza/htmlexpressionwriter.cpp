/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "htmlexpressionwriter.h"
#include "value.h"
#include "operator.h"
#include "container.h"
#include "vector.h"
#include <QStringList>
#include <KLocale>
#include "list.h"
#include "variable.h"

QString oper(const QString& op) { return i18nc("html representation of an operator", "<span class='op'>%1</span>", op); }
QString oper(const QChar& op) { return i18nc("html representation of an operator", "<span class='op'>%1</span>", op); }

HtmlExpressionWriter::HtmlExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString HtmlExpressionWriter::accept(const Vector* vec)
{
	QStringList elements;
	for(Vector::const_iterator it=vec->constBegin(); it!=vec->constEnd(); ++it)
	{
		elements += (*it)->visit(this);
	}
	return "vector "+oper("{ ")+elements.join(QString(oper(", ")))+oper(" }");
}

QString HtmlExpressionWriter::accept(const List* vec)
{
	QStringList elements;
	for(List::const_iterator it=vec->constBegin(); it!=vec->constEnd(); ++it)
	{
		elements += (*it)->visit(this);
	}
	return "list "+oper("{ ")+elements.join(QString(oper(", ")))+oper(" }");
}

QString HtmlExpressionWriter::accept(const Ci* var)
{
	return var->toHtml();
}

QString HtmlExpressionWriter::accept(const Cn* val)
{
	if(val->isBoolean()) {
		if(val->isTrue())
			return i18nc("html representation of a true. please don't translate the true for consistency", "<span class='const'>true</span>");
		else
			return i18nc("html representation of a false. please don't translate the false for consistency", "<span class='const'>false</span>");
	} else
		return i18nc("html representation of a number", "<span class='num'>%1</span>", val->value());
}

QString HtmlExpressionWriter::accept(const Operator* op)
{
	return op->name();
}

QString HtmlExpressionWriter::accept(const Container* var)
{
	QStringList ret;
	Operator *op=0;
	QString bounds;
	QStringList bvars;
	
	for(int i=0; i<var->m_params.count(); i++) {
		Q_ASSERT(var->m_params[i]!=0);
		
		if(var->m_params[i]->type() == Object::oper)
			op = (Operator*) var->m_params[i];
		else if(var->m_params[i]->type() == Object::variable) {
			Ci *b = (Ci*) var->m_params[i];
			ret << b->visit(this);
		} else if(var->m_params[i]->type() == Object::container) {
			Container *c = (Container*) var->m_params[i];
			QString s = c->visit(this);
			Operator child_op = c->firstOperator();
			
			if(op!=0 && child_op.operatorType() && op->weight()>=child_op.weight() && op->nparams()!=1) { //apply
				s=oper('(')+s+oper(')');
			}
			
			if(c->containerType() == Container::bvar) { //bvar
				Object *ul = var->ulimit(), *dl = var->dlimit();
				if(dl)
					bounds += dl->visit(this);
				if(dl || ul)
					bounds += oper("..");
				if(ul)
					bounds += ul->visit(this);
				
				bvars += s;
			}
			else if(c->containerType()!=Container::uplimit && c->containerType()!=Container::downlimit)
				ret << s;
		} else 
			ret << var->m_params[i]->visit(this);
	}
	bool func=!op || (op->operatorType()==Operator::function);
	
	QString toret;
	switch(var->containerType()) {
		case Container::declare:
			toret += ret.join(oper(":="));
			break;
		case Container::lambda: {
			QString last=ret.takeLast();
			if(bvars.count()!=1) toret +=oper('(');
			toret += bvars.join(oper(", "));
			if(bvars.count()!=1) toret +=oper(')');
			if(!bounds.isEmpty()) toret+=oper('=')+bounds;
			toret += oper("->") + last;
		}	break;
		case Container::math:
			toret += ret.join(oper(';'));
			break;
		case Container::apply:
			if(func){
				QString n = ret.takeFirst();
				if(var->m_params.first()->type()!=Object::variable)
					n='('+n+')';
				
				toret += n+oper('(')+ret.join(oper(", "))+oper(')');
			} else if(op==0)
				toret += ret.join(" ");
			else switch(op->operatorType()) {
				case Operator::plus:	toret += ret.join(oper('+')); break;
				case Operator::times:	toret += ret.join(oper('*')); break;
				case Operator::divide:	toret += ret.join(oper('/')); break;
				case Operator::eq:		toret += ret.join(oper('=')); break;
				case Operator::neq:		toret += ret.join(oper("!=")); break;
				case Operator::lt:		toret += ret.join(oper("&lt;"));  break;
				case Operator::leq:		toret += ret.join(oper("&lt;=")); break;
				case Operator::gt:		toret += ret.join(oper("&gt;"));  break;
				case Operator::geq:		toret += ret.join(oper("&gt;=")); break;
				case Operator::minus:
					if(ret.count()==1)
						toret += oper('-')+ret[0];
					else
						toret += ret.join(oper('-'));
					break;
				case Operator::power:
					toret += ret.join(oper('^'));
					break;
				default: {
					QString bounding;
					if(!bounds.isEmpty() || !bvars.isEmpty()) {
						if(bvars.count()!=1) bounding +=oper('(');
						bounding += bvars.join(oper(", "));
						if(bvars.count()!=1) bounding +=oper(')');
						
						bounding = oper(':')+bounding;
						if(!bounds.isEmpty())
							bounding+=oper('=')+bounds;
					}
						
					toret += QString("%1(%2%3)").arg(op->visit(this)).arg(ret.join(oper(", "))).arg(bounding);
				}	break;
			}
			break;
		case Container::uplimit: //x->(n1..n2) is put at the same time
		case Container::downlimit:
			break;
		case Container::bvar:
			if(ret.count()>1) toret += oper('(');
			toret += ret.join(oper(", "));
			if(ret.count()>1) toret += oper(')');
			break;
		case Container::piece:
			toret += ret[1]+oper(" ? ")+ret[0];
			break;
		case Container::otherwise:
			toret += oper(oper("? "))+ret[0];
			break;
		default:
			toret += var->tagName()+oper(" { ")+ret.join(oper(", "))+oper(" }");
			break;
	}
	return toret;
}
