/***************************************************************************
 *   Copyright (C) 2006 by Aleix Pol                                       *
 *   aleixpol@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CONTAINER_H
#define CONTAINER_H

#include <QtXml>

#include "object.h"
#include "value.h"
#include "operator.h"
/**
	@author Aleix Pol <aleixpol@gmail.com>
*/

void print_dom(QDomNode in, int ind);
void objectWalker(const Object*, int a=0);

class Container : public Object
{
	public:
	Container() : Object(container), m_cont_type(cnone) { }
	Container(enum ContainerType c) : Object(container), m_cont_type(c) { }
	Container(const Object *o);
	Container(const Container& c);
	virtual ~Container() { qDeleteAll(m_params); }
	
	void setContainerType(enum ContainerType c) { m_cont_type = c; }
	enum ContainerType containerType() const { return m_cont_type; }
	
	bool operator==(const Container& c) const;
	static enum ContainerType toContainerType(const QString& tag);
	static bool equalTree(Object const* o1, Object const * o2);
	void appendBranch(Object* o) { m_params.append(o); }
	QStringList bvarList() const;
	bool hasVars() const;
	Operator firstOperator() const;
	QList<Object*>::iterator firstValue();
	QList<Object*> copyParams() const;
	QString toString() const;
	QString toMathML() const;
	Container* ulimit() const;
	Container* dlimit() const;
	
	//Monomials
	Container(Cn* base, Object* var, Ci* degree);
	static Cn* monomialDegree(const Container&);
	static Cn* monomialBase(const Container&);
	static Object* monomialVar(const Container&);
// private:
	enum ContainerType m_cont_type;
	QList<Object*> m_params;
};



#endif
