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

#ifndef OBJECT_H
#define OBJECT_H

#include <QDebug>

// #include <typeinfo>

#include "analitzaexport.h"

// OperatorsModel m_words;
/**
 *	\internal
 *	The atomic mean of the expression trees. Should never used alone, always inherited.
 *	@author Aleix Pol <aleixpol@gmail.com>
 */

//FIXME: Check for public -> protected on some members
class ANALITZA_EXPORT Object
{
public:
	/** ObjectType is used to describe objects. */
	enum ObjectType {
		none=0,		/**< No object type, usually means an error. */
		value,		/**< Describes an object as a value. */
		variable,	/**< Describes an object as a variable. */
		oper,		/**< Describes an object as an operator. */
		container	/**< Describes an object as a container. */
	};
	
	/** Describes the nature of the value. */
	enum ValueType { Null=0, Real, Vector };
	
	/** Object destructor. Does nothing. */
	virtual ~Object() { /*qDebug() << "Destroying " << this;*/}
	
	/** Assigns a type @p t value to the object */
	void setType(enum ObjectType t) { m_type=t; }
	
	/** Returns the object type of the object */
	enum ObjectType type() const { return m_type; }
	
	/** Returns whether it is a container or not. */
	bool isContainer() const { return m_type==container; }
	
	/** Returns the string representation of the object. */
	virtual QString toString() const = 0;
	
	/** Returns the MathML representation of the object. */
	virtual QString toMathML() const = 0;
	
	/** Returns a HTML representation of the object. */
	virtual QString toHtml() const = 0;
	
	/** Converts a @p tag to a type. */
	static enum ObjectType whatType(const QString& tag); //FIXME: Needed?
	
	/** Returns whether it is a correct object or not */
	virtual bool isCorrect() const = 0;
	
	virtual void negate() {}
	
	virtual bool isZero() const { return false; }
	
	void setCorrect(bool b) { m_correct = b; }
	
	ValueType valueType() const;
protected:
	/** Creates an object with a @p t type */
	Object(enum ObjectType t) : m_correct(true), m_type(t) {}
	
	/** If it is marked as true, it is an invalid object, otherwise it may be wrong. */
	bool m_correct;
	
	/** Specifies the Object type. */
	enum ObjectType m_type;
};

/** A variable object, name refers to MathML standard. */
class Ci : public Object
{
	public:
		/** Constructor. Builds a @p o variable */
		explicit Ci(const Object *o);
		
		/** Constructor. Creates a variable with a @p b name */
		explicit Ci(QString b=QString()) : Object(variable), m_name(b), m_function(false) {}
		
		/** Sets a @p n name to a variable */
		void setName(const QString& n) { m_name=n; }
		
		/** Returns the variable name */
		QString name() const { return m_name; }
		
		/** Returns whether @p var name is equal to this variable one. */
		bool operator==(const Ci& var) { return var.m_name==m_name; }
		
		/** Sets whether it is a function. */
		void setFunction(bool f) { m_function=f; }
		
		/** Returns whether it is a variable that has to be a function */
		bool isFunction() const { return m_function; }
		
		/** Returns the string expression representation of the variable */
		QString toString() const { return m_name; }
		
		/** Returns the MathML representation of the variable */
		QString toMathML() const;
		
		/** Returns the HTML representation of the variable */
		QString toHtml() const { return QString("<span class='%1'>%2</span>").arg(m_function ? "func" : "var").arg(m_name); }
		
		/** Returns whether it is a correct object. */
		bool isCorrect() const { return m_type==Object::variable && !m_name.isEmpty(); }
	private:
		QString m_name;
		bool m_function;
};

#endif
