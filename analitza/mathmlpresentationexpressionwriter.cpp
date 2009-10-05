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

#include "mathmlpresentationexpressionwriter.h"
#include "value.h"
#include "operator.h"
#include "container.h"
#include <QStringList>
#include "vector.h"
#include "list.h"
#include "variable.h"

namespace
{

QStringList convertElements(const Vector* c, MathMLPresentationExpressionWriter* w)
{
	QStringList elems;
	Vector::const_iterator it=c->constBegin(), itEnd=c->constEnd();
	for(; it!=itEnd; ++it) {
		elems += (*it)->visit(w);
	}
	return elems;
}

QStringList convertElements(const List* c, MathMLPresentationExpressionWriter* w)
{
	QStringList elems;
	List::const_iterator it=c->constBegin(), itEnd=c->constEnd();
	for(; it!=itEnd; ++it) {
		elems += (*it)->visit(w);
	}
	return elems;
}

QStringList convertElements(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList elems;
	Container::const_iterator it=c->firstValue(), itEnd=c->m_params.constEnd();
	for(; it!=itEnd; ++it) {
		elems += (*it)->visit(w);
	}
	return elems;
}

template <const char **C>
static QString joinOp(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QString op=QString("<mo>%1</mo>").arg(*C);
	return convertElements(c, w).join(op);
}

template <const char **C, const char **D>
static QString infix(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QString exp=QString("<mo>%1</mo>%2<mo>%3</mo>").arg(*C)
		.arg(convertElements(c, w).join(QString())).arg(*D);
	return exp;
}

template <const char **C>
static QString prefix(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return QString("<mo>%1</mo>").arg(*C)+convertElements(c, w).join(QString());
}

template <const char **C>
static QString postfix(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return convertElements(c, w).join(QString())+QString("<mo>%1</mo>").arg(*C);
}

QString minus(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList e=convertElements(c, w);
	if(e.count()==1)
		return "<mo>-</mo>"+e[0];
	else
		return e.join("<mo>-</mo>");
}

QString power(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<msup>"+convertElements(c, w).join(QString())+"</msup>";
}

QString divide(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<mfrac><mrow>"+convertElements(c, w).join("</mrow><mrow>")+"</mrow></mfrac>";
}

QString quotient(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return divide(c, w);
}

QString root(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<mroot>"+convertElements(c, w).join(QString())+"</mroot>";
}

QString diff(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList bv=c->bvarList();
	return "<msubsup><mfenced>"+convertElements(c, w).join(QString())+"</mfenced>"
			"<mrow>"+bv.join("<mo>,</mo>")+"</mrow><mo>'</mo></msubsup>";
}

QString exp(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<msup><mn>&ExponentialE;</mn>"+convertElements(c, w).first()+"</msup>";
}

QString logE(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<msub><mo>log</mo><mn>&ExponentialE;</mn></msub>"+convertElements(c, w).first();
}

QString log10(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<msub><mo>log</mo><mn>10</mn></msub>"+convertElements(c, w).first();
}

QString iterative(Operator::OperatorType t, const Container* c, MathMLPresentationExpressionWriter* w)
{
	QString op = t==Operator::sum ? "&Sum;" : "&Prod;";
	QString ul="<mrow>"+c->ulimit()->toString()+"</mrow>";
	QString dl="<mrow>"+c->bvarList().join(", ")+"<mo>=</mo>"+c->dlimit()->toString()+"</mrow>";
	
	return "<mrow><msubsup><mo>"+op+"</mo>"+dl+ul+"</msubsup>"+convertElements(c, w).join(QString())+"</mrow>";
}

QString sum(const Container* c, MathMLPresentationExpressionWriter* w)
{ return iterative(Operator::sum, c, w); }

QString product(const Container* c, MathMLPresentationExpressionWriter* w)
{ return iterative(Operator::product, c, w); }

QString selector(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList el=convertElements(c, w);
	return "<msub>"+el.last()+el.first()+"</msub>";
}

const char* plus="+", *times="*", *equal="=";
const char* lt="&lt;", *gt="&gt;", *_not="&not;";
const char* leq="&leq;", *geq="&geq;", *neq="&NotEqual;", *approx="&asymp;";
const char* implies="&DoubleRightArrow;", *_and="&and;", *_or="&or;", *_xor="&CirclePlus;";
const char* mabs="|", *factorial="!";
const char *lfloor="&lfloor;", *rfloor="&rfloor;";
const char *lceil="&lceil;", *rceil="&rceil;";
const char *cardinal="#", *scalarproduct="X";
}

