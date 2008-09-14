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

#ifndef EXPLEXER_H
#define EXPLEXER_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include "analitzaexport.h"

class ANALITZA_EXPORT ExpLexer
{
	public:
		typedef struct {
			QString val;
			int type;
			QString error;
		} TOKEN;
		
		ExpLexer(const QString &source);
		~ExpLexer();
		int lex();
		int lineNumber() const { return 0; }
		
		TOKEN current;
	
		static TOKEN getToken(QString &a, int &l);
	private:
		QChar next();
		
		QString m_source;
		static QMap<QChar, int> m_operators;
		static QMap<QString, int> m_longOperators;
// 		static QMap<int, QString> m_operatorTags;
};

#endif
