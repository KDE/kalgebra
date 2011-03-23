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

#include "expressiontypechecker.h"
#include "variable.h"
#include "variables.h"
#include "list.h"
#include "vector.h"
#include "container.h"
#include "expression.h"
#include "analitzautils.h"
#include "operations.h"
#include <klocalizedstring.h>
#include "apply.h"

QDebug operator<<(QDebug dbg, const Analitza::ExpressionType &c)
{
	dbg.nospace() << "("<< qPrintable(c.toString());
// 	if(!c.assumptions().isEmpty())
// 		dbg.nospace() <<", " << c.assumptions();
	dbg.nospace() << /*":"<< c.type() <<*/")";
	
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Analitza::TypePair &c)
{
	dbg.nospace() << "["<< c.param << ":" << c.returnValue <<"]";
	
	return dbg.nospace();
}

QDebug operator<<(QDebug dbg, const Analitza::TypeTriplet &c)
{
	dbg.nospace() << "["<< c.param1 << ", " << c.param2 << ":" << c.returnValue <<"]";
	
	return dbg.nospace();
}

QDebug operator<<(QDebug dbg, const QMap<int, Analitza::ExpressionType> &c)
{
	dbg.nospace() << "starsMap(";
	for(QMap<int,Analitza::ExpressionType>::const_iterator it=c.constBegin(); it!=c.constEnd(); ++it) {
		QString str = Analitza::ExpressionType(Analitza::ExpressionType::Any, it.key()<0? -it.key():it.key()).toString();
		if(it.key()<0)
			str="<-"+str+"->";
		
		dbg.space() << "["<< str << ":" << it.value() << "]";
	}
	dbg.space() << ")";
	
	return dbg.nospace();
}

