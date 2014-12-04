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
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    resizeColumnToContents(0);
    //hideColumn(1);
    verticalHeader()->hide();
    horizontalHeader()->hide();

    // enable drag and drop
    setDragEnabled(true);
    setAcceptDrops(false);

    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setAlternatingRowColors(true);
    setFrameShape(QFrame::Box);
    proxyModel = new PLSortFilterProxyModel(this);
}

PlaylistLibraryView::~PlaylistLibraryView() {
}

void PlaylistLibraryView::setModel(QAbstractItemModel *model) {
    QTableView::setModel(proxyModel);
    proxyModel->setSourceModel(model);
}

void PlaylistLibraryView::mouseDoubleClickEvent(QMouseEvent *event) {
    QPoint clickPos = event->pos();
    QModelIndex clickIdx = QTableView::indexAt(clickPos);
    emit (QTableView::activated(clickIdx));
}

