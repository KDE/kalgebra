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

#include "kalgebramobile.h"

#include <analitza/variables.h>
#include <analitzagui/variablesmodel.h>
#include <analitzaplot/plotsmodel.h>
#include <analitzaplot/planecurve.h>
#include <analitzaplot/plotsfactory.h>
#include "analitzawrapper.h"

#include <QDeclarativeContext>
#include "graph2dmobile.h"
#include "pluginsmodel.h"

KAlgebraMobile* KAlgebraMobile::s_self=0;
KAlgebraMobile* KAlgebraMobile::self() { return s_self; }

KAlgebraMobile::KAlgebraMobile(QObject* parent)
	: QObject(parent), m_functionsModel(0), m_vars(new Analitza::Variables)
{
	Q_ASSERT(s_self==0);
	s_self=this;
	
	qmlRegisterType<PluginsModel>("org.kde.analitza", 1, 0, "PluginsModel");
	qmlRegisterType<AnalitzaWrapper>("org.kde.analitza", 1, 0, "Analitza");
	qmlRegisterType<ExpressionWrapper>("org.kde.analitza", 1, 0, "Expression");
	qmlRegisterType<PlotsModel>("org.kde.analitza", 1, 0, "PlotsModel");
	qmlRegisterType<Graph2DMobile>("org.kde.analitza", 1, 0, "Graph2DView");
	qmlRegisterType<VariablesModel>("org.kde.analitza", 1, 0, "VariablesModel");
	qmlRegisterInterface<Analitza::Variables*>("Analitza::Variables");
// 	global.setProperty("VariablesModel", varsmodel, QScriptValue::Undeletable|QScriptValue::ReadOnly);
}

PlotsModel* KAlgebraMobile::functionsModel()
{
	if(!m_functionsModel) {
		m_functionsModel = new PlotsModel(this);
		connect(m_functionsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(functionRemoved(QModelIndex,int,int)));
		connect(m_functionsModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(functionInserted(QModelIndex,int,int)));
		connect(m_functionsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(functionModified(QModelIndex, QModelIndex)));
	}
	
	return m_functionsModel;
}

void KAlgebraMobile::functionInserted(const QModelIndex& parent, int start, int end)
{
	Q_ASSERT(!parent.isValid());
	for(int i=start; i<=end; i++) {
		QModelIndex idx = functionsModel()->index(i, 1);
		m_vars->modify(idx.sibling(i,0).data().toString(), Analitza::Expression(idx.data().toString()));
	}
}

void KAlgebraMobile::functionRemoved(const QModelIndex& parent, int start, int end)
{
	Q_ASSERT(!parent.isValid());
	for(int i=start; i<=end; i++) {
		QModelIndex idx = functionsModel()->index(i);
		m_vars->remove(functionsModel()->data(idx, Qt::DisplayRole).toString());
	}
}

void KAlgebraMobile::functionModified(const QModelIndex& idxA, const QModelIndex& idxB)
{
	if(idxB.column()==1) {
		for(int i=idxA.row(); i<idxB.row(); ++i) {
			QModelIndex idx = functionsModel()->index(i, 1);
			m_vars->modify(idx.sibling(i,0).data().toString(), Analitza::Expression(idx.data().toString()));
		}
	} //else TODO: figure out how to control a "rename"
}

Analitza::Variables* KAlgebraMobile::variables() const { return m_vars; }

QColor randomFunctionColor() { return QColor::fromHsv(qrand()%255, 255, 225); }

QStringList KAlgebraMobile::addFunction(const QString& expression, double up, double down)
{
	PlotsModel* model=functionsModel();
	
	Analitza::Variables* vars = variables();
	Analitza::Expression e(expression, Analitza::Expression::isMathML(expression));
	QString fname;
	do {
		fname = model->freeId();
	} while(vars->contains(fname));
	QColor fcolor = randomFunctionColor();
	
	QStringList err;
	PlotBuilder req = PlotsFactory::self()->requestPlot(e, Dim2D);
	PlaneCurve* it = static_cast<PlaneCurve*>(req.create(fcolor, fname, m_vars));
	if(up!=down)
		it->setInterval(it->parameters().first(), down, up);
	
	if(it->isCorrect())
		model->addPlot(it);
	else {
		err = it->errors();
		delete it;
	}
	
	return err;
}

