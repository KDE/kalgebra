/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "expressiontype.h"
#include <QStringList>
#include <QDebug>

using namespace Analitza;

QDebug operator<<(QDebug dbg, const ExpressionType &c);

bool ExpressionType::assumptionsMerge(QMap<QString, ExpressionType>& data, const QMap<QString, ExpressionType>& newmap)
{
	bool ret=true;
	QMap<QString, ExpressionType>::const_iterator it=newmap.constBegin(), itEnd=newmap.constEnd();
	for(; it!=itEnd; ++it) {
		QMap<QString, ExpressionType>::iterator current = data.find(it.key());
		
		if(current!=data.end()) {
			if(!current->isError()) {
				ExpressionType t=ExpressionType::minimumType(*it, *current);
				if(t.isError()) {
					return false;
				}
				data.insert(it.key(), t);
			}
		} else
			data.insert(it.key(), it.value());
	}
	
	return ret;
}

bool ExpressionType::assumptionsUnion(QMap<QString, ExpressionType>& data, const QMap<QString, ExpressionType>& newmap)
{
	bool ret=true;
// 	qDebug() << "-----------" << data << newmap;
	QMap<QString, ExpressionType>::const_iterator it=newmap.constBegin(), itEnd=newmap.constEnd();
	for(; it!=itEnd; ++it) {
		QMap<QString, ExpressionType>::iterator current = data.find(it.key());
		
		if(current!=data.end()) {
			if(current->canReduceTo(*it)) {
				data.insert(it.key(), minimumType(*current, *it));
			} else {
				ExpressionType t(Many);
				ExpressionType t1(*it);			t1.addAssumption(it.key(), *it);
				ExpressionType t2(*current);	t2.addAssumption(it.key(), *current);
				
				t.addAlternative(t1);
				t.addAlternative(t2);
				
				data.insert(it.key(), t);
			}
		} else
			data.insert(it.key(), it.value());
	}
	
// 	qDebug() << "leeeeee" << ret << data;
	return ret;
}

ExpressionType::ExpressionType(const ExpressionType& t)
    : m_type(t.m_type), m_contained(t.m_contained)
	, m_assumptions(t.m_assumptions), m_size(t.m_size), m_objectName(t.m_objectName)
{}

ExpressionType::ExpressionType(ExpressionType::Type t, int any)
    : m_type(t), m_any(any)
{}

ExpressionType::ExpressionType(ExpressionType::Type t, const ExpressionType& contained, int s)
    : m_type(t), m_contained(QList<ExpressionType>() << contained), m_size(s)
{
	Q_ASSERT(m_type==List || m_type==Vector);  Q_ASSERT(m_type!=Vector || m_size!=0);
	m_assumptions=contained.assumptions();
}

ExpressionType::ExpressionType(const QString& objectName)
	: m_type(Object), m_objectName(objectName)
{}

ExpressionType::ExpressionType(ExpressionType::Type t, const QList< ExpressionType >& alternatives)
	: m_type(Many)
{
	Q_ASSERT(t==Many);
	foreach(const ExpressionType& t, alternatives)
		addAlternative(t);
}

QStringList typesToString(const QList<ExpressionType>& types)
{
	QStringList ret;
	foreach(const ExpressionType& t, types) {
		QString str=t.toString();
		if(t.type()==ExpressionType::Lambda)
			ret += '('+str+')';
		else
			ret += str;
	}
	return ret;
}

QString ExpressionType::toString() const
{
    QString ret;
    switch(m_type) {
        case ExpressionType::Value:
            ret="num";
            break;
        case ExpressionType::List:
            ret='['+typesToString(m_contained).join("$")+']';
            break;
        case ExpressionType::Vector:
            ret='<'+typesToString(m_contained).join("$")+','+QString::number(m_size)+'>';
            break;
        case ExpressionType::Error:
            ret="err";
            break;
        case ExpressionType::Undefined:
            ret="undef";
            break;
        case ExpressionType::Lambda:
            ret=typesToString(m_contained).join(" -> ");
			break;
        case ExpressionType::Any: {
//             ret=QString(m_any, '*');
			QString id;
			for(int i=m_any; i>0; i/='z'-'a') {
				id.prepend(QChar('a'-1+(i%('z'-'a'))));
			}
			ret=id;
// 			ret+=QString::number(m_any);
		}	break;
        case ExpressionType::Many:
            ret=/*"{"+*/typesToString(m_contained).join(" | ")/*+"}"*/;
			break;
		case ExpressionType::Object:
			ret="obj:"+m_objectName;
			break;
    }
    
    return ret;
}

