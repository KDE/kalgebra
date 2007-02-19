#ifndef EXPRESSIONEDIT_H
#define EXPRESSIONEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QCompleter>
#include <QLabel>
#include <QStandardItemModel>

#include "algebrahighlighter.h"
#include "analitza.h"
#include "exp.h"


/**
*	@author Aleix Pol i Gonzalez <aleixpol@gmail.com>
*/

class ExpressionEdit : public QTextEdit
{
Q_OBJECT
public:
	ExpressionEdit(QWidget *parent = 0, AlgebraHighlighter::Mode ini=AlgebraHighlighter::Autodetect);
	~ExpressionEdit();
	bool mode() { return m_highlight->mode(); }
	void setMode(AlgebraHighlighter::Mode en);
	void setAutocomplete(bool a);
	bool autocomplete();
	
	bool isMathML() const;
	QString editingWord(int pos, int &param);
	void setAnalitza(Analitza* in) {a=in; updateCompleter(); }
	QString text() const { return this->toPlainText();}
	void setText(const QString &str) { setPlainText(str);}
	void setCorrect(bool cor);
	bool isCorrect() const { return m_correct; }
	void setCheck(bool check) { m_check=check; }
	bool check() const { return m_check; }
	void setAns(const QString &ans) { m_ans=ans; }
	QString ans() const { return m_ans; }
	void focusInEvent (QFocusEvent * event);
	
private:
	QLabel *m_helptip;
	AlgebraHighlighter *m_highlight;
	
	void helpShow(const QString& funcname, int param=0);
	static QString findPrec(const QString& exp, int &act, int cur, int &param, const QString& tit);
	void ajudant(const QString&, QPoint p);
	QString lastWord(int);
	
	void removenl();
	void keyPressEvent(QKeyEvent * e);
	
	int m_histPos;
	QStringList m_history;
	bool help;
	bool m_auto;
	
	Analitza *a;
	bool m_check;
	bool m_correct;
	QString m_ans;
	QCompleter *m_completer;
	//QStandardItemModel *m_words;
	
public slots:
	void returnP();
	void cursorMov();
	void ajudant(const QString&);
// 	void completed(const QModelIndex &);
	void completed(const QString &);
	void updateCompleter();
signals:
	void signalHelper(const QString&);
	void returnPressed();
};

#endif
