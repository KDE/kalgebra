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
#include <QSharedPointer>
#include "expression.h"

namespace Analitza
{
class Variables;

class ANALITZA_EXPORT ExpressionType
{
	public:
		enum Type { Error=0, Value, Vector, List };
		QString toString() const;
		
		ExpressionType(Type t=Error) : type(t), contained(0), size(-1) {}
		ExpressionType(Type t, const ExpressionType& contained, int s=-1) : type(t), contained(new ExpressionType(contained)), size(s) {}
		
		ExpressionType(const ExpressionType& t) : type(t.type), contained(0), size(t.size)
			{ if(t.contained) contained=new ExpressionType(*t.contained); }
		~ExpressionType() {/* delete contained; */}
		
		bool operator==(const ExpressionType& t) const;
		bool operator!=(const ExpressionType& t) const { return !operator==(t); }
		
		Type type;
		///In case of list and vector the inside type
		ExpressionType* contained;
		
		int size;
};

class ANALITZA_EXPORT ExpressionTypeChecker : public ExpressionWriter
{
	public:
		ExpressionTypeChecker(Analitza::Variables* v);
		
		ExpressionType check(const Analitza::Expression& exp);
		
		virtual QString accept(const Analitza::Operator* var);
		virtual QString accept(const Analitza::Ci* var);
		virtual QString accept(const Analitza::Cn* var);
		virtual QString accept(const Analitza::Container* var);
		virtual QString accept(const Analitza::Vector* var);
		virtual QString accept(const Analitza::List* l);
		
		virtual QString result() const { return QString(); }
		
		bool isCorrect() const { return m_err.isEmpty(); }
		QStringList errors() const { return m_err; }
	private:
		static const Container* lambdaFor(const Object* o);
		void typeIs(const Object* o, const ExpressionType& type);
		void typeIs(QVector<Object*>::const_iterator it,
			const QVector<Object*>::const_iterator& itEnd, const ExpressionType& type );
		void typeIs(QList<Object*>::const_iterator it,
			const QList<Object*>::const_iterator& itEnd, const ExpressionType& type );
		
		QStringList m_err;
		ExpressionType current;
		Variables* m_v;
		QList<ExpressionType> parameters;
		QHash<QString, ExpressionType> m_typeForBVar;
};

QDebug operator<<(QDebug dbg, const ExpressionType &c);

}

#endif // EXPRESSIONTYPECHECKER_H
