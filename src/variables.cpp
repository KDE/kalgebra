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

#include "variables.h"
Variables::Variables() : QHash<QString, Object*>()
{
	insert("true", new Cn(1.));
	insert("false", new Cn(0.));
	insert("pi", new Cn(3.1415926535897932384626433));
	insert("e", new Cn(2.718281828));
	insert("euler", new Cn(0.5772156649));
// 	modify("perimeter", Expression("r->2*pi*r", false));
}

Variables::Variables(const Variables& v) : QHash<QString, Object*>(v)
{
	QHash<QString, Object*>::iterator i;
	for (i = this->begin(); i != this->end(); i++)
		*i = Expression::objectCopy(*i);
}

Variables::~Variables()
{
	QHash<QString, Object*>::iterator i;
	for (i = this->begin(); i != this->end(); i++)
		delete *i;
}

void Variables::modify(const QString & name, const Expression & o)
{
	modify(name, o.m_tree);
}

void Variables::modify(const QString& name, const Object* o)
{
	if(contains(name))
		delete value(name);
	
	insert(name, Expression::objectCopy(o));
}


bool Variables::rename(const QString& orig, const QString& dest)
{
	bool existed = contains(orig);
	Object *aux = NULL;
	
	if(existed) {
		aux = take(orig);
		insert(dest, aux);
	}
	
	return existed;
}

bool Variables::destroy(const QString& s)
{
	delete take(s);
	return true;
}

void Variables::stack(const QString &name, const Object *o)
{
	insert(name, Expression::objectCopy(o));
}
