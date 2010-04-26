/*************************************************************************************
 *  Copyright (C) 2007-2010 by Aleix Pol <aleixpol@kde.org>                          *
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

#ifndef APPLY_H
#define APPLY_H
#include "object.h"
#include "analitzaexport.h"
#include "operator.h"

namespace Analitza
{
class Operator;
class Ci;

/**
 *	This class is the one that will correspond to MathML apply tags.
 *	Stores and helps to retrieve any data it has inside like bvars, operators and values
 *	@author Aleix Pol <aleixpol@kde.org>  
 */

class ANALITZA_EXPORT Apply : public Object
{
	public:
		Apply();
		virtual ~Apply();
		
		typedef QList<Object*>::const_iterator const_iterator;
		typedef QList<Object*>::iterator iterator;
		
		virtual Apply* copy() const;
		virtual bool decorate(const Analitza::Object::ScopeInformation& scope);
		virtual bool matches(const Analitza::Object* exp, QMap< QString, const Analitza::Object* >* found) const;
		virtual QString visit(ExpressionWriter* exp) const;
		Operator firstOperator() const { if(m_op) return *m_op; else return Operator(Operator::function); }
		int countValues() const { return m_params.size(); }
		
		void appendBranch(Object* o);
		
		bool hasBVars() const { return !m_bvars.isEmpty(); }
		QStringList bvarStrings() const;
		Object* ulimit() const { return m_ulimit; }
		Object* dlimit() const { return m_dlimit; }
		Object* domain() const { return m_domain; }
		
		Object*& ulimit() { return m_ulimit; }
		Object*& dlimit() { return m_dlimit; }
		Object*& domain() { return m_domain; }
		iterator firstValue() { return m_params.begin(); }
		const_iterator firstValue() const { return m_params.constBegin(); }
		const_iterator constEnd() const { return m_params.constEnd(); }
		QList<Ci*> bvarCi() const { return m_bvars; }
		bool isUnary() const { return m_params.size()==1; }
		bool isEmpty() const { return m_params.isEmpty(); }
		bool operator==(const Apply& a) const;
		
		/** Adds a @p o branch right after @p before of the Container. */
		void insertBranch(Apply::iterator before, Object* o) { m_params.insert(before, o); }
		QList<Object*> values() const { return m_params; }
		
		QList<Object*> m_params;
	private:
		Apply(const Apply& );
		
		Object* m_ulimit, *m_dlimit, *m_domain;
		QList<Ci*> m_bvars;
		Operator* m_op;
};

}
#endif // APPLY_H
