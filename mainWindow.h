#pragma once
#include "debug.h"
#include "util.h"
#include "player.h"
#include "library.h"
#include <QMainWindow>
#include <QHBoxLayout>
#include <QtWidgets>

class Player;
class QHBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

private slots:
    void importFromFolder();
    void about();

private:
    Player *player;
    Library *library; 
    QWidget *centralWidget;
    QMenuBar *menubar;
    QMenu *fileMenu;
    QMenu *aboutMenu;
    QAction *exitAction;
    QAction *importFromFolderAction;
    QAction *refreshLibraryAction;
    QAction *aboutAction;
    void setupWidgets();
    void setupMenus();

};
