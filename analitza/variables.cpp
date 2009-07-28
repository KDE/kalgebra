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

#include "variables.h"
#include "container.h"
#include "expression.h"

Variables::Variables() : QHash<QString, Object*>()
{
	insert("true", new Cn(true));
	insert("false", new Cn(false));
	insert("pi", new Cn(Cn::pi()));
	insert("e", new Cn(Cn::e()));
	insert("euler", new Cn(Cn::euler()));
}

Variables::Variables(const Variables& v) : QHash<QString, Object*>(v)
{
	QHash<QString, Object*>::iterator i;
	for (i = this->begin(); i != this->end(); ++i)
		*i = (*i)->copy();
}

Variables::~Variables()
{
	QHash<QString, Object*>::iterator i;
	for (i = this->begin(); i != this->end(); ++i)
		delete *i;
}

void Variables::modify(const QString & name, const Expression & o)
{
	modify(name, o.tree());
}

void Variables::modify(const QString & name, const double & d)
{
	if(contains(name))
		delete value(name);
	
	insert(name, new Cn(d));
}

void Variables::modify(const QString& name, const Object* o)
{
	if(contains(name))
		delete value(name);
	
	insert(name, o->copy());
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
	insertMulti(name, o->copy());
}

void Variables::stack(const QString & name, double n)
{
	insertMulti(name, new Cn(n));
}
