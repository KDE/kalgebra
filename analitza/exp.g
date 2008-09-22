%parser         ExpressionTable
%decl           expressionparser.h
%impl           expressionparser.cpp

%expect 0

--%token tAssoc		"associative operator"
%token tAdd		"+"
%token tSub		"-"
%token tMul		"*"
%token tAssig	":="
%token tLimits	".."
%token tDiv		"/"
%token tPow		"^"
%token tFunc	"function"
%token tBlock	"block"
%token tLambda	"->"
%token tQm		"?"
%token tComa	","
%token tLpr		"("
%token tRpr		")"
%token tLcb		"{"
%token tRcb		"}"
%token tVal		"value"
%token tEq		"="
%token tColon	"|"


%left tComa
%right tAssig
%left otherwise_prec
%left tQm
%left tSub tAdd
%left tMul tDiv
%left tPow
%left uminus_prec

%start Program

/:
/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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

#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <QtCore/QStringList>
#include <QtCore/QVector>
#include "expressiontable_p.h"
#include "analitzaexport.h"
class AbstractLexer;

class ANALITZA_EXPORT ExpressionParser : protected $table
{
	public:
		ExpressionParser();
		~ExpressionParser();

		bool parse(AbstractLexer *lexer);

		bool isCompletelyRead() const { return true; }
		bool isCorrect() const { return m_err.isEmpty(); }
		int errorLineNumber() const { return m_errorLineNumber; }
		QStringList error() const { return m_err; }
		QString mathML() const { return m_exp; }

	private:
		void reallocateStack();

		inline QString &sym(int index)
		{ return m_symStack[m_tos + index - 1]; }

		int m_tos;
		QVector<int> m_stateStack;
		QVector<QString> m_symStack;
		int m_errorLineNumber;
		QStringList m_err;
		QString m_exp;
};

#endif

:/

/.
/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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



#include <QtCore/QDebug>
#include "expressionparser.h"
#include "abstractlexer.h"
#include "operator.h"

ExpressionParser::ExpressionParser()
{}

ExpressionParser::~ExpressionParser()
{}

QString funcToTag(const QString& name)
{
	if(Operator::toOperatorType(name)!=Operator::none)
		return QString("<%1 />").arg(name);
	else
		return "<ci type='function'>"+name+"</ci>";
}

void ExpressionParser::reallocateStack()
{
	int size = m_stateStack.size();
	if (size == 0)
		size = 128;
	else
		size <<= 1;

	m_symStack.resize(size);
	m_stateStack.resize(size);
}

