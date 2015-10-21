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

#include "kalgebraplasma.h"

#include <QFontMetrics>
#include <QGraphicsProxyWidget>
#include <QFont>
#include <KIcon>
#include <KIconLoader>

#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <plasma/tooltipcontent.h>
#include <plasma/tooltipmanager.h>

#include <analitza/expression.h>

using namespace Plasma;
using Analitza::Expression;

QColor KAlgebraPlasmoid::correctColor() { return Theme::defaultTheme()->color(Theme::TextColor);}
QColor KAlgebraPlasmoid::errorColor() { return Qt::red; }
int KAlgebraPlasmoid::simplificationSize() {  return Theme::defaultTheme()->font(Theme::DefaultFont).pointSize(); }

KAlgebraPlasmoid::KAlgebraPlasmoid(QObject *parent, const QVariantList &args)
    : PopupApplet(parent, args), m_widget(0), m_layout(0)
{
    KGlobal::locale()->insertCatalog("kalgebra");
    setAspectRatioMode(IgnoreAspectRatio);
    setAssociatedApplication("kalgebra");
}

KAlgebraPlasmoid::~KAlgebraPlasmoid() {}

void KAlgebraPlasmoid::init()
{
//     updateFactor();
    
    setPopupIcon("kalgebra");
}

QQuickItem* KAlgebraPlasmoid::graphicsWidget()
{
    if(!m_widget) {
        m_widget = new QQuickItem(this);
        m_input = new Plasma::LineEdit(m_widget);
        m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_input->setClearButtonShown(true);
        
        m_output = new Plasma::Label(m_widget);
        m_output->setMinimumSize(20, 20);
        m_output->nativeWidget()->setAlignment(Qt::AlignCenter);
        m_output->setText(i18n("Enter some expression."));
        
        m_layout = new QGraphicsLinearLayout(m_widget);
        m_layout->setOrientation(Qt::Vertical);
        m_layout->addItem(m_input);
        m_layout->addItem(m_output);
        m_widget->setPreferredSize(300,300);
        
        connect(m_input, SIGNAL(editingFinished()), this, SLOT(addOperation()));
        connect(m_input->nativeWidget(), SIGNAL(textChanged(QString)), this, SLOT(simplify()));
    }
    m_input->nativeWidget()->selectAll();
    m_input->setFocus();
    
    return m_widget;
}

void KAlgebraPlasmoid::addOperation()
{
    if ( m_input->text().isEmpty() )
        return;
    
    Expression res;
    a.setExpression(Expression(m_input->text(), false));
    if(a.isCorrect()) {
        res=a.evaluate();
    }
    
    QColor c;
    if(a.isCorrect()) {
        QString result=res.toString();
        m_output->setText(result);
        
        Plasma::ToolTipContent data;
        data.setMainText(i18n("KAlgebra"));
        data.setSubText(i18n("%1 = %2", m_input->text(), result));
        data.setImage(QIcon::fromTheme("kalgebra").pixmap(IconSize(KIconLoader::Desktop)));
        Plasma::ToolTipManager::self()->setContent(this, data);
        
        c=correctColor();
    } else {
        m_output->setText(a.errors().join("\n"));
        c=errorColor();
    }
    plasmoidFont(true, c, true);
    update();
}

void KAlgebraPlasmoid::plasmoidFont(bool big, const QColor& c, bool bold)
{
    QFont f=m_output->nativeWidget()->font();
    f.setBold(bold);
    int size;
    
    if(big) {
        size=(m_output->size().height()*2)/3;
        f.setPointSize(size);
        QFontMetrics fm(f);
        
        int w=m_output->size().width();
        Q_ASSERT(w>0);
        for(; fm.width(m_output->text()) > w; size--) {
            f.setPointSize(size);
            fm=QFontMetrics(f);
        }
    } else
        size=simplificationSize();
    f.setPointSize(size);
    
    QPalette palette = m_output->palette();
    palette.setColor(QPalette::WindowText, c);
    m_output->nativeWidget()->setPalette(palette);
    m_output->nativeWidget()->setFont(f);
}

void KAlgebraPlasmoid::simplify()
{
    Expression e(m_input->text(), false);
    if(e.isCorrect())
        a.setExpression(e);
    
    if(e.isCorrect() && a.isCorrect()) {
        a.simplify();
        m_output->setText(a.expression().toString());
        
        plasmoidFont(false, correctColor(), true);
    } else
        m_output->setText(QString());
}


