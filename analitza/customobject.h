/*************************************************************************************
 *  Copyright (C) 2011 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef CUSTOMOBJECT_H
#define CUSTOMOBJECT_H

#include "object.h"
#include "analitzaexport.h"

#include <cmath>
#include <QVariant>

class QDomElement;

namespace Analitza
{

/**
*	The CustomObject class is the one that represents a value in the expression trees.
*	@author Aleix Pol <aleixpol@kde.org>  
*/

class ANALITZA_EXPORT CustomObject : public Object
{
	public:
		explicit CustomObject(const QVariant& v) : Object(Object::custom), m_value(v) {}
		virtual Object* copy() const;
		virtual bool matches(const Analitza::Object* exp, QMap< QString, const Analitza::Object* >* found) const;
		virtual QString visit(ExpressionWriter* exp) const;
		
		bool operator==(const CustomObject& obj) const;
		QVariant value() const { return m_value; }
		
	private:
		QVariant m_value;
		
};

}

#endif
