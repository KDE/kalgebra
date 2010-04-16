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

#ifndef EXPRESSIONTYPE_H
#define EXPRESSIONTYPE_H

#include "analitzaexport.h"
#include <QString>
#include <QList>
#include <QMap>

namespace Analitza
{

class ANALITZA_EXPORT ExpressionType
{
	public:
		///Just use undefined type when returning from a recursion
		enum Type { Error=0, Value, Vector, List, Lambda, Any, Many, Undefined };
		QString toString() const;
		
		ExpressionType(Type t=Error, int any=-1);
		ExpressionType(Type t, const ExpressionType& contained, int s=0);
		ExpressionType(const ExpressionType& t);
		
		~ExpressionType() {/* delete contained; */}
		
		bool operator==(const ExpressionType& t) const;
		bool operator!=(const ExpressionType& t) const { return !operator==(t); }
		ExpressionType operator=(const ExpressionType& et);
		
		/** Depth search to check if it's defined */
		bool isError() const;
		
		Type type() const { return m_type; }
		ExpressionType contained() const { return m_contained.first(); }
		QList<ExpressionType> alternatives() const { Q_ASSERT(m_type==Many); return m_contained; }
		void addAlternative(const ExpressionType& t) { Q_ASSERT(m_type==Many); m_contained.append(t); }
		int size() const { return m_size; }
		int anyValue() const { return m_any; }
		
		void addParameter(const ExpressionType& t) { Q_ASSERT(m_type==Lambda); m_contained.append(t); }
		QList<ExpressionType> parameters() const { Q_ASSERT(m_type==Lambda); return m_contained; }
		ExpressionType returnValue() const;
		
		void addAssumption(const QString& bvar, const ExpressionType& t);
		QMap<QString, ExpressionType> assumptions() const;
		ExpressionType assumptionFor(const QString& bvar) const { return m_assumptions.value(bvar); }
		void addAssumptions(const QMap< QString, ExpressionType >& a);
		void clearAssumptions() { m_assumptions.clear(); }
		
		/** Returns a new type with the stars solved according t @p info */
		ExpressionType starsToType(const QMap<int, ExpressionType>& info) const;
		
		bool isUndefined() const { return m_type==Undefined; }
		bool canReduceTo(const ExpressionType& type) const;
		int increaseStars(int stars, QMap<int, int>* values=0);
		
		ExpressionType& simplifyStars();
	private:
		static void starsSimplification(ExpressionType& t, QMap<int, int>& reductions, int& next);
		void printAssumptions(const ExpressionType& t, int ind=0) const;
		
		Type m_type;
		///In case of list and vector the inside type
		QList<ExpressionType> m_contained;
		QMap<QString, ExpressionType> m_assumptions;
		union { int m_size; int m_any; };
};

}
#endif // EXPRESSIONTYPE_H
