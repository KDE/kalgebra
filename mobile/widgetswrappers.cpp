#include "widgetswrappers.h"
#include <QVBoxLayout>

VerticalLayout::VerticalLayout(const QWidget* w) { setLayout(new QVBoxLayout); }
void VerticalLayout::addWidget(QWidget* w) { layout()->addWidget(w); }

////////////////////////////////////

ListWidget::ListWidget(QWidget* parent)
	: QListWidget(parent)
{}

void ListWidget::addItem(const QString& item) { QListWidget::addItem(item); }
void ListWidget::clear() { QListWidget::clear(); }

////////////////////////////////////

TreeView::TreeView(QWidget* parent): QTreeView(parent)
{}

void TreeView::setModel(QObject* model)
{
    QTreeView::setModel(qobject_cast<QAbstractItemModel*>(model));
}