namespace Analitza
{

void printAssumptions(const QString& prefix, const ExpressionType& current)
{
	qDebug() << qPrintable(prefix) << current << current.assumptions();
	if(current.type()==ExpressionType::Many) {
		foreach(const ExpressionType& t, current.alternatives()) {
			qDebug() << qPrintable(prefix) << "XXXSSS" << t << t.assumptions();
		}
	}
	qDebug() << qPrintable(prefix) << "--";
}

ExpressionTypeChecker::ExpressionTypeChecker(Variables* v)
	: m_v(v)
{}

ExpressionType ExpressionTypeChecker::check(const Expression& exp)
{
	m_deps.clear();
	m_stars=1;
	current=ExpressionType(ExpressionType::Error);
	
	exp.tree()->visit(this);
	
// 	qDebug() << "cheeeeeeck" << m_vars;
	return current;
}

QString ExpressionTypeChecker::accept(const Operator*) { Q_ASSERT(false && "should not get here"); return QString(); }

bool ExpressionTypeChecker::inferType(const ExpressionType& found, const ExpressionType& targetType, QMap<QString, ExpressionType>* assumptions)
{
	Q_ASSERT(!targetType.isError() && assumptions);
	
	QMap<int, ExpressionType> stars;
	stars=ExpressionType::computeStars(stars, found, targetType);
	
	bool ret=found.canReduceTo(targetType);
	ret&=ExpressionType::assumptionsMerge(*assumptions, found.assumptions());
	
	if(ret && found.type()==ExpressionType::Many) {
		foreach(const ExpressionType& t, found.alternatives()) {
			ExpressionType::assumptionsUnion(*assumptions, t.assumptions());
		}
	}
	
	for(QMap<QString, ExpressionType>::iterator it=assumptions->begin(), itEnd=assumptions->end(); it!=itEnd; ++it) {
		*it=it->starsToType(stars);
	}
// 	qDebug() << "fuuuuu" << stars << *assumptions << ret;
	
	return ret;
}

QList<TypePair> ExpressionTypeChecker::computePairs(const QList<TypePair>& options, const ExpressionType& param)
{
	QList<TypePair> ret;
	
	if(param.type()==ExpressionType::Any) {
		int basestars=m_stars;
		foreach(const TypePair& opt, options) {
			TypePair toadd=opt;
			m_stars=qMax<int>(m_stars, toadd.increaseStars(basestars));
			QMap<int, ExpressionType> stars;
			
			//We know the parameter is any, we don't have to infer
			stars=ExpressionType::computeStars(stars, param, toadd.param);
			
			toadd.returnValue.addAssumptions(param.assumptions());
			toadd.returnValue=toadd.returnValue.starsToType(stars);
			ret += toadd;
		}
	} else 
		foreach(const TypePair& opt, options) {
// 			qDebug() << "TURURURU" << opt.param << param;
			if(opt.param.canReduceTo(param)) //Infer on param!=param but canReduce?
				ret += opt;
		}
	
// 	qDebug() << "_______" << param << exp->toString() << ret;
	return ret;
}

ExpressionType ExpressionTypeChecker::solve(const Operator* o, const QList< Object* >& parameters)
{
	Q_ASSERT(o->operatorType()!=Operator::function);
	
	ExpressionType ret(ExpressionType::Many);
	if(parameters.size()==1) {
		parameters.first()->visit(this);
		QList<ExpressionType> types=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
// 		const QMap<QString, ExpressionType> assumptions=current.assumptions();
		
		foreach(const ExpressionType& t, types) {
			QList<TypePair> thing=computePairs(Operations::inferUnary(o->operatorType()), t);
			foreach(const TypePair& opt, thing)
				ret.addAlternative(opt.returnValue);
		}
// 		qDebug() << "bam" << ret ;
	} else {
		Q_ASSERT(parameters.size()>=2);
// 		QMap<int, ExpressionType> stars;
		
		parameters.first()->visit(this);
		ExpressionType firstType=current;
		QList<ExpressionType> firstTypes= firstType.type()==ExpressionType::Many ?  firstType.alternatives() : QList<ExpressionType>() << firstType;
		
		QList<Object*>::const_iterator it=parameters.constBegin()+1, itEnd=parameters.constEnd();
		for(; it!=itEnd; ++it)
		{
			(*it)->visit(this);
			ExpressionType secondType=current;
			
			QList<ExpressionType> secndTypes=secondType.type()==ExpressionType::Many ? secondType.alternatives() : QList<ExpressionType>() << secondType;
			int starsbase=m_stars;
// 			static int ind=3;
// 			qDebug() << qPrintable("+" +QString(ind++, '-')) << o->toString() << firstType << secondType;
			const QList<TypeTriplet> res=Operations::infer(o->operatorType());
			foreach(const ExpressionType& _first, firstTypes) {
				foreach(const ExpressionType& _second, secndTypes) {
					QMap<int, ExpressionType> _starToType;
					if(!ExpressionType::matchAssumptions(&_starToType, _first.assumptions(), _second.assumptions()))
						continue;
					foreach(const TypeTriplet& _opt, res) {
						TypeTriplet opt(_opt);
						m_stars=qMax<int>(m_stars, opt.increaseStars(starsbase));
						
						Q_ASSERT(!opt.returnValue.isError());
						QMap<int, ExpressionType> starToType, starToParam;
						
						ExpressionType first =_first .starsToType(_starToType);
						ExpressionType second=_second.starsToType(_starToType);
// 						qDebug() << "9999999" << _second.assumptions() << second.assumptions() << starToType;
						
						starToType=ExpressionType::computeStars(starToType, first,  opt.param1);
						starToType=ExpressionType::computeStars(starToType, second, opt.param2);
						
						first =first .starsToType(starToType);
						second=second.starsToType(starToType);
						
						starToParam=ExpressionType::computeStars(starToParam, opt.param1, first);
						starToParam=ExpressionType::computeStars(starToParam, opt.param2, second);
// 						qDebug() << "XXXXXX" << starToParam;
// 						qDebug() << "PPPPPP" << opt << first << second << "|||||" << first.assumptions() << second.assumptions();
						
// 						starToType=ExpressionType::computeStars(starToType, first,  opt.param1.starsToType(starToParam));
// 						starToType=ExpressionType::computeStars(starToType, second, opt.param2.starsToType(starToParam));
						
// 						first =first .starsToType(starToType);
// 						second=second.starsToType(starToType);
						
						QMap<QString, ExpressionType> assumptions=first.assumptions();
						bool valid=ExpressionType::assumptionsMerge(assumptions, second.assumptions());
						
// 						qDebug() << "fifuuuuuuu" << first << (*(it-1))->toString() << 
// 													second << (*it)->toString() << assumptions << valid;
						
						valid &= first .canReduceTo(opt.param1.starsToType(starToParam));
						valid &= second.canReduceTo(opt.param2.starsToType(starToParam));
						
// 						qDebug() << "POPOPO" << (*(it-1))->toString() << (*(it))->toString() << valid << first << second << starToParam;
						if(valid) {
							ExpressionType toadd=opt.returnValue;
							toadd.addAssumptions(assumptions);
							toadd=toadd.starsToType(starToParam);
							
							ret.addAlternative(toadd);
						}
					}
				}
			}
// 			qDebug() << qPrintable("\\"+QString(--ind, '-')) << o->toString() << ret;
		}
	}
	
	if(ret.alternatives().isEmpty())
		return ExpressionType(ExpressionType::Error);
	else if(ret.alternatives().count()==1)
		return ret.alternatives().first();
	
	return ret;
}

QString ExpressionTypeChecker::accept(const Ci* var)
{
	if(m_typeForBVar.contains(var->name())) {
		current=m_typeForBVar.value(var->name());
	} else if(!m_lambdascope.contains(var->name()) && isVariableDefined(var->name())) {
		current=typeForVar(var->name());
	} else {
		current=ExpressionType(Analitza::ExpressionType::Any, m_stars++);
		current.addAssumption(var->name(), current);
		
		if(var->depth()<0 && !isVariableDefined(var->name()) && !m_calculating.contains(var->name()))
			m_deps += var->name();
	}
	
	return QString();
}

bool ExpressionTypeChecker::isVariableDefined(const QString& id) const
{
	return m_v->contains(id) || m_vars.contains(id);
}

QString ExpressionTypeChecker::accept(const Cn*)
{
	current=ExpressionType(ExpressionType::Value);
	return QString();
}

QStringList objectsToString(const QList<Object*>& objs) {
	QStringList ret;
	foreach(const Object* o, objs) {
		ret+=o->toString();
	}
	return ret;
}

ExpressionType ExpressionTypeChecker::commonType(const QList<Object*>& values)
{
	ExpressionType ret;
	
	if(values.isEmpty()) {
		ret=ExpressionType(ExpressionType::Any, m_stars++);
	} else foreach(const Object* o, values) {
		o->visit(this);
		
// 		qDebug()<< "sususu" << current << ret << "||" << current.assumptions() << ret.assumptions();
		
		QMap<int, ExpressionType> stars;
		bool b=ExpressionType::matchAssumptions(&stars, current.assumptions(), ret.assumptions());
		if(!b) {
			current=ExpressionType(ExpressionType::Error);
			break;
		}
		
// 		qDebug() << "falala" << ret << current << stars;
		
// 		printAssumptions("commonnnnXX", current);
// 		printAssumptions("commonnnnYY", ret);
		ret=ExpressionType::minimumType(current, ret);
		stars=ExpressionType::computeStars(stars, current, ret);
		ret=ret.starsToType(stars);
	}
	
	return ret;
}

QString ExpressionTypeChecker::accept(const Apply* c)
{
	QMap<QString, ExpressionType> ctx=m_typeForBVar;
	QMap<QString, ExpressionType> assumptions;
	Operator o=c->firstOperator();
	
	if(o.isBounded()) {
		Object* ul=c->ulimit();
		ExpressionType tt;
		if(ul) {
			Object* dl=c->dlimit();
			ul->visit(this);
			
			tt=current; //FIXME: should remove when done
			if(!current.isError() && current.type()!=ExpressionType::Any)
				assumptions=typeIs(dl, ExpressionType(current));
			else {
				dl->visit(this);
				tt=current;
				
				if(!current.isError())
					assumptions=typeIs(ul, ExpressionType(current));
			}
		} else if(c->domain()) {
			c->domain()->visit(this);
			
			if(current.type()==ExpressionType::Any) {
				ExpressionType anyItem(ExpressionType::Any, m_stars++);
				
				ExpressionType anyList(ExpressionType::List, anyItem);
				ExpressionType anyVector(ExpressionType::Vector, anyItem, -1);
				
				ExpressionType anyContainer(ExpressionType::Many);
				anyContainer.addAlternative(anyList);
				anyContainer.addAlternative(anyVector);
				
				anyItem.addAssumption(static_cast<Ci*>(c->domain())->name(), anyContainer);
				tt=anyItem;
			} else {
				tt=current.contained();
				tt.addAssumptions(current.assumptions());
			}
			//TODO: should control the many case
		}
		
		//FIXME: should remove when done
		foreach(const QString& s, c->bvarStrings())
			m_typeForBVar[s]=ExpressionType(tt);
	}
	
	switch(o.operatorType()) {
		case Operator::none:
		case Operator::sum:
		case Operator::product:
			(*c->firstValue())->visit(this);
			current.addAssumptions(assumptions);
			break;
		case Operator::diff:
			//TODO Check inside
			
			current=ExpressionType(ExpressionType::Lambda);
			current.addParameter(ExpressionType::Value);
			current.addParameter(ExpressionType::Value);
			break;
		case Operator::function: {
// 					qDebug() << "calling" << c->toString();
			c->m_params.first()->visit(this);
// 					qDebug() << "retrieved lambda" << c->m_params.first()->toString() << current << current.assumptions();
			ExpressionType returned = current;
			assumptions=current.assumptions();
			
			QList<ExpressionType> exps;
			Container::const_iterator it=c->firstValue()+1, itEnd=c->constEnd();
			for(; it!=itEnd; ++it) {
				(*it)->visit(this);
				
				exps += current;
			}
// 			qDebug() << "---------" << exps << returned;
			
			if(returned.type()==ExpressionType::Any || returned.isError()) {
				ExpressionType ret;
				if(returned.isError())
					ret=ExpressionType(ExpressionType::Error);
				else
					ret=ExpressionType(ExpressionType::Any, m_stars++);
// 				qDebug() << "fffffffffffffff" << m_stars;
				ret.addAssumptions(assumptions);
				
				exps += ret;
				QList<ExpressionType> t=ExpressionType::lambdaFromArgs(exps);
// 				qDebug() << "OOOOOOOOOOOOOOOOOOOOOOO" << c->toString() << ret /*<< stars*/ << ret.assumptions() << "||||" << t << exps;
				
				ExpressionType ret2(ExpressionType::Many);
				foreach(const ExpressionType& type, t) {
					ExpressionType returntype(type.parameters().last());
					returntype.addAssumptions(type.assumptions());
					if(c->m_params.first()->type()==Object::variable) {
						QString name=static_cast<Ci*>(c->m_params.first())->name();
// 						qDebug() << "inserting..." << name << type;
// 						assumptions[name] = type;
						
						if(returntype.assumptions().contains(name)) {
							ExpressionType oldt=returntype.assumptionFor(name);
							
							oldt=ExpressionType::minimumType(oldt, type);
							oldt.addAssumption(name, oldt);
							QMap<int,ExpressionType> stars;
							stars=ExpressionType::computeStars(stars, type, oldt);
							bool b=ExpressionType::matchAssumptions(&stars, oldt.assumptions(), type.assumptions());
							Q_ASSERT(b);
							
							oldt=oldt.starsToType(stars);
// 							qDebug() << "hmmm..." << oldt << stars << "::::::\n\t" << oldt.assumptions() << "\n\t" << type.assumptions();
							returntype=oldt.parameters().last();
							
							returntype.addAssumption(name, oldt);
						} else
							returntype.addAssumption(name, type);
						
					}
					returntype.addAssumptions(type.assumptions());
					
					ret2.addAlternative(returntype);
				}
				
				current=ret2;
			} else {
				ExpressionType ret(ExpressionType::Many), signature(returned);
				
				QList<ExpressionType> alts=signature.type()==ExpressionType::Many ? signature.alternatives() : QList<ExpressionType>() << signature;
				QList<ExpressionType> args=ExpressionType::lambdaFromArgs(exps);
				foreach(const ExpressionType& opt, alts) {
					if(opt.type()!=ExpressionType::Lambda) {
// 								addError(i18n("We can only call functions."));
						continue;
					}
					
					foreach(const ExpressionType& f, args) {
						QList<ExpressionType> altargs=f.parameters();
						
						if(opt.parameters().size()!=altargs.size()+1) {
							addError(i18np("Invalid parameter count for '%2'. Should have 1 parameter.",
														"Invalid parameter count for '%2'. Should have %1 parameters.",
														opt.parameters().size(), c->toString()));
							break;
						}
						
						bool valid=true;
						QMap<QString, ExpressionType> assumptions;
						QMap<int, ExpressionType> starToType, starToParam;
						for(int i=0; valid && i<opt.parameters().size()-1; i++) {
							starToType=ExpressionType::computeStars(starToType, opt.parameters()[i], altargs[i]);
							
							ExpressionType t=opt.parameters()[i].starsToType(starToType);
							starToParam=ExpressionType::computeStars(starToParam, altargs[i], t);
							valid=!t.isError() && inferType(altargs[i].starsToType(starToParam), t, &assumptions);
							
// 							qDebug() << "pepeluis" << c->toString() << c->m_params[i+1]->toString() << altargs[i] << t << valid;
						}
						
						if(valid) {
							ExpressionType t=opt.parameters().last();
							t.addAssumptions(assumptions);
							
							ret.addAlternative(t.starsToType(starToType));
						}
					}
				}
					
				if(ret.alternatives().isEmpty()) {
// 					qDebug() << "peee" << signature;
					
					current=ExpressionType(ExpressionType::Error);
					addError(i18n("Could not call '%1'", c->toString()));
				} else
					current=ret;
			}
		} break;
		default:
			current=solve(&o, c->values());
			if(current.type()==ExpressionType::Error)
				addError(i18n("Could not solve '%1'", c->toString()));
			break;
	}
	m_typeForBVar=ctx;
	
	if(current.type()==ExpressionType::Many && current.alternatives().size()==1) {
		current=current.alternatives().first();
	}
	
	return QString();
}

QString ExpressionTypeChecker::accept(const CustomObject*)
{
	Q_ASSERT(false && "we shouldn't have to construct any custom object");
	return QString();
}

//1. Check if parameters are applied correctly
//2. Return the operator result type
QString ExpressionTypeChecker::accept(const Container* c)
{
// 	qDebug() << "XIUXIU" << c->toString();
	switch(c->containerType()) {
		case Container::piecewise: {
			ExpressionType type=commonType(c->m_params);
			
			if(type.isError())
				addError(i18n("Could not determine the type for piecewise"));
			else {
				QMap<QString, ExpressionType> assumptions=typeIs(c->constBegin(), c->constEnd(), type);
				
// 				QMap<int, ExpressionType> stars;
// 				bool b=ExpressionType::matchAssumptions(&stars, assumptions, type.assumptions());
// 				Q_ASSERT(b);
// 				type=type.starsToType(stars);
				
				type.addAssumptions(assumptions); //branches
			}
			
			current=type;
		}	break;
		case Container::piece: {
			QMap<QString, ExpressionType> assumptions=typeIs(c->m_params.last(), ExpressionType(ExpressionType::Value)); //condition check
			c->m_params.first()->visit(this); //we return the body
			
			QMap<int, ExpressionType> stars;
			bool b=ExpressionType::matchAssumptions(&stars, assumptions, current.assumptions());
			
			Q_ASSERT(b);
			current=current.starsToType(stars);
			current.addAssumptions(assumptions);
		}	break;
		case Container::declare:{
			Q_ASSERT(c->m_params.first()->type()==Object::variable);
			Ci* var = static_cast<Ci*>(c->m_params.first());
			
			m_calculating.append(var->name());
			c->m_params.last()->visit(this);
			m_calculating.removeLast();
			
			QList<ExpressionType> opts=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
			
			for(QList< ExpressionType >::iterator it=opts.begin(), itEnd=opts.end(); it!=itEnd; ++it) {
				QMap< QString, ExpressionType >::const_iterator itFound=it->assumptions().constFind(var->name());
				if(itFound!=it->assumptions().constEnd()) {
					QMap<int, ExpressionType> stars;
					stars=ExpressionType::computeStars(stars, *itFound, *it);
// 					qDebug() << "fiiiiiiiii" << stars << *it << *itFound;
					*it=it->starsToType(stars);
				}
// 				printAssumptions("fefefe", current);
			}
			current=ExpressionType(ExpressionType::Many, opts);
		}	break;
		case Container::lambda: {
			QSet<QString> aux=m_lambdascope;
			m_lambdascope+=c->bvarStrings().toSet();
			c->m_params.last()->visit(this);
			m_lambdascope=aux;
			QMap<QString, ExpressionType> assumptions=current.assumptions();
			
			QList<ExpressionType> alts=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
			ExpressionType res=ExpressionType(ExpressionType::Many);
			foreach(const ExpressionType& alt, alts) {
				QList<ExpressionType> args;
				
				foreach(const QString& bvar, c->bvarStrings()) {
					ExpressionType toadd;
					if(alt.assumptions().contains(bvar))
						toadd=alt.assumptionFor(bvar);
					else
						toadd=ExpressionType(ExpressionType::Any, m_stars++);
					
					args += toadd;
				}
				args += alt;
				args=ExpressionType::lambdaFromArgs(args);
				
				res.addAlternative(ExpressionType(ExpressionType::Many, args));
			}
			current=res;
		}	break;
		case Container::otherwise:
		case Container::math:
		case Container::none:
		case Container::downlimit:
		case Container::uplimit:
		case Container::bvar:
		case Container::domainofapplication:
			for(Container::const_iterator it=c->constBegin(); it!=c->constEnd(); ++it)
				(*it)->visit(this);
			break;
	}
	
	if(current.type()==ExpressionType::Many) {
		if(current.alternatives().isEmpty())
			current=ExpressionType(ExpressionType::Error);
		else if(current.alternatives().count()==1)
			current=current.alternatives().first();
	}
	
	return QString();
}

QString ExpressionTypeChecker::accept(const Vector* v)
{
	ExpressionType cont=commonType(v->values());
	
	if(cont.type()==ExpressionType::Many) {
		ExpressionType toret(ExpressionType::Many);
		foreach(const ExpressionType& contalt, cont.alternatives()) {
			QMap< QString, ExpressionType > assumptions;
			assumptions=typeIs(v->constBegin(), v->constEnd(), contalt);
			ExpressionType cc(ExpressionType::Vector, contalt, v->size());
			
			bool b=ExpressionType::assumptionsMerge(cc.assumptions(), assumptions);
			if(b)
				toret.addAlternative(cc);
		}
		
		current=toret;
	} else if(!cont.isError()) {
		QMap< QString, ExpressionType > assumptions=typeIs(v->constBegin(), v->constEnd(), cont);
		current=ExpressionType(ExpressionType::Vector, cont, v->size());
		current.addAssumptions(assumptions);
	} else
		current=ExpressionType(ExpressionType::Error);
	
	return QString();
}

QString ExpressionTypeChecker::accept(const List* l)
{
	ExpressionType cont;
	if(l->size()==0)
		cont=ExpressionType::Error;
	else
		cont=commonType(l->values());
// 	qDebug() << "-------" << cont;
	
	if(cont.type()==ExpressionType::Many) {
		ExpressionType toret(ExpressionType::Many);
		
		foreach(const ExpressionType& contalt, cont.alternatives()) {
			QMap< QString, ExpressionType > assumptions;
			assumptions=typeIs(l->constBegin(), l->constEnd(), contalt);
			ExpressionType cc(ExpressionType::List, contalt);
			cc.addAssumptions(assumptions);
			toret.addAlternative(cc);
		}
		
		current=toret;
	} else if(!cont.isError()) {
		QMap< QString, ExpressionType > assumptions=typeIs(l->constBegin(), l->constEnd(), cont);
		current=ExpressionType(ExpressionType::List, cont);
		bool b = ExpressionType::assumptionsMerge(current.assumptions(), assumptions);
// 		qDebug() << "mergeeeeeeh" << current << b << current.assumptions() << assumptions;
		if(!b)
			current=ExpressionType(ExpressionType::Error);
	} else
		current=ExpressionType(ExpressionType::Error);
	
// 	printAssumptions("dududu", current);
	return QString();
}

template <class T>
QMap<QString, ExpressionType> ExpressionTypeChecker::typeIs(T it, const T& itEnd, const ExpressionType& type)
{
	QMap<QString, ExpressionType> ret;
	for(; it!=itEnd; ++it) {
		(*it)->visit(this);
		bool valid=inferType(current, type, &ret);
		
		if(!valid)
			addError(i18n("Unexpected type for '%1'", (*it)->toString()));
	}
	
	return ret;
}

QMap<QString, ExpressionType> ExpressionTypeChecker::typeIs(const Object* o, const ExpressionType& type)
{
	o->visit(this);
	
	QMap<QString, ExpressionType> assumptions;
	bool corr=inferType(current, type, &assumptions);
	corr &= ExpressionType::assumptionsMerge(assumptions, m_typeForBVar);
	
	if(!corr)
		addError(i18n("Cannot convert '%1' to '%2'", o->toString(), type.toString()));
	
	return assumptions;
}

ExpressionType ExpressionTypeChecker::typeForVar(const QString& var)
{
	if(m_calculating.contains(var))
		return ExpressionType(ExpressionType::Error);
	else if(!m_vars.contains(var)) {
// 		qDebug() << "checking..." << var;
		Q_ASSERT(m_v->contains(var));
		m_calculating += var;
		m_v->value(var)->visit(this);
		m_calculating.removeLast();
		current.clearAssumptions();
// 		current.simplifyStars();
		m_vars[var]=current;
// 		qDebug() << "checked type" << var << "=" << current;
	}
	
	ExpressionType ret=m_vars.value(var);
	
// 	qDebug() << "P/P/P/P/P/" << ret;
	m_stars=ret.increaseStars(m_stars);
// 	qDebug() << "P/P/P/P/P/" << ret;
	return ret;
}

QStringList ExpressionTypeChecker::errors() const
{
	QStringList ret;
	foreach(const QStringList& errs, m_err)
		ret << errs;
	return ret;
}

void ExpressionTypeChecker::addError(const QString& err)
{
	if(m_err.isEmpty())
		m_err += QStringList();
	
	m_err.last().append(err);
}

}
