#include "analitzawrapper.h"
#include <analitza/analyzer.h>
#include <analitza/value.h>
#include <analitza/variables.h>
#include <analitzagui/variablesmodel.h>

#include <QVariant>
#include <QScriptEngine>
#include <KLocalizedString>

AnalitzaWrapper::AnalitzaWrapper(QScriptEngine* engine, QObject* parent)
	: QObject(parent)
	, m_wrapped(new Analitza::Analyzer), m_calc(false), m_engine(engine), m_varsModel(0)
{}

QVariant expressionToVariant(const Analitza::Expression& res)
{
	QVariant ret;
	if(res.isVector() || res.isList()) {
		QVariantList vals;
		
		QList<Analitza::Expression> expressions = res.toExpressionList();
		foreach(const Analitza::Expression& exp, expressions) {
			vals << expressionToVariant(exp);
		}
		
		ret = vals;
	} else if(res.isReal()) {
		Analitza::Cn val = res.toReal();
		if(val.isBoolean())
			ret = val.isTrue();
		else if(val.isInteger())
			ret = int(val.value());
		else
			ret = val.value();
	} else
		ret = res.toString();
	
	return ret;
}

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
	return expressionToVariant(res);
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
	
	return expressionToVariant(expr);
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
