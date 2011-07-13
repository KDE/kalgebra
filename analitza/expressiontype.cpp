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
namespace Analitza { void printAssumptions(const QString& prefix, const ExpressionType& current); }
QDebug operator<<(QDebug dbg, const QMap<int, Analitza::ExpressionType> &c);

bool ExpressionType::assumptionsMerge(QMap<QString, ExpressionType>& data, const QMap<QString, ExpressionType>& newmap)
{
	if(data.isEmpty() && newmap.isEmpty())
		return true;
	
// 	static int i=0;
// 	qDebug() << "merging!" << i++ << data << newmap;
	QMap<int, ExpressionType> stars;
	QMap<QString, ExpressionType>::const_iterator it=newmap.constBegin(), itEnd=newmap.constEnd();
	for(; it!=itEnd; ++it) {
		QMap<QString, ExpressionType>::iterator current = data.find(it.key());
		
		if(current!=data.end()) {
			if(!current->isError()) {
				ExpressionType t=ExpressionType::minimumType(*it, *current);
// 				qDebug() << "miiiiiiin" << it.key() << *it << *current << t << "|||" << it->assumptions() << current->assumptions() << t.assumptions();
				if(t.isError())
					return false;
				
				stars=computeStars(stars, *it, *current);
				*current = t.starsToType(stars);
			}
		} else
			data.insert(it.key(), it.value());
	}
// 	qDebug() << "merging_" << data;
	
	for(QMap<QString, ExpressionType>::iterator it=data.begin(); it!=data.end(); ++it) {
		*it=it->starsToType(stars);
	}
	
// 	i--;
	return true;
}

void ExpressionType::assumptionsUnion(QMap<QString, ExpressionType>& data, const QMap<QString, ExpressionType>& newmap)
{
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
	
// 	qDebug() << "leeeeee" << data;
}

ExpressionType::ExpressionType(const ExpressionType& t)
    : m_type(t.m_type), m_contained(t.m_contained)
	, m_assumptions(t.m_assumptions), m_size(t.m_size), m_objectName(t.m_objectName)
{}

ExpressionType::ExpressionType(ExpressionType::Type t, int any)
    : m_type(t), m_any(any)
{Q_ASSERT(m_type==Any || m_type==Error || m_type==Value || m_type==Bool || m_type==Char || m_type==Many || m_type==Lambda); }

ExpressionType::ExpressionType(ExpressionType::Type t, const ExpressionType& contained, int s)
    : m_type(t), m_contained(QList<ExpressionType>() << contained), m_size(s)
{
	Q_ASSERT(m_type==List || m_type==Vector);  Q_ASSERT(m_type!=Vector || m_size!=0);
	m_assumptions=contained.assumptions();
}

ExpressionType::ExpressionType(const QString& objectName)
	: m_type(Object), m_size(-1), m_objectName(objectName)
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
        case ExpressionType::Value:ret="num";  break;
        case ExpressionType::Char: ret="char"; break;
        case ExpressionType::Bool: ret="bool"; break;
        case ExpressionType::List:
            ret='['+typesToString(m_contained).join("$")+']';
            break;
        case ExpressionType::Vector:
            ret='<'+typesToString(m_contained).join("$")+','+QString::number(m_size)+'>';
            break;
        case ExpressionType::Error:
            ret="err";
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

