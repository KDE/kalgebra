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
#include <QtCore/QQueue>
#include "analitzaexport.h"

class ANALITZA_EXPORT ExpLexer
{
	public:
		struct TOKEN
		{
			TOKEN(int _type, uint _pos, const QString& _val=QString(), unsigned char _len=0) :
				type(_type), val(_val), len(_len), pos(_pos) {}
			
			int type;
			QString val;
			unsigned char len;
			uint pos;
		};
		
		ExpLexer(const QString &source);
		~ExpLexer();
		int lex();
		int lineNumber() const { return 0; }
		
		TOKEN current;
		QString error() const { return m_err; }
	private:
		void getToken(const QString &a, int &p);
		QString m_err;
		QChar next();
		int m_pos;
		
		QString m_source;
		QQueue<TOKEN> m_tokens;
		static QMap<QChar, int> m_operators;
		static QMap<QString, int> m_longOperators;
};

#endif
