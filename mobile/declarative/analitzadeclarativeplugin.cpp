/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "analitzadeclarativeplugin.h"
#include "analitzawrapper.h"
#include <analitzaplot/plotsmodel.h>
#include <analitzagui/variablesmodel.h>
#include <analitza/variables.h>
#include <graph2dmobile.h>
#include <QtDeclarative/QDeclarativeItem>

void AnalitzaDeclarativePlugin::registerTypes(const char* uri)
{
    qmlRegisterType<AnalitzaWrapper>("org.kde.analitza", 1, 0, "Analitza");
    qmlRegisterType<ExpressionWrapper>("org.kde.analitza", 1, 0, "Expression");
	qmlRegisterType<Graph2DMobile>("org.kde.analitza", 1, 0, "Graph2DView");
    qmlRegisterType<Analitza::PlotsModel>("org.kde.analitza", 1, 0, "PlotsModel");
    qmlRegisterType<Analitza::VariablesModel>("org.kde.analitza", 1, 0, "VariablesModel");
    qmlRegisterInterface<Analitza::Variables*>("Analitza::Variables");
}
