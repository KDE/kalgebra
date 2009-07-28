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

#ifndef MATHMLPRESENTATIONLEXERLEXER_H
#define MATHMLPRESENTATIONLEXERLEXER_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QStack>
#include <QtCore/QXmlStreamReader>
#include "abstractlexer.h"
#include "analitzaexport.h"

class ANALITZA_EXPORT MathMLPresentationLexer : public AbstractLexer
{
	public:
		
		MathMLPresentationLexer(const QString &source);
		
	private:
		void getToken();
		QXmlStreamReader m_xml;
		QStack<QString> m_tags;
		QMap<QString, TOKEN> m_tokenTags;
};

#endif
