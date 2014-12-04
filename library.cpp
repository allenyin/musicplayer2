#include "debug.h"
#include "library.h"
#include "libraryModel.h"
#include "playlistLibraryModel.h"
#include "playlistLibraryView.h"
#include <QAbstractItemView>
#include <QtWidgets>

Library::Library(QWidget *parent) : QWidget(parent) {
    
    libraryLabel = new QLabel(this);
    libraryLabel->setText("Media Library");
    playlistLabel = new QLabel(this);
    playlistLabel->setText("Playlists");

    // library model
    libraryModel = new LibraryModel(this);

    // library view
    libraryView = new LibraryView(this);
    libraryView->setModel(libraryModel);

    // playlist-library model
    plModel = new PlaylistLibraryModel(this);

    // playlist-library view
    plView = new PlaylistLibraryView(this);
    plView->setModel(plModel);

    // layout
    QBoxLayout *displayLayout = new QVBoxLayout;
    displayLayout->addWidget(libraryLabel);
    displayLayout->setSpacing(2);
    displayLayout->addWidget(libraryView);
    displayLayout->setSpacing(2);
    displayLayout->addWidget(playlistLabel);
    displayLayout->setSpacing(2);
    displayLayout->addWidget(plView);
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

PlaylistLibraryModel* Library::model_pl() const {
    return plModel;
}

PlaylistLibraryView* Library::view_pl() const {
    return plView;
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
