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

#ifndef ASKNAME_H
#define ASKNAME_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

class AskName : public QDialog
{
public:
    AskName(const QString &text, QWidget *parent)
        : QDialog(parent)
    {
        edit = new QLineEdit(this);
        edit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[a-zA-Z][\\w]*")), edit));

        QDialogButtonBox *buttonBox;
        QVBoxLayout *items = new QVBoxLayout(this);
        items->addWidget(new QLabel(text, this));
        items->addWidget(edit);
        //             items->addItem(new QSpacerItem());
        items->addWidget(buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, this));

        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    QString name() const
    {
        return edit->text();
    }

private:
    QLineEdit *edit;
};

#endif
