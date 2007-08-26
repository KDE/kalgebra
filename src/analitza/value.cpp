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

#include "value.h"
#include "operator.h"
#include "variables.h"

#include <KLocale>

Cn::Cn(Object const *o)
	: Object(Object::value), m_value(0.), m_boolean(false)
{
	if(o->type()==Operator::value){
		Cn *v = (Cn*) o;
		m_value = v->value();
		m_boolean = v->isBoolean();
		setCorrect(v->isCorrect());
	} else {
		setCorrect(false);
	}
}

QString Cn::toString() const
{
	if(isBoolean()) {
		if(isTrue())
			return "true";
		else
			return "false";
	} else
		return QString("%1").arg(m_value, 0, 'g', 12);
}

QString Cn::toMathML() const
{
	if(isBoolean()) {
		if(isTrue())
			return "<cn type='constant'>true</cn>";
		else
			return "<cn type='constant'>false</cn>";
	} else
		return QString("<cn>%1</cn>").arg(m_value, 0, 'g', 12);
}

QString Cn::toHtml() const
{
	if(isBoolean()) {
		if(isTrue())
			return i18nc("html representation of a number", "<span class='const'>true</span>");
		else
			return i18nc("html representation of a number", "<span class='const'>false</span>");
	} else
		return i18nc("html representation of a number", "<span class='num'>%1</span>", m_value);
}

/*enum Cn::ValueFormat Cn::whatValueFormat(const QDomElement& val)
{
	enum ValueFormat ret= none;
	QString tag = val.tagName();
	if(tag == "cn"){ // a is a number
		if(val.attribute("type","real") == "real")		ret = real;
		else if(val.attribute("type") == "integer")		ret = integer;
		else if(val.attribute("type") == "e-notation")		ret = real;
		else if(val.attribute("type") == "rational")		ret = real;
		else if(val.attribute("type") == "complex-cartesian")	ret = none;
		else if(val.attribute("type") == "complex-polar")	ret = none;
		else if(val.attribute("type") == "constant") {
			if(val.text() == "&pi;")		ret=real;
			else if (val.text() == "&ee;" || val.text() == "&ExponentialE;")	ret = real;
			else if (val.text() == "&true;")	ret = boolean;
			else if (val.text() == "&false;")	ret = boolean;
			else if (val.text() == "&gamma;")	ret = real;
			//else if (val.text() == "&ImagniaryI;")	; //TODO: Not implemented 
			else if (val.text() == "&infin;")	ret=nan;
			else if (val.text() == "&NaN;")		ret=nan;
		}
	} else if(tag=="true")		ret = boolean;
	else if(tag=="false")		ret = boolean;
	else if(tag=="pi")		ret = real;
	else if(tag=="exponentiale")	ret = real;
	else if(tag=="eulergamma")	ret = real;
	
	return ret;
}*/

void Cn::setValue(const QDomElement& val)
{
// 	this->m_vformat=whatValueFormat(val);
	bool wrong;
	QString tag = val.tagName();
	m_boolean=false;
	
	if(tag == "cn"){ // a is a number
		if(val.attribute("type", "integer") == "real") {
			m_value= val.text().trimmed().toDouble(&wrong); //TODO: Base on double not implemented
		} else if(val.attribute("type", "integer") == "integer"){
			int base = val.attribute("base", "10").toInt(NULL, 10);
			m_value= val.text().trimmed().toInt(&wrong, base);
		}
#if 0
		else if(val.attribute("type") == "e-notation")	{ /*TODO: Not implemented */ }
		else if(val.attribute("type") == "rational")		{ /*TODO: Not implemented */ }
		else if(val.attribute("type") == "complex-cartesian")	{ /*TODO: Not implemented */ }
		else if(val.attribute("type") == "complex-polar")	{ /*TODO: Not implemented */ }
#endif
		else if(val.attribute("type") == "constant"){
			if(val.text() == "&pi;")			{ m_value = Variables::pi().m_value; }
			else if (val.text() == "&ee;" || val.text() == "&ExponentialE;"){ m_value = Variables::e().m_value; }
			else if (val.text() == "&true;")	{ m_value=1.; m_boolean=true; }
			else if (val.text() == "&false;")	{ m_value=0.; m_boolean=true; }
			else if (val.text() == "&gamma;")	{ m_value = 0.5772156649; }
#if 0
			else if (val.text() == "&ImagniaryI;")	; //TODO: Not implemented 
			else if (val.text() == "&infin;")	; //TODO: Not implemented  }
			else if (val.text() == "&NaN;")		; //TODO: Not implemented  }*/
#endif
		}
	}
}
