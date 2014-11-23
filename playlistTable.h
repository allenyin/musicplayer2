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

private:
    bool ignoreNextRelease;
    QTimer *timer;
    QMouseEvent *my_event;

signals:
    // None right now.

public slots:
    void emitClicked();
};
