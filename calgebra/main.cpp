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

#include <readline/readline.h>
#include <readline/history.h>
#include <QElapsedTimer>
#include <QFile>

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
        QElapsedTimer time;
        if(configuration.showElapsedType) time.start();
        
        if(t==Calculate)
            ans=a.calculate();
        else
            ans=a.evaluate();
        
        if(configuration.showElapsedType) qDebug() << "Ellapsed time: " << time.elapsed();
    }
    
    if(a.isCorrect()) {
        qDebug() << qPrintable(ans.toString());
        a.insertVariable(QStringLiteral("ans"), ans);
    } else {
        const QStringList errors = a.errors();
        qDebug() << "Error:";
        for (const QString &err : errors)
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
            qDebug() << "Use: " << argv[0] << "[Options] ...";
            qDebug() << "\t--evaluate:\tTries to simplify symbolically before calculating";
            qDebug() << "\t--calculate:\tCalculates straight away. If some symbol is missing, it will fail";
            qDebug() << "\t--print-time:\tOutputs the ellapsed time of an operation";
            qDebug() << "\t--help:\t\twill print this help";
            qDebug() << "\t...\t\tfiles that will be executed first";
            return 0;
        } else {
            QFile f(QString::fromUtf8(arg));
            if(!f.open(QIODevice::ReadOnly)) {
                qWarning() << "File not found: " << arg;
                return 1;
            }
            
            QTextStream str(&f);
            a.importScript(&str);
            
            if(!a.isCorrect()) {
                const QStringList errors = a.errors();
                qDebug() << "Error:";
                for(const QString &err : errors)
                    qDebug() << " -" << qPrintable(err);
            }
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
            entry+=QString::fromUtf8(expr);
            
            if(Expression::isCompleteExpression(entry)) {
                Expression e(entry);
//                 qDebug() << entry << e.toString();
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
#ifdef HAVE_FREE_HISTORY_ENTRY
        free_history_entry(he);
#else
        free((void*)he->line);
        free(he);
#endif
    }
    qDebug("\nExit.");
    return 0;
}
