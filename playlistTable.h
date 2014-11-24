#pragma once
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
    virtual void contextMenuEvent(QContextMenuEvent *e);
    // for drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:

signals:
    // None right now.

public slots:
};
