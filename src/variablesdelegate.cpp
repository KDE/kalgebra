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

#include "variablesdelegate.h"

#include <QDoubleSpinBox>

QWidget *VariablesDelegate::createEditor(QWidget *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const
{
    QVariant val = idx.model()->data(idx);
    if (val.metaType().id() == QMetaType::Double) {
        QDoubleSpinBox *spin = new QDoubleSpinBox(p);
        spin->setDecimals(10);
        return spin;
    } else
        return QItemDelegate::createEditor(p, opt, idx);
}

void VariablesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(editor);
    if (spin)
        spin->setValue(index.model()->data(index).value<double>());
    else
        QItemDelegate::setEditorData(editor, index);
}
