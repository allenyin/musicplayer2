######################################################################
# Automatically generated by qmake (3.0) Wed Nov 12 23:21:40 2014
######################################################################

TEMPLATE = app
TARGET = music_player2
INCLUDEPATH += .
QT += xml\
      multimedia \
      multimediawidgets \
      widgets
LIBS += -ltag

# Input
HEADERS += player.h playercontrols.h playlistmodel.h playlistTable.h mainWindow.h debug.h
SOURCES += main.cpp player.cpp playercontrols.cpp playlistmodel.cpp playlistTable.cpp mainWindow.cpp
