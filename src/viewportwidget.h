/*************************************************************************************
 *  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef VIEWPORTWIDGET_H
#define VIEWPORTWIDGET_H

#include <QWidget>
class QDoubleSpinBox;

class ViewportWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit ViewportWidget(QWidget * parent = 0);
        
        QRectF viewport() const;
        
    public Q_SLOTS:
        void setViewport(const QRectF& current);
        
    private Q_SLOTS:
        void emitViewport();
        
    Q_SIGNALS:
        void viewportChange(const QRectF& newViewport);
        
    private:
        QDoubleSpinBox* m_left;
        QDoubleSpinBox* m_top;
        QDoubleSpinBox* m_width;
        QDoubleSpinBox* m_height;
};

#endif // VIEWPORTDIALOG_H
