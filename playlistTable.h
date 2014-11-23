#pragma once
#include <QTableView>
#include <QMouseEvent>

class PlaylistTable : public QTableView {
    Q_OBJECT

public:
    PlaylistTable(QWidget* parent = 0);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void contextMenuEvent(QContextMenuEvent *e);

signals:
    // None right now.
};
