#include "playlistTable.h"
#include "tableDelegate.h"
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
    std::cout << "Double click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
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

void PlaylistTable::contextMenuEvent(QContextMenuEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    std::cout << "Right click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
}

void PlaylistTable::dragEnterEvent(QDragEnterEvent *event) {
    qDebug()<<"dragEnterEvent";
    if (event->mimeData()->hasFormat("playlistItem")) {
        QPoint dragPos = event->pos();
        QModelIndex dragIdx = QTableView::indexAt(dragPos);
        qDebug()<<"dragIdx is " << dragIdx;
        event->accept();
    }
    else {
        event->ignore();
    }
}

void PlaylistTable::dragMoveEvent(QDragMoveEvent *event) {
    //qDebug()<<"dragMoveEvent";
    if (event->mimeData()->hasFormat("playlistItem")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void PlaylistTable::dropEvent(QDropEvent *event) {
    qDebug()<<"dropEvent";
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
            qDebug() << "Decoded mimeData: " << itemRow;
        }
        qSort(itemRowList);
        int offset = dropRow - itemRowList.back();
        qDebug()<<"dropRow is " << dropRow;
        PlaylistModel *model = dynamic_cast<PlaylistModel*>(QTableView::model());
        model->swapSong(dropRow, itemRowList, offset);
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}
