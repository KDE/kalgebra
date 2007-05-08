#include "operatorsmodel.h"

#include "operator.h"

#define KEYWORDNUM 200

OperatorsModel::OperatorsModel(int num, QObject *parent) : QStandardItemModel(num, 4, parent), m_count(Object::nOfOps)
{
	for (int i=0; i<m_count; ++i) {
		setData(index(i, 0), Operator::m_words[i]);
	}
}

/*QString OperatorsModel::operToString(const Operator& op) const
{
	QStandardItem *it;
	
	for(int i=0; i<KEYWORDNUM; i++) {
		it=item(i,2);
		if(it!=NULL && it->data(Qt::EditRole).toInt()==op.operatorType()) {
			return item(i,0)->data(Qt::EditRole).toString();
		}
	}
	return QString();
}*/
