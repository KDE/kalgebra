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

#ifndef VARIABLES_H
#define VARIABLES_H

#include <QHash>

#include "analitzaexport.h"

namespace Analitza
{

class Cn;
class Expression;
class Object;

/**
*	Stores the variables in a hash map and make them available 
*	@author Aleix Pol <aleixpol@kde.org>  
*/

class ANALITZA_EXPORT Variables : public QHash<QString, Object*>
{
	public:
		/** 
		*	Creates an empty variable hash table with the usual constants
		*/
		Variables();
		
		/**
		*	Copy constructor, copies the old one, a bit heavy, be careful.
		*/
		Variables(const Variables& v);
		
		/** Destroys the object */
		~Variables();
		
		/**
		*	Modifies the value of the variable called @p name,
		*	and if didn't exist, a @p name variable is created with an @p o value.
		*/
		void modify(const QString& name, const Object* o);
		
		/**
		*	Modifies the value of the variable called @p name,
		*	and if didn't exist, a @p name variable is created with an @p e expression.
		*/
		void modify(const QString& name, const Expression& o);
		
		/**
		*	The same as the last one but having @p d as a value for @p name.
		*/
		Cn* modify(const QString& name, const double& d);
		
		/**
		*	The @p orig named variable will be called @p dest , then @p orig will be removed.
		*/
		void rename(const QString& orig, const QString& dest);
		
		/**
		 * Adds again the initial constants
		 */
		void initializeConstants();
};

}
#endif
