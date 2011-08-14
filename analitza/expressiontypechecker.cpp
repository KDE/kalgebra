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
#include <localize.h>
#include "apply.h"
#include "value.h"

QDebug operator<<(QDebug dbg, const Analitza::ExpressionType &c)
{
	dbg.nospace() << "("<< qPrintable(c.toString());
// 	if(!c.assumptions().isEmpty())
// 		dbg.nospace() <<", " << c.assumptions();
	dbg.nospace() << /*":"<< c.type() <<*/")";
	
	return dbg.space();
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
	Q_ASSERT(found.type()!=ExpressionType::Many);
	
	QMap<int, ExpressionType> stars;
	stars=ExpressionType::computeStars(stars, found, targetType);
	bool ret=found.canReduceTo(targetType);
	ret&=ExpressionType::matchAssumptions(&stars, found.assumptions(), targetType.assumptions());
	ret&=ExpressionType::assumptionsMerge(*assumptions, found.assumptions());
	
	for(QMap<QString, ExpressionType>::iterator it=assumptions->begin(), itEnd=assumptions->end(); it!=itEnd; ++it) {
		*it=it->starsToType(stars);
	}
// 	qDebug() << "fuuuuu" << stars << *assumptions << ret;
	
	return ret;
}

QList<ExpressionType> ExpressionTypeChecker::computePairs(const QList<ExpressionType>& options, const ExpressionType& param)
{
	QList<ExpressionType> ret;
	
	if(param.type()==ExpressionType::Any) {
		int basestars=m_stars;
		foreach(const ExpressionType& opt, options) {
			ExpressionType toadd=opt;
			m_stars=qMax<int>(m_stars, toadd.increaseStars(basestars));
			QMap<int, ExpressionType> stars;
			
			//We know the parameter is any, we don't have to infer
			stars=ExpressionType::computeStars(stars, param, toadd.parameters().first());
			
			toadd.parameters().last().addAssumptions(param.assumptions());
			toadd.parameters().last()=toadd.parameters().last().starsToType(stars);
			ret += toadd;
		}
	} else 
		foreach(const ExpressionType& opt, options) {
// 			qDebug() << "TURURURU" << opt.param << param;
			if(opt.parameters().first().canReduceTo(param)) //Infer on param!=param but canReduce?
				ret += opt;
		}
	
// 	qDebug() << "_______" << param << exp->toString() << ret;
	return ret;
}

