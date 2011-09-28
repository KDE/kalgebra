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

class VariablesModel;
class QScriptEngine;
namespace Analitza {
	class Analyzer;
}

class AnalitzaWrapper : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool calculate READ isCalculate WRITE setCalculate)
	Q_PROPERTY(bool isCorrect READ isCorrect)
	Q_PROPERTY(QString error READ error)
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
		
		QString error() const { return m_error; }
		bool isCorrect() const { return m_error.isEmpty(); }
	private:
		void throwError(const QString& error);
		Analitza::Analyzer* m_wrapped;
		bool m_calc;
		VariablesModel* m_varsModel;
		QString m_error;
};

#endif // ANALITZAWRAPPER_H
