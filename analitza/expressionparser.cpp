// This file was generated by qlalr - DO NOT EDIT!

#line 100 "exp.g"

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

#line 189 "exp.g"

case 0:
	m_exp = "<math>"+sym(1)+"</math>";
	break;

#line 196 "exp.g"
 case 1: 
#line 198 "exp.g"

case 2:
	sym(1) = lexer->current.val;
	break;

#line 206 "exp.g"

case 4:
	sym(1) = "<ci>"+sym(1)+"</ci>";
	break;

#line 214 "exp.g"

case 6:
	sym(1) = sym(2);
	break;

#line 224 "exp.g"
 case 9: sym(1)=sym(2); break; 
#line 225 "exp.g"
 case 10: sym(1)=funcToTag(sym(1)); break; 
#line 227 "exp.g"
 case 11: sym(1) = "<apply>"+sym(1)+sym(2)+"</apply>"; break; 
#line 228 "exp.g"
 case 12: sym(1) = "<apply>"+sym(1)+sym(3)+"</apply>"; break; 
#line 229 "exp.g"
 case 13: sym(1) = "<apply>"+sym(1)+       "</apply>"; break; 
#line 235 "exp.g"

case 15:
	sym(1).prepend(sym(3));
	break;

#line 242 "exp.g"

case 16:
	sym(1)=sym(3)+sym(5)+sym(1);
	break;

#line 250 "exp.g"

case 17:
	sym(1) = '<'+sym(1)+'>'+sym(3)+"</"+sym(1)+'>';
	break;

#line 258 "exp.g"

case 18:
	sym(1) = "<lambda>"+sym(1)+sym(3)+"</lambda>";
	break;

#line 266 "exp.g"

case 19:
	sym(1) = "<apply><minus />"+sym(2)+"</apply>";
	break;

#line 273 "exp.g"

case 20:
	sym(1) = "<otherwise>"+sym(2)+"</otherwise>";
	break;

#line 280 "exp.g"
 case 21: sym(1) = "<apply><plus />"  +sym(1)+sym(3)+"</apply>"; break; 
#line 281 "exp.g"
 case 22: sym(1) = "<apply><minus />" +sym(1)+sym(3)+"</apply>"; break; 
#line 282 "exp.g"
 case 23: sym(1) = "<apply><times />" +sym(1)+sym(3)+"</apply>"; break; 
#line 283 "exp.g"
 case 24: sym(1) = "<apply><times />" +sym(1)+sym(2)+"</apply>"; break; 
#line 284 "exp.g"
 case 25: sym(1) = "<apply><divide />"+sym(1)+sym(3)+"</apply>"; break; 
#line 285 "exp.g"
 case 26: sym(1) = "<apply><power />" +sym(1)+sym(3)+"</apply>"; break; 
#line 286 "exp.g"
 case 27: sym(1) = "<apply><eq />"    +sym(1)+sym(3)+"</apply>"; break; 
#line 288 "exp.g"
 case 28: sym(1) = "<piece>"+sym(3)+sym(1)+"</piece>"; break; 
#line 289 "exp.g"
 case 29: sym(1) = "<declare><ci>"+sym(1)+"</ci>"+sym(3)+"</declare>"; break; 
#line 294 "exp.g"

case 31:
	sym(1) += sym(3);
	break;

#line 303 "exp.g"

case 33:
	sym(1) = sym(2);
	break;

#line 310 "exp.g"

case 34:
	sym(1) = "<bvar><ci>"+sym(1)+"</ci></bvar>";
	break;

#line 317 "exp.g"

case 35:
	sym(1) += sym(3);
	break;

#line 324 "exp.g"

case 36:
	sym(1) += sym(3);
	break;

#line 331 "exp.g"

case 37:
	sym(1) = "<uplimit>"+sym(3)+"</uplimit><downlimit>"+sym(1)+"</downlimit>";
	break;

#line 337 "exp.g"

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

