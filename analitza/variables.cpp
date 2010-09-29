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
#include "expression.h"
#include "value.h"
#include "container.h"

using namespace Analitza;

Variables::Variables() : QHash<QString, Object*>()
{
	initializeConstants();
}

void Variables::initializeConstants()
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
	qDeleteAll(*this);
}

void Variables::modify(const QString & name, const Expression & e)
{
	const Analitza::Object* o=e.tree();
	if(o->isContainer() && static_cast<const Container*>(o)->containerType()==Container::math) {
		o=*static_cast<const Container*>(o)->constBegin();
	}
	modify(name, o);
}

Cn* Variables::modify(const QString & name, const double & d)
{
	Cn* val=new Cn(d);
	modify(name, val);
	return val;
}

void Variables::modify(const QString& name, const Object* o)
{
	delete value(name);
	
	insert(name, o->copy());
}

void Variables::rename(const QString& orig, const QString& dest)
{
	Q_ASSERT(contains(orig));
	insert(dest, take(orig));
}

Expression Variables::valueExpression(const QString& name) const
{
	return Expression(value(name)->copy());
}