bool ExpressionType::isError() const
{
    if(m_type==Error)
        return true;
    foreach (const ExpressionType& t, m_contained) {
        if(t.isError())
            return true;
    }
    return false;
}

ExpressionType ExpressionType::operator=(const ExpressionType& et)
{
    if(&et!=this) {
        m_type=et.m_type;
        m_contained=et.m_contained;
        m_size=et.m_size;
        m_assumptions=et.m_assumptions;
		m_objectName=et.m_objectName;
    }
    
    return *this;
}

bool ExpressionType::operator==(const ExpressionType& t) const
{
// 	qDebug() <<"seeee" << m_type << t.m_type << t.m_contained << m_contained << t << *this;
	bool ret=t.m_type==m_type
				&& (m_type==ExpressionType::Any ? m_any==t.m_any : (m_size<1 || t.m_size<1 ? true : m_size==t.m_size) )
				&& t.m_contained==m_contained;
	
	if(!ret && t.type()==ExpressionType::Many && t.alternatives().size()==1)
		ret=*this==t.alternatives().first();
	
	if(!ret &&   type()==ExpressionType::Many &&   alternatives().size()==1)
		ret=t==alternatives().first();
	
	ret &= m_objectName==t.m_objectName;
// 	qDebug() << "++++++++" << t << *this << ret;
	return ret;
}

void ExpressionType::addAssumption(const QString& bvar, const Analitza::ExpressionType& t)
{
	ExpressionType toadd=t;
	addAssumptions(t.assumptions());
	toadd.clearAssumptions();
	
	QMap< QString, ExpressionType >::iterator it=m_assumptions.find(bvar);
	if(it==m_assumptions.end())
		m_assumptions.insert(bvar, toadd);
	else {
		toadd=minimumType(toadd,*it);
		Q_ASSERT(!toadd.isError());
		*it=toadd;
	}
}

void ExpressionType::addAssumptions(const QMap<QString, ExpressionType>& a)
{
// 	qDebug() << "=====1" << m_assumptions << a;
	bool valid=assumptionsMerge(m_assumptions, a);
// 	qDebug() << "=====2" << m_assumptions;
	Q_ASSERT(valid);
}

ExpressionType ExpressionType::starsToType(const QMap< int, ExpressionType>& info) const
{
	ExpressionType ret;
// 	static int deep=0;
// 	qDebug() << qPrintable("hohohoho"+QString(++deep, '-')) << *this << info;
	
// 	qDebug() << ".........." << *this << info.keys();
	if((m_type==ExpressionType::Any || (m_type==ExpressionType::Vector && m_size<0)) && info.contains(m_any)) {
		ret=info.value(m_any);
		
		ret.m_assumptions=m_assumptions;
	} else {
		ret=*this;
	}
	
	for(QList<ExpressionType>::iterator it=ret.m_contained.begin(), itEnd=ret.m_contained.end(); it!=itEnd; ++it) {
		*it=it->starsToType(info);
	}
	
	for(QMap<QString, ExpressionType>::iterator it=ret.m_assumptions.begin(), itEnd=ret.m_assumptions.end(); it!=itEnd; ++it) {
		*it=it->starsToType(info);
	}
// 	deep--;
// 	qDebug() << "MMMMMMMM" << ret << ret.assumptions() << m_assumptions;
	
	return ret;
}

