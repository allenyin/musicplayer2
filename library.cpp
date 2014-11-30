#include "debug.h"
#include "library.h"
#include "libraryModel.h"
#include <QAbstractItemView>
#include <QtWidgets>

Library::Library(QWidget *parent) : QWidget(parent) {
    libraryModel = new LibraryModel(this);

    // labels
    label = new QLabel(tr("Library:"));
    
    // view
    libraryView = new LibraryView(this);
    libraryView->setModel(libraryModel);

    // layout
    QBoxLayout *displayLayout = new QVBoxLayout;
    displayLayout->addWidget(label);
    displayLayout->addWidget(libraryView);
    setLayout(displayLayout);

    // signal connections
    connect(libraryView, SIGNAL(activated(QModelIndex)), this, SLOT(addToPlaylist(QModelIndex)));

}

Library::~Library() {
    delete libraryModel;
    delete libraryView;
    delete label;
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
        emit(addToPlaylist(libraryModel->getSongInfo(idx)));
    }
}
