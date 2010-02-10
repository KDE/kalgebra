/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef EXPRESSIONTYPECHECKER_H
#define EXPRESSIONTYPECHECKER_H

#include "analitzaexport.h"
#include "expressionwriter.h"
#include "analitzaexport.h"
#include "expression.h"
#include <QStack>

namespace Analitza
{
class Variables;

class ANALITZA_EXPORT ExpressionType
{
	public:
		enum Type { Undefined=0, Value, Vector, List };
		QString toString() const;
		
		ExpressionType(Type t=Undefined) : m_type(t), m_contained(0), m_size(-1) {}
		ExpressionType(Type t, const ExpressionType& contained, int s=-1) : m_type(t), m_contained(new ExpressionType(contained)), m_size(s) {}
		
		ExpressionType(const ExpressionType& t) : m_type(t.m_type), m_contained(0), m_size(t.m_size)
			{ if(t.m_contained) m_contained=new ExpressionType(*t.m_contained); }
		~ExpressionType() {/* delete contained; */}
		
		bool operator==(const ExpressionType& t) const;
		bool operator!=(const ExpressionType& t) const { return !operator==(t); }
		ExpressionType operator=(const ExpressionType& et);
		
		Type type() const { return m_type; }
		ExpressionType contained() const { return *m_contained; }
		int size() const { return m_size; }
		
	private:
		Type m_type;
		///In case of list and vector the inside type
		ExpressionType* m_contained;
		
		int m_size;
};

class ANALITZA_EXPORT ExpressionTypeChecker : public ExpressionWriter
{
	public:
		ExpressionTypeChecker(Variables* v);
		
		ExpressionType check(const Expression& exp);
		
		virtual QString accept(const Operator* var);
		virtual QString accept(const Ci* var);
		virtual QString accept(const Cn* var);
		virtual QString accept(const Container* var);
		virtual QString accept(const Vector* var);
		virtual QString accept(const List* l);
		
		virtual QString result() const { return QString(); }
		
		bool isCorrect() const { return m_err.isEmpty(); }
		QStringList errors() const { return m_err; }
	private:
		static const Container* lambdaFor(const Object* o);
		void typeIs(const Object* o, const ExpressionType& type);
		
		template <class T>
		void typeIs(T it,
			const T& itEnd, const ExpressionType& type );
		
		QStringList m_err;
		ExpressionType current;
		Variables* m_v;
		QList<ExpressionType> parameters;
		QHash<QString, ExpressionType> m_typeForBVar;
		QStack<const Object*> m_calls;
};

QDebug operator<<(QDebug dbg, const ExpressionType &c);

}

#endif // EXPRESSIONTYPECHECKER_H