MathMLPresentationExpressionWriter::operatorToString
	MathMLPresentationExpressionWriter::m_operatorToPresentation[] = { 0,
			joinOp<&plus>, joinOp<&times>,
			minus, divide, quotient,
			power, root, postfix<&factorial>,
			joinOp<&_and>,joinOp<&_or>,joinOp<&_xor>, prefix<&_not>,
			0,0,0,0,//gcd, lcm, rem, factorof,
			0,0,//max, min,
			joinOp<&lt>, joinOp<&gt>,
			joinOp<&equal>,
			joinOp<&neq>, joinOp<&leq>, joinOp<&geq>, joinOp<&implies>,
			joinOp<&approx>, infix<&mabs, &mabs>, infix<&lfloor, &rfloor>, infix<&lceil, &rceil>,
			// approx, abs, floor, ceiling,
			0,0,0,// sin, cos, tan,
			0,0,0,// sec, csc, cot,
			0,0,0,// sinh, cosh, tanh,
			0,0,0,// sech, csch, coth,
			0,0,0,// arcsin, arccos, arctan,
			0,// arccot,// arccoth,
			0,0,0,// arccosh, arccsc, arccsch,
			0,0,0,0,// arcsec, arcsech, arcsinh, arctanh,
			exp, logE, log10,// exp, ln, log,
// 			0,0,0,0,// // 			conjugate, arg, real, imaginary,
			sum, product, diff,// sum, product, diff,
			prefix<&cardinal>, joinOp<&scalarproduct>, selector
			// function
	};

MathMLPresentationExpressionWriter::MathMLPresentationExpressionWriter(const Object* o)
{
	m_result=o->visit(this);
}

QString MathMLPresentationExpressionWriter::accept(const Ci* var)
{
	return "<mi>"+var->name()+"</mi>";
}

QString MathMLPresentationExpressionWriter::accept(const Operator* op)
{
	return op->name();
}

QString MathMLPresentationExpressionWriter::accept(const Cn* val)
{
	if(val->isBoolean()) {
		if(val->isTrue())
			return "<mo>true</mo>";
		else
			return "<mo>false</mo>";
	} else
		return QString("<mn>%1</mn>").arg(val->value(), 0, 'g', 12);

}

QString piecewise(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QString ret="<mrow>"
	"<mo stretchy='true'> { </mo>"
	"<mtable columnalign='left left'>";
	for(QList<Object*>::const_iterator it=c->firstValue(); it!=c->m_params.constEnd(); ++it) {
		Q_ASSERT((*it)->isContainer());
		Container *piece=static_cast<Container*>(*it);
		if(piece->containerType()==Container::piece) {
			ret += "<mtr>"
			"<mtd>"
				+piece->m_params.first()->visit(w)+
			"</mtd>"
			"<mtd>"
				"<mtext>if </mtext>"
				+piece->m_params.last()->visit(w)+
			"</mtd>"
			"</mtr>";
		} else {
			ret += "<mtr>"
			"<mtd>"
				+piece->m_params.first()->visit(w)+
			"</mtd>"
			"<mtd>"
				"<mtext>otherwise</mtext>"
			"</mtd>"
			"</mtr>";
		}
	}
	
	ret+="</mtable></mrow>";
	return ret;
}

QString MathMLPresentationExpressionWriter::accept(const Container* c)
{
	QString ret;
// 	objectWalker(c);
// 	qDebug() << "ttttttttttt" << m_operatorToPresentation << op.operatorType()
// 			 << m_operatorToPresentation[op.operatorType()] << op.name();
	
	switch(c->containerType()) {
		case Container::math:
			ret="<math><mrow>"+convertElements(c, this).join(QString())+"</mrow></math>";
			break;
		case Container::apply: {
			Operator op=c->firstOperator();
			if(m_operatorToPresentation[op.operatorType()]!=0) {
				operatorToString call=m_operatorToPresentation[op.operatorType()];
				ret = call(c, this);
			} else if(op.operatorType()!=0) {
				QString bvars;
				if(!c->bvarList().isEmpty()) {
					bvars=c->bvarList().join(QString());
					if(c->bvarList().size()>1)
						bvars="<mfenced>"+bvars+"</mfenced>";
					const Object *ul=c->ulimit(), *dl=c->dlimit();
					if(ul || dl) {
						bvars += "<mo>=</mo>";
						if(dl) bvars += dl->visit(this);
						bvars += "<mo>..</mo>";
						if(ul) bvars += ul->visit(this);
					}
					bvars="<mo>:</mo>"+bvars;
				}
				
				ret="<mi>"+op.name()+"</mi>"
					"<mo> &ApplyFunction; </mo>"
					"<mfenced>"
					+convertElements(c, this).join(QString())
					+bvars
					+"</mfenced>";
			}
		}	break;
		case Container::piecewise:
			ret=piecewise(c, this);
			break;
		case Container::otherwise:
		case Container::piece:
		case Container::bvar:
		case Container::uplimit:
		case Container::downlimit:
		case Container::lambda:
		case Container::declare:
		case Container::none:
			Q_ASSERT(false);
			break;
	}
	
	return ret;
}

QString MathMLPresentationExpressionWriter::accept(const Vector* var)
{
	return "<mrow><mo>&lt;</mo>"+convertElements(var, this).join("<mo>,</mo>")+"<mo>&gt;</mo></mrow>";
}

QString MathMLPresentationExpressionWriter::accept(const List* var)
{
	return "<mrow><mo>[</mo>"+convertElements(var, this).join("<mo>,</mo>")+"<mo>]</mo></mrow>";
}
