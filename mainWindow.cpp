#include "mainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("AAMusicPlayer"));
    
    // set up player/playlist area
    player = new Player();

    // set up library
    library = new Library();

    // central Widget
    centralWidget = new QWidget();
    
    setupWidgets();
    //setupMenus();
}

MainWindow::~MainWindow() {
    delete player;
    delete library;
    delete centralWidget;
}

void MainWindow::setupWidgets() {
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
}
