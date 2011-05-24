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
%token tId		"identifier"
%token tLambda	"->"
%token tQm		"?"
%token tComa	","
%token tLpr		"("
%token tRpr		")"
%token tLcb		"{"
%token tRcb		"}"
%token tLsp		"["
%token tRsp		"]"
%token tPipe	"|"
%token tVal		"value"
%token tEq		"="
%token tLt		"<"
%token tGt		">"
%token tLeq		">="
%token tGeq		"<="
%token tNeq		"!="
%token tColon	":"
%token tAt		"@"
%token tComment	"//Comment//"
%token tString	"abc"


%left tComa
%left tLambda
%left otherwise_prec
%left tQm
%left tEq tNeq tLt tLeq tGt tGeq
%left tSub tAdd
%left tMul tDiv
%left uminus_prec
%left tPow
%left tPipe

%start Program

/:
/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
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
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
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
#include <KLocale>

ExpressionParser::ExpressionParser()
{}

ExpressionParser::~ExpressionParser()
{}

QString funcToTag(const QString& name)
{
	if(Analitza::Operator::toOperatorType(name)!=Analitza::Operator::none)
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
		while(yytoken==tComment) {
			//TODO: Add mathml comment
			yytoken = lexer->lex();
		}
		
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

Program ::= ;/. case $rule_number: ./
Program ::= Declaration ;/. case $rule_number: ./
Program ::= Expression ;
/.
case $rule_number:
	m_exp = "<math>"+sym(1)+"</math>";
	break;
./

Declaration ::= Id tAssig Expression ;       /. case $rule_number: sym(1) = "<declare><ci>"+sym(1)+"</ci>"+sym(3)+"</declare>"; break; ./

-- primary
Id ::=  tId; /. case $rule_number: ./
String ::=  tString; /. case $rule_number: ./
Number ::= tVal;
/.
case $rule_number:
	sym(1) = lexer->current.val;
	break;
./

Value ::= Number;
Value ::= String;
Value ::= Id;
/.
case $rule_number:
	sym(1) = "<ci>"+sym(1)+"</ci>";
	break;
./

PrimaryExpression ::= Value | BlockExpression;

SubscriptExpression ::= PrimaryExpressionExt tLsp Expression tRsp; --vect[2+2]
/.
case $rule_number:
	sym(1) = "<apply><selector />"+sym(3)+sym(1)+"</apply>";
	break;
./

PrimaryExpressionExt ::= tLpr Expression tRpr;
/.
case $rule_number:
	sym(1) = sym(2);
	break;
./

PrimaryExpressionExt ::= PrimaryExpression | SubscriptExpression | FunctionCall;
Expression ::= PrimaryExpressionExt;

-- function
FunctionId ::= tLpr Expression tRpr; /. case $rule_number: sym(1)=sym(2); break; ./
FunctionId ::= Id; /. case $rule_number: sym(1)=funcToTag(sym(1)); break; ./

Expression ::= FunctionId PrimaryExpression ; /. case $rule_number: sym(1) = "<apply>"+sym(1)+sym(2)+"</apply>"; break; ./
FunctionCall ::= FunctionId tLpr  FBody  tRpr ; /. case $rule_number: sym(1) = "<apply>"+sym(1)+sym(3)+"</apply>"; break; ./
FunctionCall ::= FunctionId tLpr         tRpr ; /. case $rule_number: sym(1) = "<apply>"+sym(1)+       "</apply>"; break; ./

PipedCall ::= Expression tPipe FunctionId; /. case $rule_number: sym(1) = "<apply>"+sym(3)+sym(1)+"</apply>"; break; ./
Expression ::= PipedCall;

-- function's body
FBody ::= Parameters ;

FBody ::= Parameters tColon BVars;
/.
case $rule_number:
	sym(1).prepend(sym(3));
	break;
./

FBody ::= Parameters tColon BVars tEq Limits;
/.
case $rule_number:
	sym(1)=sym(3)+sym(5)+sym(1);
	break;
./

FBody ::= Parameters tColon BVars tAt Expression;
/.
case $rule_number:
	sym(1)=sym(3)+"<domainofapplication>"+sym(5)+"</domainofapplication>"+sym(1);
	break;
./

-- block
BlockExpression ::= Id tLcb            tRcb ;
/.
case $rule_number:
	sym(1) = '<'+sym(1)+" />";
	break;
./

BlockExpression ::= Id tLcb Parameters tRcb ;
/.
case $rule_number:
	sym(1) = '<'+sym(1)+'>'+sym(3)+"</"+sym(1)+'>';
	break;
./

-- lambda
Expression ::= BVars tLambda Expression ;
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
Expression ::= Expression tAdd Expression ; /. case $rule_number: sym(1) = "<apply><plus />"  +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tSub Expression ; /. case $rule_number: sym(1) = "<apply><minus />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tMul Expression ; /. case $rule_number: sym(1) = "<apply><times />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Number PrimaryExpressionExt; /. case $rule_number: sym(1) = "<apply><times />" +sym(1)+sym(2)+"</apply>"; break; ./
Expression ::= Expression tDiv Expression ; /. case $rule_number: sym(1) = "<apply><divide />"+sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tPow Expression ; /. case $rule_number: sym(1) = "<apply><power />" +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tEq  Expression ; /. case $rule_number: sym(1) = "<apply><eq />"    +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tLeq Expression ; /. case $rule_number: sym(1) = "<apply><leq />"   +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tGeq Expression ; /. case $rule_number: sym(1) = "<apply><geq />"   +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tLt  Expression ; /. case $rule_number: sym(1) = "<apply><lt />"    +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tGt  Expression ; /. case $rule_number: sym(1) = "<apply><gt />"    +sym(1)+sym(3)+"</apply>"; break; ./
Expression ::= Expression tNeq Expression ; /. case $rule_number: sym(1) = "<apply><neq />"   +sym(1)+sym(3)+"</apply>"; break; ./

Expression ::= Expression tQm  Expression ; /. case $rule_number: sym(1) = "<piece>"+sym(3)+sym(1)+"</piece>"; break; ./

-- parameters
Parameters ::= Expression ;
Parameters ::= Parameters tComa Expression ;
/.
case $rule_number:
	sym(1) += sym(3);
	break;
./

-- bvars
BVars ::= BValue;
BVars ::= tLpr BVarList tRpr ;
/.
case $rule_number:
	sym(1) = sym(2);
	break;
./

BValue ::= Id;
/.
case $rule_number:
	sym(1) = "<bvar><ci>"+sym(1)+"</ci></bvar>";
	break;
./

BVarList ::= BValue tComa BValue;
/.
case $rule_number:
	sym(1) += sym(3);
	break;
./

BVarList ::= BVarList tComa BValue;
/.
case $rule_number:
	sym(1) += sym(3);
	break;
./

Limits ::= PrimaryExpressionExt tLimits PrimaryExpressionExt;
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
		int tokFoundType=lexer->current.type;
		QString error;
		
		if (shifts && shifts<3) {
			QString tokFound(spell[tokFoundType]);
			QStringList expectedTokens;
			for (int s = 0; s < shifts; ++s) {
				expectedTokens += '\''+QLatin1String(spell[expected_tokens[s]])+'\'';
			}
			error=i18nc("error message", "Expected %1 instead of '%2'", expectedTokens.join(i18n(", ")), tokFound);
		} else if(tokFoundType==tLpr) {
			error=i18n("Missing right parenthesis");
		} else if(tokFoundType==tRpr || tokFoundType==tRcb) {
			error=i18n("Unbalanced right parenthesis");
		} else
			error=i18n("Unexpected token %1", spell[tokFoundType]);
		m_err.append(error);
		return false;
		}
	}

	return false;
}

./
