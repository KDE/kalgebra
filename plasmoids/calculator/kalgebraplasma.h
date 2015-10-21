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


#ifndef KALGEBRAPLASMA_HEADER
#define KALGEBRAPLASMA_HEADER

#include <Plasma/PopupApplet>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <QGraphicsLinearLayout>
#include <analitza/analyzer.h>


class KAlgebraPlasmoid : public Plasma::Applet
{
    Q_OBJECT
    public:
        KAlgebraPlasmoid(QObject *parent, const QVariantList &args);
        ~KAlgebraPlasmoid();
        
        void init();
        virtual QQuickItem* graphicsWidget();
        
    private Q_SLOTS:
        void simplify();
        void addOperation();
        
    private:
        static QColor correctColor();
        static QColor errorColor();
        static int simplificationSize();
        
        void plasmoidFont(bool big, const QColor& c, bool bold);
        
        QQuickItem* m_widget;
        QGraphicsLinearLayout* m_layout;
        Plasma::LineEdit *m_input;
        Plasma::Label *m_output;
        Analitza::Analyzer a;
};

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(kalgebraplasmoid, KAlgebraPlasmoid)

#endif
