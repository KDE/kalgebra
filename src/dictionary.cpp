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
#include "functionsmodel.h"
#include "expression.h"
#include "graph2d.h"

#include <QLabel>
#include <QListView>
#include <QGroupBox>
#include <QSpacerItem>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <KLineEdit>
#include <KLocale>

Dictionary::Dictionary(QWidget *p) : QWidget(p)
{
	QHBoxLayout *mainLayo= new QHBoxLayout(this);
	m_ops = new OperatorsModel(this);
	m_sortProxy = new QSortFilterProxyModel(this);
	m_sortProxy->setSourceModel(m_ops);
	m_sortProxy->sort(2, Qt::AscendingOrder);
	m_sortProxy->setFilterKeyColumn(2);
	
	QLayout *leftLayo=new QVBoxLayout();
	m_filter=new KLineEdit(this);
	m_filter->setClearButtonShown(true);
	connect(m_filter, SIGNAL(textChanged(const QString&)), this, SLOT(filterChanged(const QString&)));
	m_filter->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
	m_list = new QListView(this);
	m_list->setModel(m_sortProxy);
	m_list->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
	leftLayo->addWidget(new QLabel("Look for:", this));
	leftLayo->addWidget(m_filter);
	leftLayo->addWidget(m_list);
	
	QGroupBox *descr=new QGroupBox(i18n("Information"), this);
	QFormLayout *descrLayo=new QFormLayout;
	QVBoxLayout *graphLayo=new QVBoxLayout;
	m_name=new QLabel(descr);
	m_descr=new QLabel(descr);
	m_sample=new QLabel(descr);
	m_example=new QLabel(descr);
	m_funcs=new FunctionsModel(descr);
	m_graph=new Graph2D(m_funcs, descr);
	m_graph->setReadOnly(true);
	m_graph->setViewport(QRect(QPoint(-30, 7), QPoint(30, -7)));
	m_graph->setResolution(400);
	m_graph->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	m_name->setIndent(10);
	m_descr->setIndent(10);
	m_sample->setIndent(10);
	m_example->setIndent(10);
	
	descrLayo->addRow(i18n("<b>%1</b>", m_ops->headerData(0, Qt::Horizontal).toString()), m_name);
	descrLayo->addRow(i18n("<b>%1</b>", m_ops->headerData(1, Qt::Horizontal).toString()), m_descr);
	descrLayo->addRow(i18n("<b>%1</b>", m_ops->headerData(2, Qt::Horizontal).toString()), m_sample);
	descrLayo->addRow(i18n("<b>%1</b>", m_ops->headerData(3, Qt::Horizontal).toString()), m_example); 
	graphLayo->addWidget(descr);
	graphLayo->addWidget(m_graph);
	descr->setLayout(descrLayo);
	
	mainLayo->addLayout(graphLayo);
	mainLayo->addLayout(leftLayo);
	
// 	connect(m_list, SIGNAL(clicked ( const QModelIndex & )), this, SLOT(activated(const QModelIndex &)));
	connect(m_list->selectionModel(), SIGNAL(currentChanged ( const QModelIndex &, const QModelIndex & )),
		this, SLOT(activated(const QModelIndex& , const QModelIndex& )));
}

void Dictionary::activated(const QModelIndex& idx, const QModelIndex& prev)
{
	Q_UNUSED(prev);
	QModelIndex nameIdx, descriptionIdx, sampleIdx, exampleIdx;
	nameIdx = idx.sibling(idx.row(), 0);
	descriptionIdx = idx.sibling(idx.row(), 1);
	sampleIdx = idx.sibling(idx.row(), 2);
	exampleIdx = idx.sibling(idx.row(), 3);
	
	QString name=m_sortProxy->data(nameIdx).toString();
	QString description=m_sortProxy->data(descriptionIdx).toString();
	QString sample=m_sortProxy->data(sampleIdx).toString();
	QString example=m_sortProxy->data(exampleIdx).toString();
	
	m_name->setText(name);
	m_descr->setText(description);
	m_sample->setText(sample);
	m_example->setText(example);
	
	m_funcs->clear();
	m_funcs->addFunction(function("func", Expression(example, false), QColor(0,150,0)));
}

void Dictionary::filterChanged(const QString &filter)
{
	m_sortProxy->setFilterFixedString(filter);
}

#include "dictionary.moc"