ExpressionType ExpressionTypeChecker::solve(const Operator* o, const QList< Object* >& parameters)
{
	Q_ASSERT(o->operatorType()!=Operator::function);
	
	QList<ExpressionType> paramtypes;
	for(QList<Object*>::const_iterator it=parameters.constBegin(), itEnd=parameters.constEnd(); it!=itEnd; ++it) {
		(*it)->visit(this);
		paramtypes += current;
	}
	
	ExpressionType ret(ExpressionType::Many);
	if(parameters.size()==1) {
		QList<ExpressionType> types=paramtypes.first().type()==ExpressionType::Many ? paramtypes.first().alternatives() : QList<ExpressionType>() << paramtypes.first();
// 		const QMap<QString, ExpressionType> assumptions=current.assumptions();
		
		foreach(const ExpressionType& t, types) {
// 			qDebug() << "leee" << t.assumptions();
			QList<ExpressionType> thing=computePairs(Operations::inferUnary(o->operatorType()), t);
			foreach(const ExpressionType& opt, thing) {
				ExpressionType tt(opt.parameters().last());
				tt.addAssumptions(t.assumptions());
				ret.addAlternative(tt);
			}
		}
// 		qDebug() << "bam" << ret ;
	} else {
		Q_ASSERT(parameters.size()>=2);
// 		QMap<int, ExpressionType> stars;
		
		ExpressionType firstType=paramtypes.first();
		QList<ExpressionType> firstTypes= firstType.type()==ExpressionType::Many ?  firstType.alternatives() : QList<ExpressionType>() << firstType;
		
		const QList<ExpressionType> res=Operations::infer(o->operatorType());
		
		for(QList<ExpressionType>::const_iterator it=paramtypes.constBegin()+1, itEnd=paramtypes.constEnd(); it!=itEnd; ++it)
		{
			ExpressionType secondType=*it;
			
			QList<ExpressionType> secndTypes=secondType.type()==ExpressionType::Many ? secondType.alternatives() : QList<ExpressionType>() << secondType;
			int starsbase=m_stars;
// 			static int ind=3;
// 			qDebug() << qPrintable("+" +QString(ind++, '-')) << o->toString() << firstType << secondType;
			
			foreach(const ExpressionType& _first, firstTypes) {
				foreach(const ExpressionType& _second, secndTypes) {
					QMap<int, ExpressionType> _starToType;
					bool matches=ExpressionType::matchAssumptions(&_starToType, _first.assumptions(), _second.assumptions());
					//TODO: maybe error here
					
					if(!matches) {
// 						qDebug() << "peee" << ExpressionType::wrongAssumptions(_first.assumptions(), _second.assumptions());
						continue;
					}
					foreach(const ExpressionType& _opt, res) {
						ExpressionType opt(_opt);
						m_stars=qMax<int>(m_stars, opt.increaseStars(starsbase));
						
						Q_ASSERT(!opt.parameters().last().isError());
						QMap<int, ExpressionType> starToType, starToParam;
						
						ExpressionType first =_first .starsToType(_starToType);
						ExpressionType second=_second.starsToType(_starToType);
// 						qDebug() << "9999999" << _second.assumptions() << second.assumptions() << starToType;
						
						starToType=ExpressionType::computeStars(starToType, first,  opt.parameters()[0]);
						starToType=ExpressionType::computeStars(starToType, second, opt.parameters()[1]);
						
						first =first .starsToType(starToType);
						second=second.starsToType(starToType);
						
						starToParam=ExpressionType::computeStars(starToParam, opt.parameters()[0], first);
						starToParam=ExpressionType::computeStars(starToParam, opt.parameters()[1], second);
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
						
						valid &= first .canReduceTo(opt.parameters()[0].starsToType(starToParam));
						valid &= second.canReduceTo(opt.parameters()[1].starsToType(starToParam));
						
// 						qDebug() << "POPOPO" << (*(it-1))->toString() << (*(it))->toString() << valid << first << second << starToParam;
						if(valid) {
							ExpressionType toadd=opt.parameters().last();
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

QString ExpressionTypeChecker::accept(const Cn* c)
{
	ExpressionType::Type type;
	
	switch(c->format()) {
		case Cn::Boolean:
			type=ExpressionType::Bool;
			break;
		case Cn::Char:
			type=ExpressionType::Char;
			break;
		default:
			type=ExpressionType::Value;
			break;
	}
	
	current=ExpressionType(type);
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
		
// 		qDebug() << "falala" << ret << current << stars;
		QList<ExpressionType> retalts;
		QList<ExpressionType> alts1=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
		QList<ExpressionType> alts2=ret.type()==ExpressionType::Many ? ret.alternatives() : QList<ExpressionType>() << ret;
// #error foreach + canReduce + computestars to tell that type of:foldr1==f==elems
		
		foreach(const ExpressionType& t1, alts1) {
			QMap<int, ExpressionType> _stars;
			QMap<QString, ExpressionType> assumptions(ret.assumptions());
			bool b=ExpressionType::matchAssumptions(&_stars, current.assumptions(), assumptions);
			b &= ExpressionType::assumptionsMerge(assumptions, current.assumptions());
			if(!b) {
				continue;
			}
			
			foreach(const ExpressionType& t2, alts2) {
				QMap<int, ExpressionType> stars(_stars);
				
// 				printAssumptions("commonnnnXX", t1);
// 				printAssumptions("commonnnnYY", t2);
				ExpressionType rr=ExpressionType::minimumType(t1, t2);
				if(!rr.isError() && ExpressionType::matchAssumptions(&stars, t2.assumptions(), t1.assumptions())) {
					stars=ExpressionType::computeStars(stars, t2, t1);
					bool b=ExpressionType::assumptionsMerge(rr.assumptions(), assumptions);
// 					Q_ASSERT(b);
// 					qDebug() << "*************" << o->toString() << rr << b << "||||||" << stars;
// 					printAssumptions("commonnnnWW", rr);
					if(b)
						retalts += rr.starsToType(stars);
// 					printAssumptions("commonnnnZZ", retalts.last());
				}
			}
		}
		
		if(retalts.size()==1)
			ret=retalts.first();
		else if(retalts.isEmpty())
			ret=ExpressionType::Error;
		else
			ret=ExpressionType(ExpressionType::Many, retalts);
	}
	
// 	printAssumptions("ffffffff", ret);
	
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
			} else if(current.type()==ExpressionType::Vector || current.type()==ExpressionType::List) {
				tt=current.contained();
				tt.addAssumptions(current.assumptions());
			} else
				addError(i18n("The domain should be either a vector or a list."));
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
			Apply::const_iterator it=c->firstValue()+1, itEnd=c->constEnd();
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
						QMap<QString, ExpressionType>::const_iterator itf=returntype.assumptions().constFind(name);
						
						if(itf!=returntype.assumptions().constEnd() && itf->type()==ExpressionType::Lambda) {
							ExpressionType oldt=ExpressionType::minimumType(*itf, type);
							oldt.addAssumption(name, oldt);
							QMap<int,ExpressionType> stars;
							stars=ExpressionType::computeStars(stars, type, oldt);
							bool b=ExpressionType::matchAssumptions(&stars, oldt.assumptions(), type.assumptions());
							Q_ASSERT(b);
							
							oldt=oldt.starsToType(stars);
// 							qDebug() << "hmmm..." << oldt << stars << type << "::::::\n\t" << oldt.assumptions() << "\n\t" << type.assumptions();
							returntype=oldt.parameters().last();
// 							printAssumptions("reeeeet", returntype);
							
							returntype.addAssumption(name, oldt);
						} else {
							returntype.addAssumption(name, type);
						}
						
					}
					returntype.addAssumptions(type.assumptions());
					
					ret2.addAlternative(returntype);
				}
				
				current=ret2;
			} else {
				ExpressionType ret(ExpressionType::Many), signature(returned);
				
				QList<ExpressionType> alts=signature.type()==ExpressionType::Many ? signature.alternatives() : QList<ExpressionType>() << signature;
				QList<ExpressionType> args=ExpressionType::lambdaFromArgs(exps);
				bool exit=false;
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
														opt.parameters().size()-1, c->toString()));
							exit=true;
							break;
						}
						
						bool valid=true;
						QMap<QString, ExpressionType> assumptions;
						QMap<int, ExpressionType> starToType, starToParam;
						for(int i=0; valid && i<opt.parameters().size()-1; i++) {
							if(!altargs[i].canCompareTo(opt.parameters()[i])) {
								valid=false;
								break;
							}
							
							starToType=ExpressionType::computeStars(starToType, opt.parameters()[i], altargs[i]);
							
							ExpressionType t=opt.parameters()[i].starsToType(starToType);
							starToParam=ExpressionType::computeStars(starToParam, altargs[i], t);
							valid=!t.isError() && inferType(altargs[i].starsToType(starToParam), t, &assumptions);
							
// 							qDebug() << "pepeluis" << c->toString() << c->m_params[i+1]->toString() << altargs[i] << t << valid;
						}
						
						valid &= ExpressionType::matchAssumptions(&starToType, opt.parameters().last().assumptions(), assumptions);
						
						if(valid) {
							ExpressionType t=opt.parameters().last();
							
							t.addAssumptions(assumptions);
							
							ret.addAlternative(t.starsToType(starToType));
						}
					}
					
					if(exit)
						break;
				}
					
				if(ret.alternatives().isEmpty()) {
// 					qDebug() << "peee" << c->toString() << signature << exps;
					
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
	
	current.removeAssumptions(c->bvarStrings());
	
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

ExpressionType ExpressionTypeChecker::tellTypeIdentity(const QString& name, const ExpressionType& type)
{
	QList<ExpressionType> opts=type.type()==ExpressionType::Many ? type.alternatives() : QList<ExpressionType>() << type;
	
	for(QList< ExpressionType >::iterator it=opts.begin(), itEnd=opts.end(); it!=itEnd; ++it) {
		QMap< QString, ExpressionType >::const_iterator itFound=it->assumptions().constFind(name);
		
		if(itFound!=it->assumptions().constEnd()) {
			QList<ExpressionType> optsFound=itFound->type()==ExpressionType::Many ? itFound->alternatives() : QList<ExpressionType>() << *itFound;
			for(QList< ExpressionType >::iterator itf=optsFound.begin(), itfEnd=optsFound.end(); itf!=itfEnd; ++itf) {
				if(!itf->canReduceTo(type)) {
// 					qDebug() << "incoherent type" << *itf << type;
					addError(i18n("Incoherent type for the variable '%1'", name));
					break;
				}
				QMap<int, ExpressionType> stars;
				stars=ExpressionType::computeStars(stars, *itf, *it);
// 				qDebug() << "fiiiiiiiii" << stars  << "\n\t" << *it << "\n\t" << *itFound;
				*it=it->starsToType(stars);
			}
		}
	}
	return ExpressionType(ExpressionType::Many, opts);
}

//1. Check if parameters are applied correctly
//2. Return the operator result type
QString ExpressionTypeChecker::accept(const Container* c)
{
// 	qDebug() << "XIUXIU" << c->toString();
	switch(c->containerType()) {
		case Container::piecewise: {
			ExpressionType type=commonType(c->m_params);
			
			if(type.isError()) {
				addError(i18n("Could not determine the type for piecewise"));
				current=type;
			} else {
				QList<ExpressionType> alts=type.type()==ExpressionType::Many ? type.alternatives() : QList<ExpressionType>() << type, alts2;
				
				for(QList< ExpressionType >::iterator it=alts.begin(), itEnd=alts.end(); it!=itEnd; ++it) {
					QMap<QString, ExpressionType> assumptions=typeIs(c->constBegin(), c->constEnd(), *it);
					
	// 				QMap<int, ExpressionType> stars;
	// 				bool b=ExpressionType::matchAssumptions(&stars, assumptions, type.assumptions());
	// 				Q_ASSERT(b);
	// 				type=type.starsToType(stars);
					
// 					qDebug() << "suuuuu\n\t" << it->assumptions() << "\n\t" << assumptions;
					QMap<int, ExpressionType> stars;
					bool b=ExpressionType::matchAssumptions(&stars, it->assumptions(), assumptions);
					b&=ExpressionType::assumptionsMerge(it->assumptions(), assumptions);
// 					qDebug() << "fefefe" << b << it->assumptions();
					
					if(b) {
						alts2 += *it;
// 						qDebug() << "!!!" << it->assumptions() << b;
					}
				}
				current=ExpressionType(ExpressionType::Many, alts2);
			}
			
		}	break;
		case Container::piece: {
			QMap<QString, ExpressionType> assumptions=typeIs(c->m_params.last(), ExpressionType(ExpressionType::Bool)); //condition check
			c->m_params.first()->visit(this); //we return the body
			QList<ExpressionType> alts=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current, rets;
			foreach(const ExpressionType& t, alts) {
				QMap<int, ExpressionType> stars;
				ExpressionType toadd=t;
				bool b=ExpressionType::assumptionsMerge(toadd.assumptions(), assumptions);
				
				b&=ExpressionType::matchAssumptions(&stars, assumptions, t.assumptions());
				
				if(b) {
					toadd=toadd.starsToType(stars);
					
					rets += toadd;
				}
			}
			current=ExpressionType(ExpressionType::Many, rets);
		}	break;
		case Container::declare:{
			Q_ASSERT(c->m_params.first()->type()==Object::variable);
			Ci* var = static_cast<Ci*>(c->m_params.first());
			
			m_calculating.append(var->name());
			c->m_params.last()->visit(this);
			m_calculating.removeLast();
			
			current=tellTypeIdentity(var->name(), current);
		}	break;
		case Container::lambda: {
			QSet<QString> aux=m_lambdascope;
			QStringList bvars=c->bvarStrings();
			m_lambdascope+=bvars.toSet();
			c->m_params.last()->visit(this);
			m_lambdascope=aux;
			QMap<QString, ExpressionType> assumptions=current.assumptions();
			
			QList<ExpressionType> alts=current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
			ExpressionType res=ExpressionType(ExpressionType::Many);
			foreach(const ExpressionType& alt, alts) {
				QList<ExpressionType> args;
				foreach(const QString& bvar, bvars) {
					ExpressionType toadd;
					if(alt.assumptions().contains(bvar))
						toadd=alt.assumptionFor(bvar);
					else
						toadd=ExpressionType(ExpressionType::Any, m_stars++);
					
// 					qDebug() << "valueee" << bvar << toadd;
					args += toadd;
				}
				args += alt;
				
				args=ExpressionType::lambdaFromArgs(args);
				
				res.addAlternative(ExpressionType(ExpressionType::Many, args));
			}
			current=res;
			current.removeAssumptions(bvars);
		}	break;
		case Container::otherwise:
		case Container::math:
		case Container::none:
		case Container::downlimit:
		case Container::uplimit:
		case Container::bvar:
		case Container::domainofapplication:
// 			for(Container::const_iterator it=c->constBegin(); it!=c->constEnd(); ++it)
// 			Q_ASSERT(c->constBegin()+1==c->constEnd());
			if(c->constBegin()+1==c->constEnd())
				(*c->constBegin())->visit(this);
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

template <class T>
QString ExpressionTypeChecker::acceptListOrVector(const T* v, ExpressionType::Type t, int size)
{
	ExpressionType cont=commonType(v->values());
	
	if(cont.type()==ExpressionType::Many) {
		ExpressionType toret(ExpressionType::Many);
		foreach(const ExpressionType& contalt, cont.alternatives()) {
			QMap< QString, ExpressionType > assumptions;
			assumptions=typeIs(v->constBegin(), v->constEnd(), contalt);
			ExpressionType cc(t, contalt, size);
			
			bool b=ExpressionType::assumptionsMerge(cc.assumptions(), assumptions);
			if(b)
				toret.addAlternative(cc);
		}
		
		current=toret;
	} else if(!cont.isError()) {
		QMap< QString, ExpressionType > assumptions=typeIs(v->constBegin(), v->constEnd(), cont);
		current=ExpressionType(t, cont, size);
		current.addAssumptions(assumptions);
	} else
		current=ExpressionType(ExpressionType::Error);
	
	return QString();
}

QString ExpressionTypeChecker::accept(const List* l)
{
	acceptListOrVector(l, ExpressionType::List, 0);
	return QString();
}

QString ExpressionTypeChecker::accept(const Vector* l)
{
	acceptListOrVector(l, ExpressionType::Vector, l->size());
	return QString();
}

template <class T>
QMap<QString, ExpressionType> ExpressionTypeChecker::typeIs(T it, const T& itEnd, const ExpressionType& type)
{
	QList<ExpressionType> types;
	for(; it!=itEnd; ++it) {
		(*it)->visit(this);
		types+=current;
	}
	types=ExpressionType::manyFromArgs(types);
	
	bool valid=false;
	QMap<QString, ExpressionType> ret;
	
	foreach(const ExpressionType& tmany, types) {
		bool v=true;
		QMap<QString, ExpressionType> ret2;
		foreach(const ExpressionType& t, tmany.alternatives()) {
			v&=inferType(t, type, &ret2);
		}
		
		if(v) {
			valid=true;
			ExpressionType::assumptionsUnion(ret, ret2);
		}
	}
	
	if(!valid)
		addError(i18n("Unexpected type"));
	return ret;
}

QMap<QString, ExpressionType> ExpressionTypeChecker::typeIs(const Object* o, const ExpressionType& type)
{
	o->visit(this);
// 	qDebug() << "fluuuu" << current << type;
	
	bool corr=false;
	QList<ExpressionType> alts = current.type()==ExpressionType::Many ? current.alternatives() : QList<ExpressionType>() << current;
	
	QMap<QString, ExpressionType> assumptions(m_typeForBVar);
	foreach(const ExpressionType& t, alts) {
		QMap<QString, ExpressionType> ass;
		bool correct = inferType(t, type, &ass);
		if(correct) {
			ExpressionType::assumptionsUnion(assumptions, ass);
		}
		corr|=correct;
	}
	
	if(!corr)
		addError(i18n("Cannot convert '%1' to '%2'", o->toString(), type.toString()));
	
	return assumptions;
}

ExpressionType ExpressionTypeChecker::typeForVar(const QString& var)
{
	if(m_calculating.contains(var))
		return ExpressionType(ExpressionType::Any, m_stars++);
	else if(!m_vars.contains(var)) {
// 		qDebug() << "checking..." << var;
		Q_ASSERT(m_v->contains(var));
		m_calculating += var;
		m_v->value(var)->visit(this);
		m_calculating.removeLast();
		current=tellTypeIdentity(var, current);
		current.clearAssumptions();
		current.simplifyStars();
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
