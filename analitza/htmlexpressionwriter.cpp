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

#include "htmlexpressionwriter.h"
#include "value.h"
#include "operator.h"
#include "container.h"
#include <QStringList>
#include <KLocale>

HtmlExpressionWriter::HtmlExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString HtmlExpressionWriter::accept(const Ci* var)
{
	return var->name();
}

QString HtmlExpressionWriter::accept(const Cn* val)
{
	if(val->isBoolean()) {
		if(val->isTrue())
			return i18nc("html representation of a number", "<span class='const'>true</span>");
		else
			return i18nc("html representation of a number", "<span class='const'>false</span>");
	} else
		return i18nc("html representation of a number", "<span class='num'>%1</span>", val->value());
}

QString HtmlExpressionWriter::accept(const Operator* op)
{
	return op->name();
}

QString HtmlExpressionWriter::accept(const Container* c)
{
	bool bounded=false;
	QStringList ret;
	bool func=false;
	
	Operator *op=0;
	for(int i=0; i<c->m_params.count(); i++) {
		Q_ASSERT(c->m_params[i]!=0);
		
		if(c->m_params[i]->type() == Object::oper)
			op = (Operator*) c->m_params[i];
		else if(c->m_params[i]->type() == Object::variable) {
			Ci *b = (Ci*) c->m_params[i];
			func|=b->isFunction();
			ret << b->visit(this);
		} else if(c->m_params[i]->type() == Object::container) {
			Container *c1 = (Container*) c->m_params[i];
			QString s = c1->visit(this);
			Operator child_op = c1->firstOperator();
			if(op!=0 && op->weight()>child_op.weight() && op->nparams()!=1)
				s=i18n("<span class='op'>(</span>%1<span class='op'>)</span>", s);
			
			if(c1->containerType()!=Container::uplimit && c1->containerType()!=Container::downlimit)
				ret << s;
			
			if(c1->containerType() == Container::bvar) { //bvar
				bounded=true;
				QString bounds;
				Container *ul = c->ulimit(), *dl = c->dlimit();
				if(dl)
					bounds += dl->visit(this);
				if(dl || ul)
					bounds += i18n("<span class='op'>..</span>");
				if(ul)
					bounds += ul->visit(this);
				if(!bounds.isEmpty())
					ret << bounds;
			}
		} else 
			ret << c->m_params[i]->visit(this);
	}
	
	QString toret;
	switch(c->containerType()) {
		case Container::declare:
			toret += ret.join(i18n("<span class='op'>:=</span>"));
			break;
		case Container::lambda:
			{
			QString res=ret.takeFirst();
			if(ret.count()==1)
				res=res+ret.first();
			else
				res=res+"<span class='op'>(</span>"+ret.join(", ")+"<span class='op'>)</span>";
			toret += res;
		}	break;
		case Container::math:
			toret += ret.join(i18nc("Not really correct", "<span class='op'>,</span> "));
			break;
		case Container::apply:
			if(func){
				QString n = ret.takeFirst();
				toret += QString("<span class='func'>%1</span><span class='op'>(</span>"
								"%2<span class='op'>)</span>").arg(n).arg(ret.join(", "));
			} else if(op==0)
				toret += ret.join(" ");
			else switch(op->operatorType()) {
				case Operator::plus:
					toret += ret.join(i18n("<span class='op'>+</span>"));
					break;
				case Operator::times:
					toret += ret.join(i18n("<span class='op'>*</span>"));
					break;
				case Operator::divide:
					toret += ret.join(i18n("<span class='op'>/</span>"));
					break;
				case Operator::minus:
					if(ret.count()==1)
						toret += i18n("<span class='op'>-</span>%1", ret.first());
					else
						toret += ret.join(i18n("<span class='op'>-</span>"));
					break;
				case Operator::power:
					toret += ret.join(i18n("<span class='op'>^</span>"));
					break;
				default:
					if(bounded) {
						QString bounding=ret.takeFirst();
						ret[0]=bounding+ret[0];
					}
					toret += i18n("<span class='func'>%1</span><span class='op'>(</span>%2<span class='op'>)</span>",
								  op->visit(this), ret.join(", "));
					break;
			}
				break;
		case Container::uplimit: //x->(n1..n2) is put at the same time
		case Container::downlimit:
			break;
		case Container::bvar:
			toret += ret.join(i18n("<span class='op'>-&gt;</span>"))+i18n("<span class='op'>-&gt;</span>");
			break;
		case Container::piece:
            toret += i18n("%1 <span class='op'>?</span> %2", ret[1], ret[0]);
			break;
		case Container::otherwise:
			toret += i18n("<span class='op'>?</span> %1", ret[0]);
			break;
		default:
			toret += i18n("<span class='cont'>%1</span><span class='op'> { </span>%2<span class='op'> }</span>",
						  c->tagName(), ret.join(i18n("<span class='op'>,</span> ")));
			break;
	}
	return toret;
}
