#include "plSortFilterProxyModel.h"
#include "playlistLibraryModel.h"
#include <QSortFilterProxyModel>
#include <QDebug>

class PlaylistLibraryModel;

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

bool PLSortFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
       static_cast<PlaylistLibraryModel *>(sourceModel())->changePlaylistName(mapToSource(index), value.toString());
       return true;
    }
    qDebug() << "Or Am I here?";
    return false;
}

Qt::ItemFlags PLSortFilterProxyModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    else {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
}
