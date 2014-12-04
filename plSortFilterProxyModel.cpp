#include "plSortFilterProxyModel.h"
#include <QSortFilterProxyModel>
#include <QDebug>

PLSortFilterProxyModel::PLSortFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
}

bool PLSortFilterProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const {
    return sourceColumn == 0;
}

bool PLSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    QString leftData = sourceModel()->data(left).toString();
    QString rightData = sourceModel()->data(right).toString();
    //qDebug() << "lessThan(): leftData=" << leftData << ", rightData=" << rightData << ", answer=" << (leftData < rightData);

    return leftData < rightData;
}

void PLSortFilterProxyModel::sortItems() {
    //qDebug() << "Am I sorting?????";
    sort(0, Qt::AscendingOrder);
}
