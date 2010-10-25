#ifndef WIDGETSWRAPPERS_H
#define WIDGETSWRAPPERS_H

#include <QListWidget>
#include <QTreeView>

class VerticalLayout : public QWidget
{
	Q_OBJECT
	public:
		VerticalLayout(const QWidget* w=0);
		
	public slots:
		void addWidget(QWidget* w);
};

class ListWidget : public QListWidget
{
	Q_OBJECT
	public:
		explicit ListWidget(QWidget* parent = 0);
		
	public slots:
		void addItem(const QString& item);
		void clear();
};

class TreeView : public QTreeView
{
	Q_OBJECT
	public:
		explicit TreeView(QWidget* parent = 0);
		Q_SCRIPTABLE virtual void setModel(QObject* model);
};

#endif
