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

#ifndef OPERATOR_H
#define OPERATOR_H

#include <QStandardItemModel>
#include "object.h"

class Operator;
class OperatorsModel;

class Operator : public Object
{
	public:
		Operator(OperatorType t) : Object(oper), m_optype(t) {}
		Operator(Object const *o);
		virtual ~Operator() {}
		
		void setOperator(enum OperatorType t) { m_optype=t; }
		static int nparams(enum OperatorType);
		int nparams() const { return nparams(m_optype); }
		enum OperatorType operatorType() const { return m_optype; }
		static enum OperatorType toOperatorType(const QString &s);
		unsigned int weight() const { return operatorWeight(m_optype); }
		static unsigned int operatorWeight(OperatorType);
		QString toString() const;
		QString toMathML() const;
		
		bool operator==(const Operator& o) { return m_optype==o.m_optype; }
		Operator operator=(const Operator &a) { setType(a.type()); m_optype=a.operatorType(); return *this;}
		static const OperatorsModel m_words;
		static OperatorType multiplicityOperator(const OperatorType& t); //When we have n t's, we got one rn
		OperatorType multiplicityOperator() const { return multiplicityOperator(m_optype); }
		bool isBounded() const;
	private:
		enum OperatorType m_optype;
};

class OperatorsModel : public QStandardItemModel
{
public:
	OperatorsModel(QObject *parent=NULL);
	QString operToString(const Operator& op) const;
	int count() const { return m_count; }
private:
	int m_count;
};

#endif
