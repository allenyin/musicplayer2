#include "plSortFilterProxyModel.h"
#include <QSortFilterProxyModel>

PLSortFilterProxyModel::PLSortFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
}

bool PLSortFilterProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const {
    return sourceColumn == 0;
}
