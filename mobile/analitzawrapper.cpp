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

#include "analitzawrapper.h"
#include <analitza/analyzer.h>
#include <analitza/value.h>
#include <analitza/variables.h>
#include <analitzagui/variablesmodel.h>

#include <QVariant>
#include <QScriptEngine>
#include <KLocalizedString>
#include <analitza/analitzautils.h>

AnalitzaWrapper::AnalitzaWrapper(QScriptEngine* engine, QObject* parent)
	: QObject(parent)
	, m_wrapped(new Analitza::Analyzer), m_calc(false), m_engine(engine), m_varsModel(0)
{}

QVariant AnalitzaWrapper::execute(const QString& expression)
{
	Analitza::Expression e(expression, false);
	if(!e.isCorrect()) {
		m_engine->currentContext()->throwError(i18n("Error: %1", e.error().join(", ")));
		return QVariant();
	}
	m_wrapped->setExpression(e);
	
	Analitza::Expression res;
	if(m_calc)
		res = m_wrapped->calculate();
	else
		res = m_wrapped->evaluate();
	
	if(!m_wrapped->isCorrect()) {
		m_engine->currentContext()->throwError(i18n("Error: %1", m_wrapped->errors().join(", ")));
		return QVariant();
	} else if(m_varsModel) {
		m_varsModel->updateInformation();
	}
	return AnalitzaUtils::expressionToVariant(res);
}

Analitza::Expression variantToExpression(const QVariant& v)
{
	if(v.canConvert(QVariant::Double))
		return Analitza::Expression(Analitza::Cn(v.toReal()));
	else if(v.canConvert(QVariant::List)) {
		QVariantList list = v.toList();
		QList<Analitza::Expression> expressionList;
		
		foreach(const QVariant& elem, list) {
			expressionList << variantToExpression(elem);
		}
		
		return Analitza::Expression::constructList(expressionList);
	} else if(v.canConvert(QVariant::String))
		return Analitza::Expression(v.toString());
	
	Q_ASSERT(false && "couldn't figure out the type");
	return Analitza::Expression();
}

QVariant AnalitzaWrapper::executeFunc(const QString& name, const QVariantList& args)
{
	if(!m_wrapped->variables()->contains(name)) {
		m_engine->currentContext()->throwError(i18n("Undefined Identifier: %1", name));
		return QVariant();
	}
	
	QStack<Analitza::Object*> stack;
	QList<Analitza::Expression> exps;
	foreach(const QVariant& v, args) {
		exps += variantToExpression(v);
		stack << exps.last().tree();
	}
	
	m_wrapped->setExpression(m_wrapped->variables()->valueExpression(name));
	m_wrapped->setStack(stack);
	Analitza::Expression expr = m_wrapped->calculateLambda();
	
	return AnalitzaUtils::expressionToVariant(expr);
}

QString AnalitzaWrapper::unusedVariableName() const
{
	QString candidate;
	char curr='a';
	
	Analitza::Variables* v = m_wrapped->variables();
	for(candidate=curr; v->contains(candidate); ) {
		curr+=1;
		if(curr>'z')
			curr='a';
		else
			candidate.chop(1);
		
		candidate += curr;
	}
	
	return candidate;
}

void AnalitzaWrapper::removeVariable(const QString& name)
{
	m_wrapped->variables()->remove(name);
}

VariablesModel* AnalitzaWrapper::variablesModel()
{
	if(!m_varsModel)
		m_varsModel = new VariablesModel(m_wrapped->variables());
	return m_varsModel;
}
