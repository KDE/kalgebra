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


QStringList convertElements(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QStringList elems;
	for(QList<Object*>::const_iterator it=c->firstValue(); it!=c->m_params.constEnd(); ++it) {
		elems += (*it)->visit(w);
	}
	return elems;
}

template <const char **C>
QString joinOp(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QString op=QString("<mo>%1</mo>").arg(*C);
	return convertElements(c, w).join(op);
}

template <char C>
QString joinOp(const Container* c, MathMLPresentationExpressionWriter* w)
{
	QString op=QString("<mo>%1</mo>").arg(C);
	return convertElements(c, w).join(op);
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
	return "<msqrt>"+convertElements(c, w).join(QString())+"</msqrt>";
}

#ifdef Q_CC_MSVC
const char* plus="+", *mult="*", *equal="=";
#endif
const char* lt="&lt;", *gt="&gt;";
const char* leq="&lt;=", *geq="&gt;=", *neq="&NotEqual;", *implies="=&gt;", *_and="&And;", *_or="&Or;", *_xor="&CirclePlus;";
MathMLPresentationExpressionWriter::operatorToString
	MathMLPresentationExpressionWriter::m_operatorToPresentation[] = { 0,
#ifdef Q_CC_MSVC
			joinOp<&plus>, joinOp<&mult>,
#else
			joinOp<'+'>, joinOp<'*'>,
#endif
            minus, divide, quotient,
			power, root, /*factorial*/0,
			joinOp<&_and>,joinOp<&_or>,joinOp<&_xor>, 0/*not*/,
			0,0,0,0,//gcd, lcm, rem, factorof,
			0,0,//max, min,
			joinOp<&lt>, joinOp<&gt>,
#ifdef Q_CC_MSVC
            joinOp<&equal>,
#else
            joinOp<'='>,
#endif
            joinOp<&neq>, joinOp<&leq>, joinOp<&geq>, joinOp<&implies>
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
		ret="<mi>"+op.name()+"</mi>"
			"<mo>(</mo>"
			+convertElements(c, this).join("<mo>,</mo> ")+
			"<mo>)</mo>";
	} else if(c->containerType()==Container::piecewise) {
		ret=piecewise(c, this);
	}
	
	return ret;
}

QString MathMLPresentationExpressionWriter::accept(const Vector* var)
{
	Q_UNUSED(var);
	//TODO
	return QString();
}