bool ExpressionType::canReduceTo(const ExpressionType& type) const
{
	bool ret=false;
	
	if(type==*this || m_type==Undefined || m_type==Any || isError())
		ret=true;
	else if(m_type==Many) {
		foreach(const ExpressionType& t, m_contained) {
			if(t.canReduceTo(type)) {
				ret=true;
				break;
			}
		}
	} else if(type.m_type==Many) {
		foreach(const ExpressionType& t, type.m_contained) {
			if(t.canReduceTo(*this)) {
				ret=true;
				break;
			}
		}
	} else if(m_type==Lambda && m_contained.size()==type.m_contained.size()) {
		ret = true;
		QMap<int, ExpressionType> reductionsApplied;
		for(int i=0; ret && i<m_contained.size(); i++) {
			ExpressionType a=m_contained[i].starsToType(reductionsApplied), b=type.m_contained[i].starsToType(reductionsApplied);
			ret&=a.canReduceTo(b);
			
			if(ret && a.type()==Any) {
				b.clearAssumptions();
				reductionsApplied.insert(a.anyValue(), b);
			}
		}
	} else if(m_type==Vector && type.m_type==Vector) {
		ret  = m_size<0 || type.m_size<0 || m_size==type.m_size;
		ret &= contained().canReduceTo(type.contained());
	} else if(m_type==List && type.m_type==List) {
		ret = contained().canReduceTo(type.contained());
	}
	
// 	qDebug() << "OOOOOOOOOOOOO" << *this << type << ret;
	
	return ret;
}

// void ExpressionType::printAssumptions(const ExpressionType& t, int ind) const
// {
// 	if(ind>10) return;
// 	
// 	qDebug()<< QString(ind, ':') << t.m_assumptions;
// 	foreach(const ExpressionType& ass, t.m_assumptions) {
// 		printAssumptions(ass, ind+1);
// 	}
// }

QMap<QString, ExpressionType> ExpressionType::assumptions() const
{
// 	printAssumptions(*this);
	return m_assumptions;
}

QMap< QString, ExpressionType >& ExpressionType::assumptions()
{
	return m_assumptions;
}

int ExpressionType::increaseStars(int stars, QMap<int, int>* values)
{
	int ret=stars;
	switch(m_type) {
		case ExpressionType::Any: {
			int old=m_any;
			m_any=stars+m_any;
			if(m_any>stars)
				stars=m_any+1;
			
			if(values) {
				values->insert(old, m_any);
			}
		}	break;
		case ExpressionType::Many:
		case ExpressionType::Vector:
		case ExpressionType::List:
		case ExpressionType::Lambda:
		case ExpressionType::Object:
		case ExpressionType::Value:
		case ExpressionType::Error:
		case ExpressionType::Undefined:
			break;
	}
	
	for(QList<ExpressionType>::iterator it=m_contained.begin(), itEnd=m_contained.end(); it!=itEnd; ++it) {
		ret=qMax(it->increaseStars(stars/*, values*/), stars);
	}
	
	for(QMap<QString,ExpressionType>::iterator it=m_assumptions.begin(), itEnd=m_assumptions.end(); it!=itEnd; ++it) {
		ret=qMax(it->increaseStars(stars/*, values*/), stars);
	}
	
	return ret;
}

void ExpressionType::starsSimplification(ExpressionType& t, QMap<int, int>& reductions, int &next)
{
	switch(t.m_type) {
		case ExpressionType::Any:
			if(reductions.contains(t.m_any))
				t.m_any=reductions.value(t.m_any);
			else {
				reductions.insert(t.m_any, next);
				t.m_any=next++;
			}
			
			break;
		case ExpressionType::Many:
		case ExpressionType::Vector:
		case ExpressionType::List:
		case ExpressionType::Object:
		case ExpressionType::Lambda: {
			QList<ExpressionType>::iterator it=t.m_contained.begin(), itEnd=t.m_contained.end();
			for(; it!=itEnd; ++it) {
				starsSimplification(*it, reductions, next);
			}
		}	break;
		case ExpressionType::Value:
		case ExpressionType::Error:
		case ExpressionType::Undefined:
			break;
	}
}

