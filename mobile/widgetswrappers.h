#ifndef WIDGETSWRAPPERS_H
#define WIDGETSWRAPPERS_H

#include <QListWidget>

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

#endif
