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

#ifndef VARIABLES_H
#define VARIABLES_H

#include <QHash>
#include "container.h"

/**
	@author Aleix Pol <aleixpol@gmail.com>
*/
class Variables : public QHash<QString, Object*>
{
public:
	Variables();
	Variables(const Variables& v);
	~Variables();
	
	void modify(const QString&, const Object*);
	void stack(const QString&, const Object*);
	void modify(const QString& s, const double& d) { modify(s, new Cn(d)); }
	bool rename(const QString&, const QString&);
	bool destroy(const QString&);
	
};

#endif
