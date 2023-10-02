/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef VAREDIT_H
#define VAREDIT_H

#include <QDialog>
#include <QLabel>

class QDialogButtonBox;
namespace Analitza
{
class Analyzer;
class Variables;
class Expression;
class ExpressionEdit;
}

/**
 *    The VarEdit provides a dialog to allow users to edit/create a variable.
 *    @author Aleix Pol i Gonzalez
 */

class VarEdit : public QDialog
{
    Q_OBJECT
public:
    /** Constructor. Creates a variable editing dialog. */
    explicit VarEdit(QWidget *parent = nullptr, bool modal = false);

    /** Sets the editing variable name */
    void setName(const QString &newVar);

    /** Sets an Analitza which will evaluate it. It may be interesting because variables can change. */
    void setAnalitza(Analitza::Analyzer *na);

    /** Returns the resulting variable expression */
    Analitza::Expression val();

private:
    bool canRemove(const QString &name) const;

    Analitza::ExpressionEdit *m_exp;

    QLabel *m_valid;
    QSharedPointer<Analitza::Variables> vars;
    bool m_correct;
    QString m_var;
    QDialogButtonBox *m_buttonBox;
    QPushButton *m_removeBtn;

private Q_SLOTS:
    void edit();
    void ok();
    void removeVariable();
};

#endif
