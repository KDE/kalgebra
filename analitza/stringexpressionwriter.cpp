/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include "stringexpressionwriter.h"
#include "value.h"
#include "operator.h"
#include "container.h"
#include <QStringList>

StringExpressionWriter::StringExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString StringExpressionWriter::accept(const Ci* var)
{
	return var->name();
}

QString StringExpressionWriter::accept(const Operator* op)
{
	return op->name();
}

QString StringExpressionWriter::accept(const Cn* var)
{
	if(var->isBoolean()) {
		if(var->isTrue())
			return "true";
		else
			return "false";
	} else
		return QString("%1").arg(var->value(), 0, 'g', 12);
}

QString StringExpressionWriter::accept(const Container* var)
{
	QStringList ret;
	bool func=false, bounded=false;
	Operator *op=0;
	
	for(int i=0; i<var->m_params.count(); i++) {
		Q_ASSERT(var->m_params[i]!=0);
		
		if(var->m_params[i]->type() == Object::oper)
			op = (Operator*) var->m_params[i];
		else if(var->m_params[i]->type() == Object::variable) {
			Ci *b = (Ci*) var->m_params[i];
			func|=b->isFunction();
			ret << b->visit(this);
		} else if(var->m_params[i]->type() == Object::container) {
			Container *c = (Container*) var->m_params[i];
			QString s = c->visit(this);
			Operator child_op = c->firstOperator();
			
			if(op!=0 && child_op.operatorType() && op->weight()>=child_op.weight() && op->nparams()!=1) { //apply
				s=QString("(%1)").arg(s);
			}
			
			if(c->containerType()!=Container::uplimit && c->containerType()!=Container::downlimit)
				ret << s;
			
			if(c->containerType() == Container::bvar) { //bvar
				bounded=true;
				QString bounds;
				Container *ul = var->ulimit(), *dl = var->dlimit();
				if(dl)
					bounds += dl->visit(this);
				if(dl || ul)
					bounds += "..";
				if(ul)
					bounds += ul->visit(this);
				if(!bounds.isEmpty())
					ret << bounds;
			}
		} else 
			ret << var->m_params[i]->visit(this);
	}
	
	QString toret;
	switch(var->containerType()) {
		case Container::declare:
			toret += ret.join(":=");
			break;
		case Container::lambda:
		{
			QString res=ret.takeFirst();
			if(ret.count()==1)
				res=res+ret.first();
			else
				res=res+'('+ret.join(", ")+')';
			toret += res;
		}	break;
		case Container::math:
			toret += ret.join("; ");
			break;
		case Container::apply:
			if(func){
				QString n = ret.takeFirst();
				toret += QString("%1(%2)").arg(n).arg(ret.join(", "));
			} else if(op==0)
				toret += ret.join(" ");
			else switch(op->operatorType()) {
				case Operator::plus:
					toret += ret.join("+");
					break;
				case Operator::times:
					toret += ret.join("*");
					break;
				case Operator::divide:
					toret += ret.join("/");
					break;
				case Operator::minus:
					if(ret.count()==1)
						toret += '-'+ret[0];
					else
						toret += ret.join("-");
					break;
				case Operator::power:
					toret += ret.join("^");
					break;
				default:
					if(bounded) {
						QString bounding=ret.takeFirst();
						ret[0]=bounding+ret[0];
					}
						
					toret += QString("%1(%2)").arg(op->visit(this)).arg(ret.join(", "));
					break;
			}
			break;
		case Container::uplimit: //x->(n1..n2) is put at the same time
		case Container::downlimit:
			break;
		case Container::bvar:
			toret += ret.join("->")+"->";
			break;
		case Container::piece:
			toret += ret[1]+" ? "+ret[0];
			break;
		case Container::otherwise:
			toret += "? "+ret[0];
			break;
		default:
			toret += var->tagName()+" { "+ret.join(", ")+" }";
			break;
	}
	return toret;
}
