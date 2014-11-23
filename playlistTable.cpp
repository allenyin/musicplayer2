#include "playlistTable.h"
#include "tableDelegate.h"
#include <stdio.h>
#include <iostream>
#include <QApplication>

PlaylistTable::PlaylistTable(QWidget* parent) : QTableView(parent) {
    ignoreNextRelease = false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(emitClicked()));
}

PlaylistTable::~PlaylistTable() {
    delete timer;
}

/*
void PlaylistTable::mouseDoubleClickEvent(QMouseEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    std::cout << "Double click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
    emit QTableView::activated(clickIdx);
}
*/

/*
void PlaylistTable::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        // Use base handler
        std::cout << "Single left click here!\n";
        QTableView::mouseReleaseEvent(e);
    }
    else {
        QTableView::mouseReleaseEvent(e);
    }
}*/

void PlaylistTable::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        my_event = e;
        if (!ignoreNextRelease) {
            timer->start(QApplication::doubleClickInterval());
            blockSignals(true);
            QTableView::mouseReleaseEvent(e);
            blockSignals(false);
        }
        ignoreNextRelease = false;
    }
}

void PlaylistTable::mouseDoubleClickEvent(QMouseEvent *e) {
    ignoreNextRelease = true;
    timer->stop();
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    std::cout << "Double click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
    emit QTableView::activated(clickIdx);
}

void PlaylistTable::contextMenuEvent(QContextMenuEvent* e) {
    QPoint clickPos = e->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    std::cout << "Right click at (" << clickPos.x() << ", " << clickPos.y() << ")\n";
    std::cout << "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")\n";
}

void PlaylistTable::emitClicked() {
    timer->stop();
}
