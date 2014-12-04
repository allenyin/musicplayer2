#pragma once
#include <QSortFilterProxyModel>

class PLSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    PLSortFilterProxyModel(QObject *parent = 0);

protected:
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void sortItems();

protected:
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

};
