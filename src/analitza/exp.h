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

#ifndef EXP_H
#define EXP_H

#include <QStack>
#include <QString>
#include <QStringList>

#include "analitzaexport.h"

/**
	@author Aleix Pol i Gonzalez
*/

typedef enum {
	S,
	R,
	A,
	E1,
	E2,
	E3,
	E,
	K
} actEnum;

typedef enum {
	tAssig, 	//:=
	tLambda,	//->
	tLimits,	//..
	tAdd,		//+
	tSub,		//-
	tMul,		//*
	tDiv,		// /
	tPow,		//^
	tUmi,		//-a
	tUqm,		//?a
	tFunc,		//f(x)
	tBlock,		//b{x}
	tQm,		//?
	tComa,		//,
	tLpr,		//(
	tRpr,		//)
	tLcb,		//{ Curly bracket!
	tRcb,		//}
	tEof,
	tMaxOp,
	tVal
} tokEnum;

struct TOKEN {
	QString val;
	tokEnum tipus;
};

// void printPilaOpr(QValueStack<int> opr);
// QString opr2str(int);

class ANALITZA_EXPORT Exp
{
public:
	Exp(QString);
	~Exp();
	
	int parse();
	QStringList error();
	QString mathML();
	
	bool isCompletelyRead() const { return completelyRead==0; }
	
	static TOKEN getToken(QString &a, int &l, tokEnum prev);
private:
	QString mml;
	QString str; //Auxiliar pel parsing
	QStringList err;
	
	tokEnum tok;
	QString tokval;
	
	QStack<int> opr;
	QStack<QString> val;
	QStack<QString> func;
	
	int valTop, oprTop;
	bool firsttok;
	tokEnum antnum;
	
	int getTok();
	int shift();
	int reduce();
	int completelyRead;
};

#endif
