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
	public:
		explicit AnalitzaWrapper(QScriptEngine* engine, QObject* parent = 0);
		
		void setCalculate(bool calc) { m_calc = calc; }
		bool isCalculate() const { return m_calc; } 
		
		Analitza::Analyzer* wrapped() const { return m_wrapped; }
		
		Q_SCRIPTABLE QVariant execute(const QString& expression);
		Q_SCRIPTABLE QVariant executeFunc(const QString& name, const QVariantList& args);
		Q_SCRIPTABLE QString unusedVariableName() const;
		Q_SCRIPTABLE void removeVariable(const QString& name);
		VariablesModel* variablesModel();
		
	private:
		Analitza::Analyzer* m_wrapped;
		bool m_calc;
		QScriptEngine* m_engine;
		VariablesModel* m_varsModel;
};

#endif // ANALITZAWRAPPER_H
