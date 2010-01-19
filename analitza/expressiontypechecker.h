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
		
		ExpressionType(Type t=Error) : type(t), contained(0) {}
		ExpressionType(const ExpressionType& t) : type(t.type), contained(0)
			{ if(t.contained) contained=new ExpressionType(*t.contained); }
		~ExpressionType() {/* delete contained; */}
		
		Type type;
		///In case of list and vector the inside type
		ExpressionType* contained;
		
		QString error;
};

class ANALITZA_EXPORT ExpressionTypeChecker : public ExpressionWriter
{
	public:
		ExpressionTypeChecker(const Analitza::Expression& exp, Analitza::Variables* v);
		
		ExpressionType check();
		
		virtual QString accept(const Analitza::Operator* var);
		virtual QString accept(const Analitza::Ci* var);
		virtual QString accept(const Analitza::Cn* var);
		virtual QString accept(const Analitza::Container* var);
		virtual QString accept(const Analitza::Vector* var);
		virtual QString accept(const Analitza::List* l);
		
		virtual QString result() const { return QString(); }
	private:
		static Container* lambdaFor(Object*);
		
		ExpressionType current;
		Variables* m_v;
		Expression m_exp;
		QList<ExpressionType> parameters;
		QHash<QString, ExpressionType> m_typeForBVar;
};

}

#endif // EXPRESSIONTYPECHECKER_H
