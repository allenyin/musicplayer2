#pragma once
#include <QSortFilterProxyModel>

class PLSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    PLSortFilterProxyModel(QObject *parent = 0);

protected:
    //bool filterAcceptRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;

};
