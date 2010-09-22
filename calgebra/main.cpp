/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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

#include <analitza/expression.h>
#include <analitza/analyzer.h>
#include <analitza/explexer.h>
#include <analitza/expressionparser.h>

#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#include <QTime>

using namespace std;

using Analitza::Expression;

Analitza::Analyzer a;

enum CalcType { Evaluate, Calculate };

static const char* prompt=">>> ";
static const char* insidePrompt="... ";

struct Config {
	CalcType calcType;
	bool showElapsedType;
};
static Config configuration;

void calculate(const Expression& e, CalcType t)
{
	Expression ans;
	a.setExpression(e);
	if(e.isCorrect()) {
		QTime time;
		if(configuration.showElapsedType) time.start();
		
		if(t==Calculate)
			ans=a.calculate();
		else
			ans=a.evaluate();
		
		if(configuration.showElapsedType) qDebug() << "Ellapsed time: " << time.elapsed();
	}
	
	if(a.isCorrect()) {
		qDebug() << qPrintable(ans.toString());
		a.insertVariable("ans", ans);
	} else {
		QStringList errors = a.errors();
		qDebug() << "Error:";
		foreach(const QString &err, errors)
			qDebug() << " -" << qPrintable(err);
	}
}

int main(int argc, char *argv[])
{
	configuration.calcType=Evaluate;
	configuration.showElapsedType=false;
	
	for(int i=1; i<argc; ++i) {
		QByteArray arg=argv[i];
		if(arg=="--print-time")
			configuration.showElapsedType=true;
		else if(arg=="--calculate")
			configuration.calcType=Calculate;
		else if(arg=="--evaluate")
			configuration.calcType=Evaluate;
		else if(arg=="--help" || arg=="-h") {
			qDebug() << "This is KAlgebra console version";
			qDebug() << "Use: " << argv[0] << "[Options]";
			qDebug() << "\t--evaluate:\tTries to simplify symbolically before calculating";
			qDebug() << "\t--calculate:\tCalculates straight away. If some symbol is missing, it will fail";
			qDebug() << "\t--print-times:\tOutputs the ellapsed time of an operation";
			qDebug() << "\t--help:\t\twill print this help";
			return 0;
		} else {
			qDebug() << "Unknown argument: " << argv[i];
			return 1;
		}
	}
	
	bool done=false;
	bool inside=false;
	
	using_history();
	QString entry;
	while(!done) {
		char * expr;
		if(inside)
			expr=readline(insidePrompt);
		else
			expr=readline(prompt);
		
		if(!expr)
			done=true;
		else if(*expr==0) {
			inside=false;
			entry.clear();
		} else {
			add_history(expr);
			entry+=QString(expr);
			
			ExpLexer lex(entry);
			ExpressionParser ex;
			ex.parse(&lex);
			if(lex.isCompletelyRead()) {
				Expression e(ex.mathML(), true);
// 				qDebug() << entry << e.toString();
				calculate(e, configuration.calcType);
				inside =false;
				entry.clear();
			} else {
				inside =true;
			}
		}
	}
	
	for(int i=0; i<history_get_history_state()->length; i++) {
		HIST_ENTRY *he = remove_history(i);
// 		free(he->line);
		free_history_entry(he);
	}
	qDebug("\nExit.");
	return 0;
}
