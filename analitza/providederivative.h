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


#ifndef PROVIDEDERIVATIVE_H
#define PROVIDEDERIVATIVE_H
#include "abstractexpressiontransformer.h"
#include <QList>
#include <QPair>
#include <QStringList>

namespace Analitza
{

class ProvideDerivative : public AbstractExpressionTransformer
{
	public:
		ProvideDerivative(const QString& var);
		Object* run(const Object* o);
		
		bool isCorrect() const { return m_errors.isEmpty(); }
		QStringList errors() const { return m_errors; }
		
	private:
		QStringList m_errors;
		virtual Object* walkApply(const Analitza::Apply* pattern);
		
		Object* derivateContentVector(const Vector* v);
		Object* derivateContentList(const List* v);
		Object* derivativeContainer(const Container *c);
		Object* derivativeApply(const Apply *c);
		
		Object* makeDiff(const Analitza::Object* o) const;
		
		typedef QPair<const Object*, const Object*> Transformation;
		static QList<Transformation> s_transformations;
		QString var;
};

}
#endif // PROVIDEDERIVATIVE_H
