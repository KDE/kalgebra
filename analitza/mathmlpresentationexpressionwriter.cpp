/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
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

static QStringList convertElements(const Vector* c, MathMLPresentationExpressionWriter* w)
{
	QStringList elems;
	Vector::const_iterator it=c->constBegin(), itEnd=c->constEnd();
	for(; it!=itEnd; ++it) {
		elems += (*it)->visit(w);
	}
	return elems;
}

static QStringList convertElements(const Container* c, MathMLPresentationExpressionWriter* w)
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

// template <char C>
// static QString joinOp(const Container* c, MathMLPresentationExpressionWriter* w)
// {
// 	QString op=QString("<mo>%1</mo>").arg(C);
// 	return convertElements(c, w).join(op);
// }

static QString minus(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList e=convertElements(c, w);
	if(e.count()==1)
		return "<mo>-</mo>"+e[0];
	else
		return e.join("<mo>-</mo>");
}

static QString power(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<msup>"+convertElements(c, w).join(QString())+"</msup>";
}

static QString divide(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<mfrac><mrow>"+convertElements(c, w).join("</mrow><mrow>")+"</mrow></mfrac>";
}

static QString quotient(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return divide(c, w);
}

static QString root(const Container* c, MathMLPresentationExpressionWriter* w)
{
	return "<msqrt>"+convertElements(c, w).join(QString())+"</msqrt>";
}

static QString diff(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList bv=c->bvarList();
	return "<msubsup><mrow><mo>(</mo>"+convertElements(c, w).join(QString())+"<mo>)</mo></mrow>"
			"<mrow>"+bv.join("<mo>,</mo>")+"</mrow><mo>'</mo></msubsup>";
}

const char* plus="+", *times="*", *equal="=";
const char* lt="&lt;", *gt="&gt;";
const char* leq="&leq;", *geq="&geq;", *neq="&NotEqual;";
const char* implies="&DoubleRightArrow;", *_and="&And;", *_or="&Or;", *_xor="&CirclePlus;";

MathMLPresentationExpressionWriter::operatorToString
	MathMLPresentationExpressionWriter::m_operatorToPresentation[] = { 0,
			joinOp<&plus>, joinOp<&times>,
			minus, divide, quotient,
			power, root, /*factorial*/0,
			joinOp<&_and>,joinOp<&_or>,joinOp<&_xor>, 0/*not*/,
			0,0,0,0,//gcd, lcm, rem, factorof,
			0,0,//max, min,
			joinOp<&lt>, joinOp<&gt>,
			joinOp<&equal>,
			joinOp<&neq>, joinOp<&leq>, joinOp<&geq>, joinOp<&implies>,
			0,0,0,0,// approx, abs, floor, ceiling,
			0,0,0,// sin, cos, tan,
			0,0,0,// sec, csc, cot,
			0,0,0,// sinh, cosh, tanh,
			0,0,0,// sech, csch, coth,
			0,0,0,// arcsin, arccos, arctan,
			0,// arccot,// arccoth,
			0,0,0,// arccosh, arccsc, arccsch,
			0,0,0,0,// arcsec, arcsech, arcsinh, arctanh,
			0,0,0,// exp, ln, log,
// 			0,0,0,0,// // 			conjugate, arg, real, imaginary,
			0,0,diff// sum, product, diff,
			// card, scalarproduct, selector,
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
				+piece->m_params.last()->visit(w)+
			"</mtd>"
			"<mtd>"
				"<mtext>if </mtext>"
				+piece->m_params.first()->visit(w)+
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
	Operator op=c->firstOperator();
// 	objectWalker(c);
// 	qDebug() << "ttttttttttt" << m_operatorToPresentation << op.operatorType()
// 			 << m_operatorToPresentation[op.operatorType()] << op.name();
	
	if(m_operatorToPresentation[op.operatorType()]!=0) {
		operatorToString call=m_operatorToPresentation[op.operatorType()];
		ret = call(c, this);
	} else if(c->containerType()==Container::math) {
		ret="<math><mrow>"+convertElements(c, this).join(QString())+"</mrow></math>";
	} else if(op.operatorType()!=0) {
		QString bvars;
		if(!c->bvarList().isEmpty()) {
			bvars="<mo>:</mo>"+c->bvarList().join("<mo>,</mo>");
			if(c->bvarList().size()>1)
				bvars="<mo>(</mo>"+bvars+"<mo>)</mo>";
			const Object *ul=c->ulimit(), *dl=c->dlimit();
			if(ul || dl) {
				bvars += "<mo>=</mo>";
				if(dl) bvars += dl->visit(this);
				bvars += "<mo>..</mo>";
				if(ul) bvars += ul->visit(this);
			}
		}
		
		ret="<mi>"+op.name()+"</mi>"
			"<mo>(</mo>"
			+convertElements(c, this).join("<mo>,</mo> ")
			+bvars
			+"<mo>)</mo>";
	} else if(c->containerType()==Container::piecewise) {
		ret=piecewise(c, this);
	}
	
	return ret;
}

QString MathMLPresentationExpressionWriter::accept(const Vector* var)
{
	return "<mo>&lt;</mo>"+convertElements(var, this).join("<mo>,</mo>")+"<mo>&gt;</mo>";
}
