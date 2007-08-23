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

#include "analitza.h"
#include "expression.h"
#include "exp.h"

#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

Analitza a;

void evaluate(char* expr)
{
	Expression e(expr, false), ans;
	qDebug() << e.toString();
			
	a.setExpression(e);
	if(e.isCorrect())
		ans=a.evaluate();
			
	if(a.isCorrect()) {
		qDebug() << qPrintable(ans.toString());
		a.insertVariable("ans", ans);
				
		add_history(expr);
	} else {
		free(expr);
		QStringList errors = a.errors();
		qDebug() << "Error:";
		foreach(QString err, errors)
			qDebug() << " -" << qPrintable(err);
	}
}

void calculate(char* expr)
{
	Expression e(expr, false);
	Cn ans;
	qDebug() << e.toString();
	a.setExpression(e);
	
	if(e.isCorrect())
		ans=a.calculate();
			
	if(a.isCorrect()) {
		qDebug() << qPrintable(ans.toString());
		a.insertVariable("ans", ans);
				
		add_history(expr);
	} else {
		free(expr);
		QStringList errors = a.errors();
		qDebug() << "Error:";
		foreach(QString err, errors)
			qDebug() << " -" << qPrintable(err);
	}
}

int main(int argc, char *argv[])
{
	bool done=false;
	
	using_history();
	while(!done) {
		char * expr=readline(">>> ");
		if(!expr)
			done=true;
		else {
#if 1
			Exp ex(expr);
			ex.parse();
			qDebug() << ex.mathML() << ex.error();
#endif
			
			calculate(expr);
		}
	}
	
	for(int i=0; i<history_get_history_state()->length; i++) {
		HIST_ENTRY *he = remove_history(i);
// 		free(he->line);
		free_history_entry(he);
	}
	return 0;
}
