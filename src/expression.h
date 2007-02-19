/***************************************************************************
 *   Copyright (C) 2006 by Aleix Pol                                       *
 *   aleixpol@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QtXml>
#include "container.h"


/**
	@author Aleix Pol <aleixpol@gmail.com>
*/
class Expression
{
	friend class Analitza;
	friend class VarEdit;
	
	public:
		Expression();
		Expression(const Expression& e);
		Expression(const QString& exp, bool mathml);
		~Expression();
		
		bool setText(const QString &exp);
		bool setMathML(const QString &exp);
		QStringList error() const { return m_err; }
		bool isCorrect() const { return m_tree && m_err.isEmpty(); }
		Object* branch(const QDomElement& elem);
		bool operator==(const Expression& e) const;
		Expression operator=(const Expression& e);
		Cn uplimit() const;
		Cn downlimit() const;
		const Object* tree() const { return m_tree; }
		
		QString toString() const;
		QString toMathML() const;
		
		static enum Object::ObjectType whatType(const QString& tag);
		static bool isMathML(const QString& s) { return !s.isEmpty() && s[0]=='<'; }
		static Cn uplimit(const Container& c);
		static Cn downlimit(const Container& c);
		static Object* objectCopy(const Object *);
// 	protected:
		Object* m_tree;
	private:
		QStringList m_err;
};

#endif
