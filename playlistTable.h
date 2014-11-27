#pragma once
#include "debug.h"
#include "util.h"
#include <QTableView>
#include <QMouseEvent>
#include <QTimer>

class PlaylistTable : public QTableView {
    Q_OBJECT

public:
    PlaylistTable(QWidget* parent = 0);
    ~PlaylistTable();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
#if DEBUG_PLAYLISTVIEW 
    virtual void contextMenuEvent(QContextMenuEvent *e);
#endif
    // for drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    // vor deleting items
    virtual void keyPressEvent(QKeyEvent *event);

private:
    //QList<int> getRowOfIndexes(QModelIndexList &idxList);

signals:
    // None right now.

public slots:
};
