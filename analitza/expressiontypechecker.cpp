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

bool merge(QMap<QString, Analitza::ExpressionType>& data, const QMap<QString, Analitza::ExpressionType>& newmap);

namespace Analitza
{
ExpressionTypeChecker::ExpressionTypeChecker(Variables* v)
	: m_v(v)
{}

QDebug operator<<(QDebug dbg, const ExpressionType &c)
{
	dbg.nospace() << "("<< qPrintable(c.toString());
// 	if(!c.assumptions().isEmpty())
// 		dbg.nospace() <<", " << c.assumptions();
	dbg.nospace() << /*":"<< c.type() <<*/")";
	
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const TypePair &c)
{
	dbg.nospace() << "["<< c.param << ":" << c.returnValue <<"]";
	
	return dbg.nospace();
}

QDebug operator<<(QDebug dbg, const TypeTriplet &c)
{
	dbg.nospace() << "["<< c.param1 << ", " << c.param2 << ":" << c.returnValue <<"]";
	
	return dbg.nospace();
}

QMap<int, ExpressionType> ExpressionTypeChecker::computeStars(const QMap<int, ExpressionType>& initial, const ExpressionType& candidate, const ExpressionType& type)
{
	QMap<int, ExpressionType> ret(initial);
	
	switch(candidate.type()) {
		case ExpressionType::Any: {
			int stars=candidate.anyValue();
				
			if(ret.contains(stars)) {
				ExpressionType t=ret[stars];
				if(t==candidate)
					break;
				
				ret=computeStars(initial, t, type);
			} else {
				ExpressionType cosa(type);
				cosa.clearAssumptions();
				ret[stars]=cosa.starsToType(initial);
				
				QMap<int, ExpressionType>::iterator it=ret.begin(), itEnd=ret.end();
				for(; it!=itEnd; ++it)
					*it=it->starsToType(ret);
				
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
					ExpressionType cosa(type);
					cosa.clearAssumptions();
					ret[candidate.size()] = cosa;
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
		case ExpressionType::Value:
		case ExpressionType::Many:
		case ExpressionType::Error:
		case ExpressionType::Undefined:
// 			Q_ASSERT(false && "bffff");
			break;
	}
// 	qDebug() << ";;;;;;;" << candidate << type << initial;
	
	return ret;
}

ExpressionType ExpressionTypeChecker::check(const Expression& _exp)
{
	m_stars=1;
	Expression exp(_exp);
	current=ExpressionType(ExpressionType::Error);
	
// 	Object::ScopeInformation scope=AnalitzaUtils::variablesScope(m_v);
// 	bool deps=exp.tree()->decorate(scope);
	
	exp.tree()->visit(this);
	
// 	if(current.isError())
// 		addError(i18n("Could not figure out the expression type"));
	
	return current;
}

QString ExpressionTypeChecker::accept(const Operator*) { Q_ASSERT(false && "should not get here"); return QString(); }

bool ExpressionTypeChecker::inferType(const Object* exp, const ExpressionType& targetType, QMap<QString, ExpressionType>* assumptions)
{
	Q_ASSERT(!targetType.isError() && assumptions);
	m_err.append(QStringList());
	
// 	qDebug() << "//////" << exp->toString() << targetType;
	bool ret=false;
	switch(exp->type()) {
		case Object::variable: {
			const Ci* var=static_cast<const Ci*>(exp);
			
			if(m_v->contains(var->name())) {
				ExpressionType t=typeForVar(var->name());
// 				qDebug() << "vvvvvvVVVvvvVVvvvVvvv" << var->name() << t << t.canReduceTo(targetType);
				
				ret=t==targetType;
				if(!ret && t.canReduceTo(targetType)) {
					assumptions->insert(var->name(), targetType);
					ret=true;
				}
				
			} else if(assumptions->contains(var->name())) {
// 				qDebug() << "vvvvvvVVVvvvVVvvvVvvv" << var->name() << assumptions->contains(var->name());
				ret=assumptions->value(var->name())==targetType;
				if(!ret && assumptions->value(var->name()).canReduceTo(targetType)) {
					assumptions->insert(var->name(), targetType);
					ret=true;
				}
				
			} else {
				assumptions->insert(var->name(), targetType);
				ret=true;
			}
// 			qDebug()<< "OAOAOAOA" << ret << exp->toString() << targetType;
			
		}	break;
		case Object::value:
		case Object::container:
// 			qDebug() << "PEEEEEEEEEE" << exp->toString() << targetType;
			exp->visit(this);
			ret=current.canReduceTo(targetType) && merge(*assumptions, current.assumptions());
			
// 			qDebug() << "PEEEEEEEEEE" << exp->toString() << current << targetType << ret << *assumptions;
			break;
		case Object::vector:
			ret=targetType.type()==ExpressionType::Vector;
			if(ret) {
				const Vector* vec=static_cast<const Vector*>(exp);
				ret=targetType.size()<0 || targetType.size()==vec->size();
				
				Vector::const_iterator it=vec->constBegin(), itEnd=vec->constEnd();
				for(; ret && it!=itEnd; ++it) {
					ret=inferType(*it, targetType.contained(), assumptions);
				}
			}
			break;
		case Object::list:
			ret=targetType.type()==ExpressionType::List;
			if(ret) {
				const List* vec=static_cast<const List*>(exp);
				List::const_iterator it=vec->constBegin(), itEnd=vec->constEnd();
				
				for(; ret && it!=itEnd; ++it) {
					ret=inferType(*it, targetType.contained(), assumptions);
				}
			}
			
			break;
		case Object::oper:
		case Object::none:
			Q_ASSERT(false && "wtf!");
			break;
	}
	m_err.removeLast();
	return ret;
}

QList<TypeTriplet> ExpressionTypeChecker::computeTriplets(const QList<TypeTriplet>& options,
									const ExpressionType& first, const ExpressionType& second)
{
	QList<TypeTriplet> results;
	foreach(const TypeTriplet& opt, options) {
		if(first.canReduceTo(opt.param1) && second.canReduceTo(opt.param2)) {
			results += opt;
		}
	}
	
// 	qDebug() << ";;;;;;;" << results;
	return results;
}

QList<TypePair> ExpressionTypeChecker::computePairs(const QList<TypePair>& options, const ExpressionType& param)
{
	QList<TypePair> ret;
	
	if(param.type()==ExpressionType::Any) {
		foreach(const TypePair& opt, options) {
			QMap<int, ExpressionType> stars;
			
			//We know the parameter is any, we don't have to infer
			stars=computeStars(stars, param, opt.param);
			
			TypePair toadd=opt;
			toadd.returnValue.addAssumptions(param.assumptions());
			toadd.returnValue=toadd.returnValue.starsToType(stars);
			ret += toadd;
		}
	} else 
		foreach(const TypePair& opt, options) {
// 			qDebug() << "TURURURU" << opt.param << param;
			if(opt.param==param || opt.param.canReduceTo(param)) //Infer on param!=param but canReduce?
				ret += opt;
		}
	
// 	qDebug() << "_______" << param << exp->toString() << ret;
	return ret;
}

bool ExpressionTypeChecker::matchAssumptions(QMap< int, ExpressionType >* stars,
												const QMap< QString, ExpressionType >& assum1,
												const QMap< QString, ExpressionType >& assum2)
{
	bool ret=true;
	QMap<QString, ExpressionType>::const_iterator it=assum1.constBegin(), itEnd=assum1.constEnd();
	QMap<QString, ExpressionType>::const_iterator itFind, itFindEnd=assum2.constEnd();
	
	for(; ret && it!=itEnd; ++it) {
		itFind=assum2.find(it.key());
		
		if(itFind!=itFindEnd) {
// 			qDebug() << it.key() << *it << *itFind << (itFind.value()!=it.value());
			if(itFind.value()!=it.value()) {
				if(itFind.value().canReduceTo(it.value()))
					*stars=computeStars(*stars, itFind.value(), it.value());
				else if(it.value().canReduceTo(itFind.value()))
					*stars=computeStars(*stars, it.value(), itFind.value());
				else
					ret=false;
// 				qDebug() << "seh" << *stars << ret;
			}
		}
	}
	
	return ret;
}

ExpressionType ExpressionTypeChecker::solve(const Operator* o, const QList< Object* >& parameters)
{
	Q_ASSERT(o->operatorType()!=Operator::function);
	
	ExpressionType ret(ExpressionType::Many);
	if(parameters.size()==1) {
		parameters.first()->visit(this);
		QList<ExpressionType> types=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
		
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
		
		QList<Object*>::const_iterator it=parameters.constBegin()+1, itEnd=parameters.constEnd();
		for(; it!=itEnd; ++it)
		{
			(*it)->visit(this);
			ExpressionType secondType=current;
			
			QList<ExpressionType> firstTypes= firstType.type()==ExpressionType::Many ?  firstType.alternatives() : QList<ExpressionType>() << firstType;
			QList<ExpressionType> secndTypes=secondType.type()==ExpressionType::Many ? secondType.alternatives() : QList<ExpressionType>() << secondType;
			
			static int ind=3;
			qDebug() << "+" << qPrintable(QString(ind++, '-')) << o->toString() << firstType << secondType;
			foreach(const ExpressionType& _first, firstTypes) {
// 				qDebug() << "tralara" << _first;
				foreach(const ExpressionType& _second, secndTypes) {
					QList<TypeTriplet> res=Operations::infer(o->operatorType());//computeTriplets(Operations::infer(o->operatorType()), first, second);
// 					qDebug() << "+++++++" << res.size();
					
					foreach(const TypeTriplet& opt, res) {
						Q_ASSERT(!opt.returnValue.isError());
						QMap<int, ExpressionType> starToType;
						
						bool valid=matchAssumptions(&starToType, _first.assumptions(), _second.assumptions()); //match assumptions
						
						starToType=computeStars(starToType, opt.param1, _first);
						starToType=computeStars(starToType, opt.param2, _second);
						starToType=computeStars(starToType, _first,  opt.param1);
						starToType=computeStars(starToType, _second, opt.param2);
						
						ExpressionType first =_first .starsToType(starToType);
						ExpressionType second=_second.starsToType(starToType);
						
// 						qDebug() << "XXXXXX" << first.assumptions() << _first.assumptions() << starToType;
						
						valid&=matchAssumptions(&starToType, first.assumptions(), second.assumptions()); //match assumptions
						if(!valid)
							continue;
						
						QMap<QString, ExpressionType> assumptions=first.starsToType(starToType).assumptions();
						valid&=merge(assumptions, second.starsToType(starToType).assumptions());
						
// 						qDebug() << "fifuuuuuuu" << first << (*(it-1))->toString() << 
// 													second << (*it)->toString() << assumptions;
						
						valid &= first.canReduceTo(opt.param1.starsToType(starToType));
						valid &= second.canReduceTo(opt.param2.starsToType(starToType));
						
// 						qDebug() << "POPOPO" << /*(*(it-1))->toString() << (*(it))->toString() <<*/ valid << opt << assumptions << starToType;
						if(valid) {
							ExpressionType toadd=opt.returnValue.starsToType(starToType);
							toadd.addAssumptions(assumptions);
							
// 							qDebug() << "&&&&&&" << toadd << toadd.assumptions() << starToType;
							ret.addAlternative(toadd);
						}
					}
				}
			}
			qDebug() << "\\" << qPrintable(QString(--ind, '-')) << o->toString() << ret;
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
	if(m_v->contains(var->name())) {
		current=typeForVar(var->name());
	} else if(m_typeForBVar.contains(var->name())) {
		current=m_typeForBVar.value(var->name());
	} else {
		current=ExpressionType(Analitza::ExpressionType::Any, m_stars++);
		current.addAssumption(var->name(), current);
		qDebug() << "created" << var->name() << current;
	}
	
	return QString();
}

QString ExpressionTypeChecker::accept(const Cn*)
{
	current=ExpressionType(ExpressionType::Value);
	return QString();
}

//1. Check if parameters are applied correctly
//2. Return the operator result type
QString ExpressionTypeChecker::accept(const Container* c)
{
	qDebug() << "XIUXIU" << c->toString();
	QMap<QString, ExpressionType> assumptions;
	switch(c->containerType()) {
		case Container::apply: {
			QMap<QString, ExpressionType> ctx=m_typeForBVar;
			Operator o=c->firstOperator();
			
			if(o.isBounded()) {
				Object* ul=c->ulimit();
				if(ul) {
					ul->visit(this);
					
					m_typeForBVar[c->bvarStrings().first()]=current; //FIXME: should remove when done
					assumptions=typeIs(c->dlimit(), ExpressionType(current));
				}
			}
			
			switch(o.operatorType()) {
				case Operator::none:
				case Operator::sum:
				case Operator::product:
					(*c->firstValue())->visit(this);
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
					
					if(current.isError()) { //FIXME:if its error it means that we are in a recursive case, undefined?
						Container::const_iterator it=c->m_params.constBegin()+1, itEnd=c->m_params.constEnd();
						
						ExpressionType ret(ExpressionType::Undefined);
						for(; it!=itEnd; ++it) {
							(*it)->visit(this);
							ret.addAssumptions(current.assumptions());
						}
						
						current=ret;
					} else if(current.type()==ExpressionType::Any) {
						ExpressionType func(ExpressionType::Lambda);
						ExpressionType ret=current;
						
						QMap<int, ExpressionType> starToType;
						Container::const_iterator it=c->firstValue()+1, itEnd=c->constEnd();
						for(; it!=itEnd; ++it) {
							(*it)->visit(this);
							func.addParameter(current);
							ret.addAssumptions(current.assumptions());
						}
						func.addParameter(ret);
						
						qDebug() << "xXXXXX" << c->m_params.first()->toString();
						if(c->m_params.first()->type()==Object::variable) {
							QString name=static_cast<Ci*>(c->m_params.first())->name();
							assumptions.insert(name, func);
							
							ret.addAssumptions(assumptions);
						}
						
						current=ret;
					} else {
						ExpressionType ret(ExpressionType::Many), signature(current);
						
						QList<ExpressionType> alts=signature.type()==ExpressionType::Many ? signature.alternatives() : QList<ExpressionType>() << signature;
// 						qDebug() << "SSSSSSSSSSSSSSSSSSSSS" << alts;
						
						foreach(const ExpressionType& opt, alts) {
// 							qDebug() << "ooooopt" << opt;
							if(opt.type()!=ExpressionType::Lambda) {
// 								addError(i18n("We can only call functions."));
								continue;
							}
							
							if(opt.parameters().size()!=c->m_params.size()) {
								addError(i18n("Invalid parameter count."));
								continue;
							}
							
							bool valid=true;
							QMap<QString, ExpressionType> assumptions;
							QMap<int, ExpressionType> starToType;
							
							for(int i=0; valid && i<opt.parameters().size()-1; i++) {
								c->m_params[i+1]->visit(this);
								
								if(!opt.parameters()[i].canReduceTo(current)) {
									continue;
								}
								
								starToType=computeStars(starToType, opt.parameters()[i], current);
								
								valid&=merge(assumptions, opt.parameters()[i].assumptions());
							}
							
							QMap<QString, ExpressionType>::iterator it=assumptions.begin(), itEnd=assumptions.end();
							for(; valid && it!=itEnd; ++it) {
								*it=it->starsToType(starToType);
							}
							
							for(int i=0; valid && i<opt.parameters().size()-1; i++) {
								valid=inferType(c->m_params[i+1], opt.parameters()[i].starsToType(starToType), &assumptions);
// 								qDebug() << "pepeluis" << c->m_params[i+1]->toString() << opt.parameters()[i].starsToType(starToType) << valid;
							}
							
							if(valid) {
								ExpressionType t=opt.parameters().last().starsToType(starToType);
								
								t.addAssumptions(assumptions);
								ret.addAlternative(t);
							}
						}
							
						if(ret.alternatives().isEmpty()) {
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
		}	break;
		case Container::piecewise: {
			bool found=false;
			ExpressionType type;
			foreach(const Object* o, c->m_params) {
				o->visit(this);
				if(!current.isError()) {
					found=true;
					type=current;
					break;
				}
			}
			
			if(!found)
				addError(i18n("Could not determine the type for piecewise"));
			else
				type.addAssumptions(typeIs(c->m_params.constBegin(), c->m_params.constEnd(), type)); //branches
			
			current=type;
		}	break;
		case Container::piece: {
			assumptions=typeIs(c->m_params.last(), ExpressionType(ExpressionType::Value)); //condition check
			c->m_params.first()->visit(this); //we return the body
			current.addAssumptions(assumptions);
		}	break;
		case Container::declare:
			c->m_params.last()->visit(this);
			break;
		case Container::lambda: {
			c->m_params.last()->visit(this);
			
			QList<ExpressionType> alts=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
			
			ExpressionType res=ExpressionType(ExpressionType::Many);
			foreach(const ExpressionType& alt, alts) {
				if(alt.isUndefined()) {
					addError("Could not make up the return value of the lambda definition");
				} else {
					ExpressionType option(ExpressionType::Lambda);
					foreach(const QString& bvar, c->bvarStrings()) {
						if(alt.assumptions().contains(bvar))
							option.addParameter(alt.assumptionFor(bvar));
						else if(m_typeForBVar.contains(bvar))
							option.addParameter(m_typeForBVar.value(bvar));
						else
							option.addParameter(ExpressionType(ExpressionType::Any, m_stars++));
					}
					option.addParameter(alt); //Return value
					
					res.addAlternative(option);
				}
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
			(*c->firstValue())->visit(this);
			break;
	}
	
	if(current.type()==ExpressionType::Many) {
		if(current.alternatives().isEmpty())
			current=ExpressionType(ExpressionType::Error);
		else if(current.alternatives().count()==1)
			current=current.alternatives().first();
	}
	current.addAssumptions(assumptions);
	
	return QString();
}

QString ExpressionTypeChecker::accept(const Vector* v)
{
	v->at(0)->visit(this);
// 	qDebug() << "ccccc" << current << current.assumptions();
	
	ExpressionType t(ExpressionType::Vector, ExpressionType(current), v->size());
	QMap< QString, ExpressionType > assumptions=typeIs(v->constBegin(), v->constEnd(), t.contained());
	current=t;
	current.addAssumptions(assumptions);
	
// 	qDebug() << "pepepepe" << t << v->toString();
	return QString();
}

QString ExpressionTypeChecker::accept(const List* l)
{
	ExpressionType tc(ExpressionType::Error);
	if(l->size()>0) { //FIXME: loop until we find a proper one
		l->at(0)->visit(this);
		tc=current;
	}
	
	ExpressionType t(ExpressionType::List, tc);
	QMap<QString, ExpressionType> assumptions=typeIs(l->constBegin(), l->constEnd(), tc);
	current=t;
	current.addAssumptions(assumptions);
	
	return QString();
}

template <class T>
QMap<QString, ExpressionType> ExpressionTypeChecker::typeIs(T it, const T& itEnd, const ExpressionType& type)
{
	ExpressionType cosa(type); //FIXME
	
	QMap<QString, ExpressionType> ret;
	for(; it!=itEnd; ++it) {
		/*valid=*/merge(ret, typeIs(*it, ExpressionType(cosa))); //FIXME: merge assumptions, if can't merge, fail
	}
	
	return ret;
}

QMap<QString, ExpressionType> ExpressionTypeChecker::typeIs(const Object* o, const ExpressionType& type)
{
	QMap<QString, ExpressionType> assumptions(m_typeForBVar);
	bool corr=inferType(o, type, &assumptions);
	
	if(!corr)
		addError(i18n("Cannot convert '%1' to '%2'", o->toString(), type.toString()));
	
	return assumptions;
}

ExpressionType ExpressionTypeChecker::typeForVar(const QString& var)
{
	if(m_calculating.contains(var))
		return ExpressionType(ExpressionType::Error);
	else if(!m_vars.contains(var)) {
		qDebug() << "checking..." << var;
		Q_ASSERT(m_v->contains(var));
		m_calculating += var;
		m_v->value(var)->visit(this);
		m_calculating.removeOne(var);
		m_vars[var]=current;
		qDebug() << "checked type" << var << "=" << current;
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