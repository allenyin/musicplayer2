#include "player.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Player player;
    player.show();
    return app.exec();
}
