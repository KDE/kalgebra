#ifndef VAREDIT_H
#define VAREDIT_H

#include <QToolTip>
#include <QDialog>
#include <QLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>

#include "expressionedit.h"

/**
@author Aleix Pol i Gonzalez
*/

class VarEdit : public QDialog
{
Q_OBJECT
public:
	VarEdit(QWidget *parent = 0, bool modal = FALSE);
	~VarEdit();
	QString text() const { return m_exp->text(); }
	void setVar(const QString& newVar);	//This should edit the variable name
	void setMode(int mode);			//This should select what Option we need
	void setAnalitza(Analitza *na) { vars= na->m_vars; m_exp->setAnalitza(na); }
	Object* val();
private:
	ExpressionEdit *m_exp;
	
	QRadioButton *m_opt_calc;		//Per escollir guardar calcul
	QRadioButton *m_opt_exp;		//Per escollir guardar expressio
	
	QLabel *m_valid;
	Variables *vars;
	bool m_correct;
	QString m_var;
	
	QDialogButtonBox *buttonBox;
private slots:
	void edit();
	void ok();
};

#endif
