/*************************************************************************************
 *  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef LIST_H
#define LIST_H

#include "object.h"
#include "analitzaexport.h"

class ANALITZA_EXPORT List : public Object
{
	public:
		typedef QList<Object*>::iterator iterator;
		typedef QList<Object*>::const_iterator const_iterator;
		
		List(const List& v);
		List();
		virtual ~List();
		
		void appendBranch(Object* );
		int size() const { return m_elements.size(); }
		
		iterator erase(const iterator& it) { return m_elements.erase(it); }
		
		iterator begin() { return m_elements.begin(); }
		iterator end() { return m_elements.end(); }
		const_iterator constBegin() const { return m_elements.constBegin(); }
		const_iterator constEnd() const { return m_elements.constEnd(); }
		
		Object* at(int i) const { return m_elements.at(i); }
		void setAt(int i, Object* o) { m_elements[i]=o; }
		
		virtual QString visit (ExpressionWriter* e) const;
		virtual bool isCorrect() const;
		virtual void negate();
		virtual bool isZero() const;
		
		virtual bool matches(const Object* pattern, QMap< QString, const Object* >* found) const;
		virtual Object* copy() const;
		bool operator==(const List& v) const;
		
	private:
		QList<Object*> m_elements;
};

#endif // VECTOR_H
