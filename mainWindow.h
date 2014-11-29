#pragma once
#include "debug.h"
#include "util.h"
#include "player.h"
#include "library.h"
#include <QMainWindow>
#include <QHBoxLayout>

class Player;
class QHBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

private slots:

private:
    Player *player;
    Library *library; 
    QWidget *centralWidget;
    void setupWidgets();
    //void setupMenus();

};
