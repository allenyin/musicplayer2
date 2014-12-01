#include "debug.h"
#include "util.h"
#include "mainWindow.h"
#include <QApplication>
#include <QtGlobal>
#include <QTime>

int main(int argc, char *argv[]) {
    // create seed for random
    QTime time = QTime::currentTime();
    qsrand((uint) time.msec());
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