ExpressionType& ExpressionType::simplifyStars()
{
	int next=1;
	QMap<int, int> map;
	starsSimplification(*this, map, next);
	return *this;
}

ExpressionType ExpressionType::returnValue() const
{
	ExpressionType ret;
	
	if(m_type==Analitza::ExpressionType::Many) {
		ret=ExpressionType(ExpressionType::Many);
		foreach(const ExpressionType& t, m_contained)
			ret.addAlternative(t.returnValue());
	} else if(m_type==Analitza::ExpressionType::Lambda)
		ret=m_contained.last();
	
	return ret;
}

void ExpressionType::reduce(const Analitza::ExpressionType& type)
{
	if(m_type == Many) {
		QList<ExpressionType> newcontained;
		
		foreach(const ExpressionType& t, m_contained) {
			if(type.canReduceTo(t))
				newcontained.append(t);
		}
		
		if(newcontained.size()==1)
			*this = newcontained.first();
		else
			m_contained = newcontained;
	}
	
	if(m_type==Lambda && type.m_type==Lambda && canReduceTo(type)) {
		QList< ExpressionType >::iterator it1=m_contained.begin();
		QList< ExpressionType >::const_iterator it2=type.m_contained.constBegin();
		
		for(; it1!=m_contained.end(); ++it1, ++it2) {
			*it1=minimumType(*it1, *it2);
		}
	}
	
	bool b = assumptionsMerge(m_assumptions, type.assumptions());
	if(!b)
		*this = ExpressionType(Error);
// 	qDebug() << "done..." << *this;
}

ExpressionType ExpressionType::minimumType(const ExpressionType& t1, const ExpressionType& t2)
{
	if(t1.type()==ExpressionType::Many && t2.type()==ExpressionType::Many) {
		ExpressionType t(ExpressionType::Many);
		foreach(const ExpressionType& alt1, t1.alternatives()) {
			foreach(const ExpressionType& alt2, t2.alternatives()) {
				if(alt1.canReduceTo(alt2)) {
					ExpressionType alt(alt1);
					alt.reduce(alt2);
					
					if(!alt.isError())
						t.addAlternative(alt);
				}
			}
		}
// 		qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaaaa" << t << "\n\t --- " << t.alternatives().first().assumptions() << "\n\t --- " << t.alternatives().last().assumptions();
		return t;
	}
	else if(t2.isUndefined() || t2.isError())
		return t1;
	else if(t1.isUndefined() || t1.isError())
		return t2;
	else if(t1.type()==ExpressionType::Any && t2.type()==ExpressionType::Any)
		return t1.anyValue()<t2.anyValue() ? t1 : t2;
	else if(t1.canReduceTo(t2)) {
// 		qDebug() << "zas" << t1 << t2;
		ExpressionType t(t2);
		t.reduce(t1);
// 		qDebug() << "zas" << t;
		return t;
	} else if(t2.canReduceTo(t1)) {
		ExpressionType t(t1);
		t.reduce(t2);
		return t;
	}
	
	return ExpressionType();
}

ExpressionType& ExpressionType::addParameter(const ExpressionType& t)
{
	Q_ASSERT(m_type==Lambda);
	m_contained.append(t);
	return *this;
}

void ExpressionType::addAlternative(const ExpressionType& t)
{
	Q_ASSERT(m_type==Many);
	/*int i=m_contained.indexOf(t);
	if(i>=0) {
		m_contained[i].addAssumptions(t.assumptions());//TODO??
	} else */
	if(t.m_type==Many) {
		foreach(const ExpressionType& tt, t.alternatives()) 
			addAlternative(tt);
		
		addAssumptions(t.m_assumptions);
// 		m_contained=t.m_contained;
	} else
		m_contained.append(t);
}

void ExpressionType::clearAssumptions()
{
	m_assumptions.clear();
	QList< ExpressionType >::iterator it=m_contained.begin(), itEnd=m_contained.end();
	for(; it!=itEnd; ++it) {
		it->clearAssumptions();
	}
}

