#include "player.h"
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    Player *player = new Player;;
    w.setCentralWidget(player);
    w.show();
    return app.exec();
}
