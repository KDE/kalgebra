/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef VECTOR_H
#define VECTOR_H

#include "object.h"
#include "analitzaexport.h"

class ANALITZA_EXPORT Vector : public Object
{
	public:
		typedef QVector<Object*>::iterator iterator;
		typedef QVector<Object*>::const_iterator const_iterator;
		
		Vector(const Vector& v);
		Vector(int size);
		virtual ~Vector();
		
		void appendBranch(Object* );
		int size() const { return m_elements.size(); }
		
		iterator erase(const iterator& it) { return m_elements.erase(it); }
		
		iterator begin() { return m_elements.begin(); }
		iterator end() { return m_elements.end(); }
		const_iterator constBegin() const { return m_elements.constBegin(); }
		const_iterator constEnd() const { return m_elements.constEnd(); }
		
		Object* at(int i) const { return m_elements.at(i); }
		
		virtual QString visit (ExpressionWriter* e) const;
		virtual bool isCorrect() const;
		virtual void negate ();
		virtual bool isZero() const;
		
		virtual bool matches(const Object* pattern, QMap< QString, const Object* >* found) const;
		virtual Object* copy() const;
		bool operator==(const Vector& v) const;
	private:
		QVector<Object*> m_elements;
};

#endif // VECTOR_H
