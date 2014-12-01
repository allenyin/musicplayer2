#include "libraryView.h"
#include "libraryModel.h"
#include <QAbstractItemView>

class LibraryModel;

LibraryView::LibraryView(QWidget *parent) : QTreeView(parent) {
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAnimated(false);
    resizeColumnToContents(0);
    setItemsExpandable(true);
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    // enable drag and drop
    setDragEnabled(true);
    setAcceptDrops(false);
}

LibraryView::~LibraryView() {
    // nothing to destroy specifically yet
}

void LibraryView::mouseDoubleClickEvent(QMouseEvent* e) {
    QPoint clickPos = e->pos();
    const QModelIndex clickIdx = QTreeView::indexAt(clickPos);
    qDebug()<< "Double click at (" << clickPos.x() << ", " << clickPos.y() << ")";
    qDebug()<< "ModelIndex: (" << clickIdx.row() << ", " << clickIdx.column() << ")";
    emit QTreeView::activated(clickIdx);
}

void LibraryView::dragEnterEvent(QDragEnterEvent *event) {
    qDebug() << "LibraryView: dragEnter";
    if (event->mimeData()->hasFormat("libraryItem")) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

void LibraryView::dragMoveEvent(QDragMoveEvent *event) {
    qDebug() << "LibrarView: dragMove";
    if (event->mimeData()->hasFormat("libraryItem")) {
        event->accept();
    }
    else {
        event->ignore();
    }
}