void ExpressionType::removeAssumptions(const QStringList& bvarStrings)
{
	foreach(const QString& bvar, bvarStrings)
		m_assumptions.remove(bvar);
	
	QList<ExpressionType>::iterator it=m_contained.begin(), itEnd=m_contained.end();
	for(; it!=itEnd; ++it) {
		it->removeAssumptions(bvarStrings);
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
	for(QMap< int, ExpressionType >::const_iterator it=info.begin(); it!=info.end(); ++it) {
		Q_ASSERT(it->type()!=ExpressionType::Any || it->anyValue()!=it.key());
	}
	
	ExpressionType ret;
	static int deep=0;
// 	qDebug() << qPrintable("hohohoho"+QString(++deep, '-')) << *this << info << &info;
	
// 	qDebug() << ".........." << *this << info.keys();
	if((m_type==ExpressionType::Any || (m_type==ExpressionType::Vector && m_size<0)) && info.contains(m_any)) {
		if(m_type==ExpressionType::Any)
			ret=info.value(m_any);
		else {
			ret=info.value(m_size);
			
			QMap<int, ExpressionType> info2(info);
			info2.remove(m_size);
			ret.m_contained.first()=ret.m_contained.first().starsToType(info2);
		}
		
		ret.m_assumptions=m_assumptions;
// 		bool b=assumptionsMerge(ret.m_assumptions, m_assumptions);
// 		Q_ASSERT(b);
	} else {
		ret=*this;
		
		bool hasMany=false;
		for(QList<ExpressionType>::iterator it=ret.m_contained.begin(), itEnd=ret.m_contained.end(); it!=itEnd; ++it) {
			*it=it->starsToType(info);
			hasMany|=(it->type()==ExpressionType::Many);
		}
		
		if(hasMany) {
			QList< ExpressionType > args=manyFromArgs(ret.m_contained);
			QList<ExpressionType> alts;
			foreach(const ExpressionType& t, args) {
				ExpressionType tt=ret;
				tt.m_contained=t.m_contained;
				
				bool b=assumptionsMerge(tt.m_assumptions, t.m_assumptions);
				if(b)
					alts += tt;
			}
			ret=ExpressionType(ExpressionType::Many, alts);
		}
	}
	
	for(QMap<QString, ExpressionType>::iterator it=ret.m_assumptions.begin(), itEnd=ret.m_assumptions.end(); it!=itEnd; ++it) {
		*it=it->starsToType(info);
	}
	
// 	qDebug() << qPrintable("fuuuuuuu"+QString(deep, '-')) << info << &info;
	
	deep--;
// 	qDebug() << "MMMMMMMM" << ret << ret.assumptions() << m_assumptions;
	
	return ret;
}

bool ExpressionType::canReduceTo(const ExpressionType& type) const
{
	bool ret=false;
	
	if(type==*this || m_type==Any || isError())
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
			if(canReduceTo(t)) {
				ret=true;
				break;
			}
		}
	} else if(m_type==Lambda) {
		ret = m_contained.size()==type.m_contained.size();
		QMap<int, ExpressionType> reductionsApplied;
		for(int i=0; ret && i<m_contained.size(); i++) {
			ExpressionType a=m_contained[i].starsToType(reductionsApplied), b=type.m_contained[i].starsToType(reductionsApplied);
			ret&=a.canReduceTo(b);
			
			if(ret && a.type()==Any && a!=b) {
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

QMap<QString, ExpressionType> ExpressionType::assumptions() const
{
// 	printAssumptions(*this);
	return m_assumptions;
}

QMap< QString, ExpressionType >& ExpressionType::assumptions()
{
	return m_assumptions;
}

int ExpressionType::increaseStars(int stars)
{
	int ret=stars;
	
	if(m_type==ExpressionType::Any) {
		m_any=stars+m_any;
		if(m_any>ret) 
			ret=m_any+1;
	}
	
	for(QList<ExpressionType>::iterator it=m_contained.begin(), itEnd=m_contained.end(); it!=itEnd; ++it) {
		ret=qMax(it->increaseStars(stars), ret);
	}
	
	for(QMap<QString,ExpressionType>::iterator it=m_assumptions.begin(), itEnd=m_assumptions.end(); it!=itEnd; ++it) {
		ret=qMax(it->increaseStars(stars), ret);
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
		case ExpressionType::Lambda:
			for(QList<ExpressionType>::iterator it=t.m_contained.begin(), itEnd=t.m_contained.end(); it!=itEnd; ++it) {
				starsSimplification(*it, reductions, next);
			}
			break;
		case ExpressionType::Object:
		case ExpressionType::Value:
		case ExpressionType::Char:
		case ExpressionType::Bool:
		case ExpressionType::Error:
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
			if(t.canReduceTo(type))
				newcontained.append(t);
		}
		
		if(newcontained.size()==1) {
			bool b=assumptionsMerge(newcontained.first().assumptions(), m_assumptions);
			Q_ASSERT(b);
			*this = newcontained.first();
		} else if(newcontained.isEmpty())
			;
		else
			m_contained = newcontained;
		
	}
	
	if(m_type == Many) {
		QList<ExpressionType>::iterator it=m_contained.begin(), itEnd=m_contained.end();
		for(; it!=itEnd; ) {
			bool b = assumptionsMerge(it->assumptions(), type.assumptions());
			
// 			qDebug() << "LLLLL" << b << *it << it->assumptions();
			if(b)
				++it;
			else
				it=m_contained.erase(it);
		}
	} else {
		bool b = assumptionsMerge(m_assumptions, type.assumptions());
		if(!b)
			*this = ExpressionType(Error);
		else if(m_type==ExpressionType::Any && *this!=type) {
			QMap<int, ExpressionType> t;
			t.insert(m_any, type);
			
			QMap<QString, ExpressionType> resassumptions=assumptions();
			*this = type.starsToType(t);
			addAssumptions(resassumptions);
		}
	}
	
	if(m_type==Lambda && type.m_type==Lambda && canReduceTo(type)) {
		QList< ExpressionType >::iterator it1=m_contained.begin();
		QList< ExpressionType >::const_iterator it2=type.m_contained.constBegin();
		
		for(; it1!=m_contained.end(); ++it1, ++it2) {
			*it1=minimumType(*it1, *it2);
		}
	}
}

ExpressionType ExpressionType::minimumType(const ExpressionType& t1, const ExpressionType& t2)
{
	if(t1.type()==ExpressionType::Many && t2.type()==ExpressionType::Many) {
		QList<ExpressionType> alts;
// 		qDebug() << "IIIIIIIIII" << t1 << t2;
		foreach(const ExpressionType& alt1, t1.alternatives()) {
			foreach(const ExpressionType& alt2, t2.alternatives()) {
				if(alt1.canReduceTo(alt2)) {
					ExpressionType alt(alt1);
					alt.reduce(alt2);
					
					if(!alt.isError()) {
						alts+=alt;
					}
				}
			}
		}
		
		if(alts.isEmpty())
			return ExpressionType(ExpressionType::Error);
		else {
// 			qDebug() << "aaaaaaaaaaaaaaaaaaaaaaaaaaa" << alts << "\n\t --- " << alts.first().assumptions() << "\n\t --- " << alts.last().assumptions()
// 						<< "\n\t --- " << t1.assumptions() << t2.assumptions();
			ExpressionType ret=ExpressionType(ExpressionType::Many, alts);
			ret.addAssumptions(t1.assumptions());
			ret.addAssumptions(t2.assumptions());
			return ret;
		}
	}
	else if(t2.isError())
		return t1;
	else if(t1.isError())
		return t2;
	else if(t1.type()==ExpressionType::Any && t2.type()==ExpressionType::Any)
		return t1.anyValue()>t2.anyValue() ? t1 : t2;
	else if(t1.type()==ExpressionType::Lambda && t2.type()==ExpressionType::Lambda && t1.parameters().size()==t2.parameters().size()) {
		ExpressionType t(ExpressionType::Lambda);
		for(int i=0; i<t1.parameters().size(); i++) {
			t.addParameter(minimumType(t1.parameters()[i], t2.parameters()[i]));
		}
		if(!t.isError()) {
			t.addAssumptions(t1.assumptions());
			t.addAssumptions(t2.assumptions());
		}
		
		return t;
	} else if(t1.canReduceTo(t2)) {
// 		qDebug() << "zas" << t1 << t2 << "|||" << t1.assumptions() << t2.assumptions();
		ExpressionType t(t2);
		t.reduce(t1);
// 		qDebug() << "zus" << t << t.assumptions();
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

QMap<int, ExpressionType> ExpressionType::computeStars(const QMap<int, ExpressionType>& initial, const ExpressionType& candidate, const ExpressionType& type)
{
	QMap<int, ExpressionType> ret(initial);
	
	switch(candidate.type()) {
		case ExpressionType::Any: {
			int stars=candidate.anyValue();
			
			if(ret.contains(stars)) {
				ExpressionType t=ret.take(stars);
				ret=computeStars(ret, t, type);
				
				t=t.starsToType(ret);
				if(t.type()!=ExpressionType::Any || t.m_any!=stars)
					ret.insert(stars, t);
			} else if(type.type()!=ExpressionType::Any || type.m_any!=stars) {
				ret[stars]=type;
			}
		}	break;
		case ExpressionType::List:
			if(type.type()==ExpressionType::List) { // remove? assert?
				ret=computeStars(initial, candidate.contained(), type.contained());
			}
			break;
		case ExpressionType::Vector:
			if(type.type()==ExpressionType::Vector) { // remove?
				ret=computeStars(initial, candidate.contained(), type.contained());
				
				if(candidate.size()<0) {
					ExpressionType t=type;
					t.m_contained.first()=t.m_contained.first().starsToType(ret);
					ret[candidate.size()] = t;
				}
			}
			break;
		case ExpressionType::Lambda:
			if(type.type()!=ExpressionType::Lambda)
				break;
			Q_ASSERT(type.parameters().size()==candidate.parameters().size());
			for(int i=0; i<type.parameters().size(); i++) {
				ret=computeStars(ret, candidate.parameters()[i], type.parameters()[i]);
			}
			break;
		case ExpressionType::Many:
		case ExpressionType::Object:
		case ExpressionType::Value:
		case ExpressionType::Bool:
		case ExpressionType::Char:
		case ExpressionType::Error:
// 			Q_ASSERT(false && "bffff");
			break;
	}
	
	for(QMap<int, ExpressionType>::iterator it=ret.begin(), itEnd=ret.end(); it!=itEnd; ++it) {
		ExpressionType t=it->starsToType(ret);;
		if(t.type()!=ExpressionType::Any || t.m_any!=it.key()) {
			*it=t;
		}
	}
	
// 	qDebug() << ";;;;;;;" << candidate.type() << candidate << type << ret;
// 	bool b = matchAssumptions(&ret, type.assumptions(), candidate.assumptions());
// 	qDebug() << "feeeee" << ret;
	
	return ret;
}

bool ExpressionType::matchAssumptions(QMap< int, ExpressionType >* stars,
												const QMap< QString, ExpressionType >& assum1,
												const QMap< QString, ExpressionType >& assum2)
{
	bool ret=true;
	QMap<QString, ExpressionType>::const_iterator it=assum1.constBegin(), itEnd=assum1.constEnd();
	QMap<QString, ExpressionType>::const_iterator itFind, itFindEnd=assum2.constEnd();
	
	for(; ret && it!=itEnd; ++it) {
		itFind=assum2.find(it.key());
		
		if(itFind!=itFindEnd && *itFind!=*it) {
// 			qDebug() << "sih" << it.key() << *it << *itFind << (itFind.value()!=it.value());
			if(itFind->canReduceTo(*it) || it->canReduceTo(*itFind))
				*stars=ExpressionType::computeStars(*stars, *itFind, *it);
			else
				ret=false;
// 			qDebug() << "seh" << *stars << ret;
		}
	}
	
	return ret;
}

QList<ExpressionType> ExpressionType::manyFromArgs(const QList<ExpressionType>& args)
{
	QList<ExpressionType> funcs = QList<ExpressionType>() << ExpressionType(ExpressionType::Many);
	
	foreach(const ExpressionType& e, args) {
		QList<ExpressionType> alts = e.type()==ExpressionType::Many ? e.alternatives() : QList<ExpressionType>() << e;
		QList<ExpressionType> newfuncs;
		foreach(const ExpressionType& rr, alts) {
			foreach(const ExpressionType& f, funcs) {
				newfuncs += f;
				
				QMap<int, ExpressionType> stars;
// 				qDebug() << "&&&&&&&" << rr << newfuncs.last().assumptions() << rr.assumptions();
				bool m=ExpressionType::assumptionsMerge(newfuncs.last().assumptions(), rr.assumptions());
				m|=ExpressionType::matchAssumptions(&stars, newfuncs.last().assumptions(), rr.assumptions());
// 				qDebug() << "falala" << m;
				
				if(m)
					newfuncs.last().addAlternative(rr.starsToType(stars));
				else
					newfuncs.removeLast();
			}
		}
		
		funcs = newfuncs;
	}
	
	return funcs;
}

QList<ExpressionType> ExpressionType::lambdaFromArgs(const QList<ExpressionType>& args)
{
	QList<ExpressionType> funcs = QList<ExpressionType>() << ExpressionType(ExpressionType::Lambda);
	
	foreach(const ExpressionType& e, args) {
		QList<ExpressionType> alts = e.type()==ExpressionType::Many ? e.alternatives() : QList<ExpressionType>() << e;
		QList<ExpressionType> newfuncs;
		foreach(const ExpressionType& rr, alts) {
			foreach(const ExpressionType& f, funcs) {
				newfuncs += f;
				
				QMap<int, ExpressionType> stars;
// 				qDebug() << "&&&&&&&" << rr << newfuncs.last().assumptions() << rr.assumptions();
				bool m=ExpressionType::assumptionsMerge(newfuncs.last().assumptions(), rr.assumptions());
				m|=matchAssumptions(&stars, newfuncs.last().assumptions(), rr.assumptions());
// 				qDebug() << "falala" << m;
				
				if(m)
					newfuncs.last().addParameter(rr.starsToType(stars));
				else
					newfuncs.removeLast();
			}
		}
		
		funcs = newfuncs;
	}
	
	return funcs;
}
