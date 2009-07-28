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

#include "analitza.h"
#include "expression.h"
#include "explexer.h"
#include "expressionparser.h"

#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

Analitza a;

void evaluate(const Expression& e)
{
	Expression ans;
	a.setExpression(e);
	if(e.isCorrect())
		ans=a.evaluate();
			
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

void calculate(const Expression& e)
{
	Expression ans;
	a.setExpression(e);
	if(e.isCorrect())
		ans=a.calculate();
			
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

const char* prompt=">>> ";
const char* insidePrompt="... ";

int main(int argc, char *argv[])
{
	Q_UNUSED(argc);
	Q_UNUSED(argv);
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
				evaluate(e);
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
