#pragma once
#include "plSortFilterProxyModel.h"
#include "playlistLibraryModel.h"
#include <QTableView>
#include <QMouseEvent>

class PLSortFilterProxyModel;
class PlaylistLibraryModel;

class PlaylistLibraryView : public QTableView {
    Q_OBJECT

public:
    PlaylistLibraryView(QWidget* parent = 0);
    ~PlaylistLibraryView();
    void setModel(QAbstractItemModel *model);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    /*
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    */


signals:


public slots:

private:
    PLSortFilterProxyModel *proxyModel;
    PlaylistLibraryModel *myModel;


};

