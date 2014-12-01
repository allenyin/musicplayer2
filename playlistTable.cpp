#include "playlistTable.h"
#include "playlistmodel.h"
#include <stdio.h>
#include <iostream>
#include <QApplication>
#include <QHeaderView>
#include <QMimeData>
#include <QDebug>

class PlaylistModel;

PlaylistTable::PlaylistTable(QWidget* parent) : QTableView(parent) {
    // edit only when F2 is pressed
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    // entire row is selected when any item is clicked
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    // enable drag and drop
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false);
    setDragDropMode(QTableView::DragDrop);
}

PlaylistTable::~PlaylistTable() {
}

void PlaylistTable::mouseDoubleClickEvent(QMouseEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
#if DEBUG_PLAYLISTVIEW
    qDebug()<< "Double click at (" << clickPos.x() << ", " << clickPos.y();
    qDebug()<< "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column();
#endif
    emit QTableView::activated(clickIdx);
}

void PlaylistTable::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        // Use base handler
        std::cout << "Single left click here!\n";
        QTableView::mouseReleaseEvent(e);
    }
    else {
        QTableView::mouseReleaseEvent(e);
    }
}

#if DEBUG_PLAYLISTVIEW
void PlaylistTable::contextMenuEvent(QContextMenuEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);

    qDebug() << "Right click at (" << clickPos.x() << ", " << clickPos.y() << ")";
    qDebug() << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")";
}
#endif

void PlaylistTable::keyPressEvent(QKeyEvent *event) {
    // selectionMode must be contiguous
    if (event->key() == Qt::Key_Delete) {
        QModelIndexList selected = selectionModel()->selectedRows();
#if DEBUG_PLAYLISTVIEW
        qDebug()<< "Delete key detected on" << selected;
#endif
        PlaylistModel *model = (PlaylistModel*)(QTableView::model());
        model->removeMedia(selected.front().row(), selected.back().row());
    }
    else {
        QTableView::keyPressEvent(event);
    }
}

void PlaylistTable::dragEnterEvent(QDragEnterEvent *event) {
//#if DEBUG_PLAYLIST
//    qDebug()<<"dragEnterEvent";
//#endif

    if (event->mimeData()->hasFormat("playlistItem") ||
        event->mimeData()->hasFormat("libraryItem")) {
        qDebug() << "playlistTable::dragEnterEvent()";

#if DEBUG_PLAYLISTVIEW
        QPoint dragPos = event->pos();
        QModelIndex dragIdx = QTableView::indexAt(dragPos);
        qDebug()<<"dragIdx is " << dragIdx;
#endif
        event->accept();
    }
    else {
        event->ignore();
    }
}

void PlaylistTable::dragMoveEvent(QDragMoveEvent *event) {
//#if DEBUG_PLAYLISTVIEW
//    qDebug()<<"dragMoveEvent";
//#endif

    if (event->mimeData()->hasFormat("playlistItem")) {
        qDebug() << "playlistTable::dragMoveEvent(playlistItem)";
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    if (event->mimeData()->hasFormat("libraryItem")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void PlaylistTable::dropEvent(QDropEvent *event) {
    qDebug()<<"PlaylistTable::dropEvent()";

    if (event->mimeData()->hasFormat("playlistItem")) {
        // this is when we re-arrange items in the playlist
        QPoint dropPos = event->pos();
        int dropRow = QTableView::indexAt(dropPos).row();
        QByteArray itemData = event->mimeData()->data("playlistItem");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QList<int> itemRowList;
        while (!dataStream.atEnd()) {
            int itemRow;
            dataStream >> itemRow;
            itemRowList << itemRow;

#if DEBUG_PLAYLISTVIEW
            qDebug() << "Decoded mimeData: " << itemRow;
#endif
        }
        qSort(itemRowList);
        int offset = dropRow - itemRowList.back();
#if DEBUG_PLAYLISTVIEW
        qDebug()<<"dropRow is " << dropRow;
#endif
        PlaylistModel *model = static_cast<PlaylistModel*>(QTableView::model());
        model->swapSong(dropRow, itemRowList, offset);
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    if (event->mimeData()->hasFormat("libraryItem")) {
        //QPoint dropPos = event->pos();
        //int dropRow = QTableView::indexAt(dropPos).row();
        QByteArray itemData = event->mimeData()->data("libraryItem");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QList<QHash<QString, QString> > itemList;
        QHash<QString, QString> hash;
        while (!dataStream.atEnd()) {
            dataStream >> hash;
            itemList << hash;
        }
        PlaylistModel *model = static_cast<PlaylistModel*>(QTableView::model());
        model->addMediaList(itemList);
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}
