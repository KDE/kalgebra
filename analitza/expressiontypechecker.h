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

#include "expressionwriter.h"
#include "analitzaexport.h"
#include "expressiontype.h"
#include <QStack>
#include <QStringList>
#include <QSet>

namespace Analitza
{
struct TypePair;
struct TypeTriplet;
class Variables;
class Expression;

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
		virtual QString accept(const Apply* a);
		virtual QString accept(const CustomObject* c);
		
		virtual QString result() const { return QString(); }
		
		QStringList dependencies() const { return m_deps; }
		bool hasDependencies() const { return !m_deps.isEmpty(); }
		bool isCorrect() const { return m_err.isEmpty(); }
		QStringList errors() const;
		
		void initializeVars(const QMap<QString, ExpressionType>& types) { m_vars=types; }
		
	private:
		ExpressionType solve(const Operator* o, const QList<Object*>& parameters);
		bool inferType(const Object* exp, const ExpressionType& targetType, QMap<QString, ExpressionType>* assumptions);
		QList<TypePair> computePairs(const QList<TypePair>& options, const ExpressionType& param);
		
		QMap<int, ExpressionType> computeStars(const QMap<int, ExpressionType>& initial, const ExpressionType& candidate, const ExpressionType& type);
		
		bool matchAssumptions(QMap<int, ExpressionType>* stars, const QMap<QString, ExpressionType>& assum1, const QMap<QString, ExpressionType>& assum2);
		
		QMap<QString, ExpressionType> typeIs(const Object* o, const ExpressionType& type);
		template <class T>
			QMap<QString, ExpressionType> typeIs(T it, const T& itEnd, const ExpressionType& type);
		
		ExpressionType typeForVar(const QString& var);
		
		void addError(const QString& err);
		ExpressionType commonType(const QList<Object*>& values);
		bool isVariableDefined(const QString& id) const;
		
		uint m_stars;
		QList<QStringList> m_err;
		QStringList m_calculating;
		ExpressionType current;
		Variables* m_v;
		QMap<QString, ExpressionType> m_typeForBVar;
		QMap<QString, ExpressionType> m_vars;
		QSet<QString> m_lambdascope;
		QStack<const Object*> m_calls;
		QStringList m_deps;
};

QDebug operator<<(QDebug dbg, const ExpressionType &c);

}

#endif // EXPRESSIONTYPECHECKER_H
