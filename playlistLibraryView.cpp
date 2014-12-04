#include "playlistLibraryView.h"
#include "playlistLibraryModel.h"
#include "plSortFilterProxyModel.h"
#include <QHeaderView>
#include <QMimeData>
#include <QDebug>


class PlaylistLibraryModel;

PlaylistLibraryView::PlaylistLibraryView(QWidget* parent) : QTableView(parent) {
    // edit only when F2 is pressed
    setEditTriggers(QAbstractItemView::EditKeyPressed);

    // Appearance
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    resizeColumnToContents(0);
    verticalHeader()->hide();
    horizontalHeader()->hide();

    // enable drag and drop
    setDragEnabled(true);
    setAcceptDrops(false);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setAlternatingRowColors(true);
    //setFrameShape(QFrame::Box);
    proxyModel = new PLSortFilterProxyModel(this);

    // sort the view
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
}

PlaylistLibraryView::~PlaylistLibraryView() {
    delete proxyModel;
}

void PlaylistLibraryView::setModel(QAbstractItemModel *model) {
    myModel = static_cast<PlaylistLibraryModel*>(model);
    QTableView::setModel(proxyModel);
    proxyModel->setSourceModel(myModel);
    proxyModel->sort(0, Qt::AscendingOrder);
    connect(myModel, SIGNAL(playlistItemAdded()), proxyModel, SLOT(sortItems()));
}

void PlaylistLibraryView::mouseDoubleClickEvent(QMouseEvent *event) {
    QPoint clickPos = event->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    myModel->loadPlaylist(proxyModel->mapToSource(clickIdx));
}

void PlaylistLibraryView::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete) {
        QModelIndexList selected = selectedIndexes();
        QModelIndex idx = selected[0];
        myModel->deletePlaylist(proxyModel->mapToSource(idx));
    }
    else {
        QTableView::keyPressEvent(event);
    }

}

