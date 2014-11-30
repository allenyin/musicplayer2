#include "mainWindow.h"
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QString>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("AAMusicPlayer"));
    
    // set up player/playlist area
    player = new Player();

    // set up library
    library = new Library();

    // central Widget
    centralWidget = new QWidget();
   
    setupWidgets();
    setupMenus();
}

MainWindow::~MainWindow() {
    delete player;
    delete library;
    delete centralWidget;
    delete fileMenu;
    delete menubar;
    delete exitAction;
    delete importFromFolderAction;
    //delete importFileAction;
}

void MainWindow::setupWidgets() {
    // widgets layout
    QHBoxLayout *hboxLayout = new QHBoxLayout(centralWidget);

    QSizePolicy spLibrary(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    spLibrary.setHorizontalStretch(1);
    library->setSizePolicy(spLibrary);
    hboxLayout->addWidget(library);
    
    QSizePolicy spPlayer(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    spPlayer.setHorizontalStretch(6);
    player->setSizePolicy(spPlayer);
    hboxLayout->addWidget(player);
    setCentralWidget(centralWidget);
    connect(player, SIGNAL(changeTitle(QString)), this, SLOT(setWindowTitle(const QString &)));

    // signal connections.
    connect(library, SIGNAL(addToPlaylist(QHash<QString, QString>)), player, SLOT(addFromLibrary(QHash<QString, QString>)));
}

void MainWindow::setupMenus() {
    menubar = new QMenuBar(this);

    fileMenu = menubar->addMenu(tr("&File"));
    // actions associated with fileMenu:
    // importFromFolderAction
    importFromFolderAction = new QAction(tr("Import from folder"), this);
    fileMenu->addAction(importFromFolderAction);
    connect(importFromFolderAction, SIGNAL(triggered()), this, SLOT(importFromFolder()));
    // exitAction
    exitAction = new QAction(tr("&Exit"), this);
    fileMenu->addAction(exitAction);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::importFromFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Import from folder"),
                          QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    library->model()->addFromDir(dir);
}
