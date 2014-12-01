#include "debug.h"
#include "library.h"
#include "libraryModel.h"
#include <QAbstractItemView>
#include <QtWidgets>

Library::Library(QWidget *parent) : QWidget(parent) {
    libraryModel = new LibraryModel(this);

    // view
    libraryView = new LibraryView(this);
    libraryView->setModel(libraryModel);

    // layout
    QBoxLayout *displayLayout = new QVBoxLayout;
    displayLayout->addWidget(libraryView);
    setLayout(displayLayout);

    // signal connections
    connect(libraryView, SIGNAL(activated(QModelIndex)), this, SLOT(addToPlaylist(QModelIndex)));

}

Library::~Library() {
    delete libraryModel;
    delete libraryView;
}

LibraryModel* Library::model() const {
    return libraryModel;
}

LibraryView* Library::view() const {
    return libraryView;
}

// slot
void Library::addToPlaylist(const QModelIndex idx) {
    TreeItem *item = libraryModel->getItem(idx);
    //qDebug() << "Clicked item has data: " << item->data();
    if (item->getItemType() == TreeItem::SONG) {
        emit(addSongToPlaylist(libraryModel->getSongInfo(idx)));
    }
    if (item->getItemType() == TreeItem::ARTIST) {
        emit(addArtistToPlaylist(libraryModel->getArtistSongInfo(idx)));
    }
}