bool ExpressionParser::parse(AbstractLexer *lexer)
{
  const int INITIAL_STATE = 0;
  int yytoken = -1;

  reallocateStack();

  m_tos = 0;
  m_stateStack[++m_tos] = INITIAL_STATE;

  for(;;) {
      const int state = m_stateStack.at(m_tos);
      if (yytoken == -1 && - TERMINAL_COUNT != action_index [state]) {
        yytoken = lexer->lex();
		if(!lexer->error().isEmpty()) {
			m_err += lexer->error();
			return false;
		}
	  }
      int act = t_action (state, yytoken);
      if (act == ACCEPT_STATE)
        return true;
      else if (act > 0) {
          if (++m_tos == m_stateStack.size())
            reallocateStack();
          m_stateStack[m_tos] = act;
          yytoken = -1;
      } else if (act < 0) {
          int r = - act - 1;

          m_tos -= rhs [r];
          act = m_stateStack.at(m_tos++);
          switch (r) {
./

Program ::= Expression ;
/.
case $rule_number:
	m_exp = "<math>"+sym(1)+"</math>";
	break;
./

-- primary
Value ::= tVal ;   /. case $rule_number: ./
Func ::= tFunc ;   /. case $rule_number: ./
Block ::= tBlock ;
/.
case $rule_number:
	sym(1) = lexer->current.val;
	break;
./

Expression ::= Value ;

Expression ::= tLpr Expression tRpr ;
/.
case $rule_number:
	sym(1) = sym(2);
	break;
./

-- function
Expression ::= Func Value ;
/.
case $rule_number:
	sym(1) = "<apply>"+funcToTag(sym(1))+sym(2)+"</apply>";
	break;
./

Expression ::= Func tLpr FBody tRpr ;
/.
case $rule_number:
	sym(1) = "<apply>"+funcToTag(sym(1))+sym(3)+"</apply>";
	break;
./

Expression ::= Func tLpr       tRpr ;
/.
case $rule_number:
	sym(1) = "<apply>"+funcToTag(sym(1))+"</apply>";
	break;
./

-- function's body
FBody ::= Parameters ;

FBody ::= Parameters tColon BVars ;
/.
case $rule_number:
	sym(1) = sym(3)+sym(1);
	break;
./

-- block
Expression ::= Block tLcb Parameters tRcb ;
/.
case $rule_number: {
	QString blockName=sym(1);
	sym(1) = '<'+blockName+'>'+sym(3)+"</"+blockName+'>';
	} break;
./

-- lambda
Expression ::= BVars tLambda Expression;
/.
case $rule_number:
	sym(1) = "<lambda>"+sym(1)+sym(3)+"</lambda>";
	break;
./

-- unary constructions
Expression ::= tSub Expression %prec uminus_prec ;
/.
case $rule_number:
	sym(1) = "<apply><minus />"+sym(2)+"</apply>";
	break;
./

Expression ::= tQm  Expression %prec otherwise_prec ;
/.
case $rule_number:
	sym(1) = "<otherwise>"+sym(2)+"</otherwise>";
	break;
./

-- binary constructions
Expression ::= Expression tAdd Expression ; /. case $rule_number: sym(1) = "<apply><plus />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tSub Expression ; /. case $rule_number: sym(1) = "<apply><minus />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tMul Expression ; /. case $rule_number: sym(1) = "<apply><times />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tDiv Expression ; /. case $rule_number: sym(1) = "<apply><divide />"+sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tPow Expression ; /. case $rule_number: sym(1) = "<apply><power />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tQm  Expression ; /. case $rule_number: sym(1) = "<piece>"+sym(3)+sym(1)+"</piece>"; break; ./
Expression ::= Value tAssig Expression ;    /. case $rule_number: sym(1) = "<declare>"+sym(1)+sym(3)+"</declare>"; break; ./

Parameters ::= Expression ;
Parameters ::= Parameters tComa Expression ;
/.
case $rule_number:
	sym(1) += sym(3);
	break;
./

BValue ::= Value;
/.
case $rule_number:
	sym(1) = "<bvar>"+sym(1)+"</bvar>";
	break;
./

BValue ::= Value tEq Limits;
/.
case $rule_number:
	sym(1) = "<bvar>"+sym(1)+"</bvar>"+sym(3);
	break;
./

BVars ::= BValue ;

BVars ::= tLpr BVarList tRpr ;
/.
case $rule_number:
	sym(1) = sym(2);
	break;
./

BVarList ::= BValue tComa BValue;
/.
case $rule_number:
	sym(1) = sym(1)+sym(3);
	break;
./

BVarList ::= BVarList tComa BValue;
/.
case $rule_number:
	sym(1) += sym(3);
	break;
./

Limits ::= Value tLimits Value;
/.
case $rule_number:
	sym(1) = "<uplimit>"+sym(3)+"</uplimit><downlimit>"+sym(1)+"</downlimit>";
	break;
./

/.
		} // switch
		m_stateStack[m_tos] = nt_action(act, lhs[r] - TERMINAL_COUNT);
	} else {
		int ers = state;
		int shifts = 0;
		int reduces = 0;
		int expected_tokens[3];
		for (int tk = 0; tk < TERMINAL_COUNT; ++tk) {
			int k = t_action(ers, tk);

			if (! k)
				continue;
			else if (k < 0)
				++reduces;
			else if (spell[tk]) {
				if (shifts < 3)
				expected_tokens[shifts] = tk;
				++shifts;
			}
		}

		m_errorLineNumber = lexer->lineNumber();
		m_err.clear();
		m_err.append(QString());
		if (shifts && shifts < 3) {
			bool first = true;

			for (int s = 0; s < shifts; ++s) {
				QString error;
				if (first)
					error += QLatin1String("Expected ");
				else
					error += QLatin1String(", ");

				first = false;
				error += '\''+QLatin1String(spell[expected_tokens[s]])+'\'';
				m_err.last() += error;
			}
			m_err.last() += " instead of " + QLatin1String(spell[lexer->current.type]);
		}

		return false;
		}
	}

	return false;
}

./
