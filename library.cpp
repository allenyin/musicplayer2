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
    view = new QTreeView(this);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setAnimated(false);
    view->setModel(libraryModel);
    view->resizeColumnToContents(0);
    view->setItemsExpandable(true);
    view->setHeaderHidden(true);
    //view->setUniformRowHeights(true);

    QBoxLayout *displayLayout = new QVBoxLayout;
    displayLayout->addWidget(label);
    displayLayout->addWidget(view);
    setLayout(displayLayout);

}

Library::~Library() {
    delete libraryModel;
    delete view;
    delete label;
}

LibraryModel* Library::model() {
    return libraryModel;
}
