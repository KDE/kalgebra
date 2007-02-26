#include "console.h"
#include <QFile>
#include <QHeaderView>

#include "expression.h"

Console::Console(QWidget *parent) : QListWidget(parent), outs(0) {}

Console::~Console() {}

bool Console::addOperation(const QString& op, bool mathml)
{
	Expression res;
	Cn val;
	
	a.setExpression(Expression(op, mathml));
	if(a.isCorrect()) {
		res=a.evaluate();
		
		/*val=a.calculate();
		
		if(!a.isCorrect())
			a.flushErrors();*/
	}
	
	QListWidgetItem *item = new QListWidgetItem(this);
	QFont f = item->font();
	if(a.isCorrect()) {
		a.m_vars->modify("ans", res.tree());
		item->setText(QString("%1 = %2 = %3").arg(a.expression()->toString()).arg(res.toString()).arg(val.toString()));
// 		item->setText(QString("%1\n%2").arg(op).arg(res.value(), 0, 'g', 12));
		item->setToolTip(res.toString());
		if(++outs % 2)
			item->setBackgroundColor(QColor(233,233,222));
		else
			item->setBackgroundColor(QColor(250,250,250));
	} else {
		item->setText(QString("%1\nError: %2").arg(op).arg(a.m_err.join("\n")));
		item->setBackgroundColor(QColor(255,222,222));
		item->setTextAlignment(Qt::AlignRight);
		f.setBold(true);
		item->setFont(f);
	}
	
	qApp->processEvents();
	this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
	emit changed();
	return a.isCorrect();
}

bool Console::loadScript(const QString& path)
{
	bool correct=false;
	if(!path.isEmpty()) {
		QStringList lines;
		QFile file(path);
		if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream stream(&file);
			QString line;
			correct=true;
			while (!stream.atEnd()) {
				line = stream.readLine(); // line of text excluding '\n'
				correct &= addOperation(line);
			}
			file.close();
		}

	}
	return correct;
}

bool Console::saveLog(const QString& path)
{
	bool correct=false;
	if(!path.isEmpty()) {
		QFile file(path);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			correct=false;
		else {
			QTextStream out(&file);
			for(int i=0; i<this->count(); i++)
				out << this->item(i)->text() << endl;
			
			correct=true;
		}
	}
	return correct;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

VariableView::VariableView(QWidget *parent) : QTreeWidget(parent), a(NULL)
{
	setColumnCount(2);
	headerItem()->setText(0, i18n("Name"));
	headerItem()->setText(1, i18n("Value"));
	clear();
	header()->setResizeMode(0, QHeaderView::ResizeToContents);
	setRootIsDecorated(false);
	setSortingEnabled(false);
	setSelectionMode(QAbstractItemView::SingleSelection);
}

VariableView::~VariableView() {}

void VariableView::updateVariables()
{
	clear();
	if(a!=NULL) {
		QList<QTreeWidgetItem *> items;
		QHash<QString, Object*>::const_iterator it = a->m_vars->begin();
		for(; it != a->m_vars->end(); ++it) {
			QTreeWidgetItem* item = new QTreeWidgetItem(this);
			item->setText(0, it.key());
			item->setText(1, it.value()->toString());
			
			items.append(item);
		}
		sortItems(0, Qt::AscendingOrder);
	}
}

#include "console.moc"
