/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include "dictionary.h"
#include "operatorsmodel.h"
#include <QLabel>
#include <QListView>
#include <QLayout>
#include <QGroupBox>
#include <QSpacerItem>
#include <KLocale>

Dictionary::Dictionary(QWidget *p) : QWidget(p)
{
	QHBoxLayout *mainLayo= new QHBoxLayout(this);
	m_ops = new OperatorsModel(this);
	
	m_list = new QListView(this);
	m_list->setModel(m_ops);
	m_list->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	
	QGroupBox *descr=new QGroupBox(i18n("Information"), this);
	QVBoxLayout *descrLayo=new QVBoxLayout(descr);
	m_name=new QLabel(descr);
	m_descr=new QLabel(descr);
	m_example=new QLabel(descr);
	
	m_name->setIndent(10);
	m_descr->setIndent(10);
	m_example->setIndent(10);
	
	descrLayo->addWidget(new QLabel(i18n("<b>%1</b>", m_ops->horizontalHeaderItem(0)->text())));
	descrLayo->addWidget(m_name);
	descrLayo->addWidget(new QLabel(i18n("<b>%1</b>", m_ops->horizontalHeaderItem(1)->text())));
	descrLayo->addWidget(m_descr);
	descrLayo->addWidget(new QLabel(i18n("<b>%1</b>", m_ops->horizontalHeaderItem(2)->text())));
	descrLayo->addWidget(m_example);
	descrLayo->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
	
	mainLayo->addWidget(m_list);
	mainLayo->addWidget(descr);
	
	connect(m_list, SIGNAL(clicked ( const QModelIndex & )), this, SLOT(activated(const QModelIndex &)));
}

void Dictionary::activated(const QModelIndex& idx)
{
	QString name=m_ops->item(idx.row(), 0)->text();
	QString description=m_ops->item(idx.row(), 1)->text();
	QString example=m_ops->item(idx.row(), 2)->text();
	
	m_name->setText(name);
	m_descr->setText(description);
	m_example->setText(example);
}

#include "dictionary.moc"
