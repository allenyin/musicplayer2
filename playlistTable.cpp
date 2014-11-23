#include "playlistTable.h"
#include <stdio.h>
#include <iostream>

PlaylistTable::PlaylistTable(QWidget* parent) : QTableView(parent) {
}

void PlaylistTable::mouseDoubleClickEvent(QMouseEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    std::cout << "Double click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
    emit QTableView::activated(clickIdx);
}

void PlaylistTable::mouseReleaseEvent(QMouseEvent* e) {
/*    if (e->button() == Qt::LeftButton) {
        // Use base handler
        std::cout << "Single left click here!\n";
        QTableView::mouseReleaseEvent(e);
    }
    else {
        QTableView::mouseReleaseEvent(e);
    }*/
}

void PlaylistTable::contextMenuEvent(QContextMenuEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    std::cout << "Right click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
}
