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

#ifndef ANALITZAWRAPPER_H
#define ANALITZAWRAPPER_H

#include <QtCore/QObject>
#include <QVariant>
#include <analitza/expression.h>

class VariablesModel;
namespace Analitza { class Analyzer; }

class ExpressionWrapper : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString expression READ toString WRITE setText);
	public:
		ExpressionWrapper(QObject* parent=0);
		explicit ExpressionWrapper(const Analitza::Expression & e, QObject* parent = 0);
		
		void setText(const QString& exp);
		
	public slots:
		QString toString() const;
		bool isCorrect() const;
		QVariant value() const;
		QStringList errors() const;
		
	private:
		Analitza::Expression m_exp;
};

class AnalitzaWrapper : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool calculate READ isCalculate WRITE setCalculate)
	Q_PROPERTY(bool isCorrect READ isCorrect)
	Q_PROPERTY(QStringList errors READ errors)
	public:
		explicit AnalitzaWrapper(QObject* parent = 0);
		
		void setCalculate(bool calc) { m_calc = calc; }
		bool isCalculate() const { return m_calc; } 
		
		Analitza::Analyzer* wrapped() const { return m_wrapped; }
		
		Q_SCRIPTABLE QVariant execute(const QString& expression);
		Q_SCRIPTABLE QVariant executeFunc(const QString& name, const QVariantList& args);
		Q_SCRIPTABLE QString unusedVariableName() const;
		Q_SCRIPTABLE void removeVariable(const QString& name);
		VariablesModel* variablesModel();
		
		QStringList errors() const;
		bool isCorrect() const;
	private:
		Analitza::Analyzer* m_wrapped;
		bool m_calc;
		VariablesModel* m_varsModel;
};

#endif // ANALITZAWRAPPER_H
