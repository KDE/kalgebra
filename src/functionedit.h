#ifndef FUNCTIONEDIT_H
#define FUNCTIONEDIT_H

#include <QLayout>
#include <QLabel>
#include <QLocale>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QList>

#include "graph2d.h"
#include "expressionedit.h"
#include "algebrahighlighter.h"

/**
@author Aleix Pol i Gonzalez
*/

class ColorCombo : public QComboBox
{
Q_OBJECT
public:
	ColorCombo(QWidget* parent=0);
	~ColorCombo(){}
	
	QColor color() const;
	void setColor(const QColor &);
	
	void resizeEvent(QResizeEvent * event);
};

class FunctionEdit : public QWidget
{
Q_OBJECT
public:
	FunctionEdit(QWidget *parent=0, Qt::WFlags f = 0 );
	~FunctionEdit();
	QString text() const { return m_func->toPlainText(); }
	void setText(const QString &newText);
	QColor color() const { return m_color->color(); }
	void setColor(const QColor &newColor);
	bool isCorrect() const { return m_correct; }
	bool isMathML() const { return m_func->isMathML();}
	bool editing() const { return m_modmode; }
	void setEditing(bool m) { m_modmode=m; m_valid->setText("editing"); }
	void setName(const QString& name) { m_name->setText(name); }
	QString name() const { return m_name->text(); }
private:
	ExpressionEdit *m_func;
	QLineEdit *m_name;
	QPushButton *m_ok;
	QLabel *m_valid;
	Graph2D *m_graph;
	ColorCombo *m_color;
	
	bool m_correct;
	bool m_modmode;
	
	void focusInEvent(QFocusEvent*){ m_func->setFocus(); }
private slots:
	void edit();
	void ok();
	void colorChange(int);
	
public slots:
	void clear();
	
signals:
	void accept();
};

#endif
