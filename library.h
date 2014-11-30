#pragma once
#include "libraryModel.h"
#include "libraryView.h"
#include <QWidget>
#include <QTreeView>
#include <QLabel>

class LibraryModel;
class LibraryView;

class Library : public QWidget {
    Q_OBJECT

public:
    Library(QWidget *parent = 0);
    ~Library();
    LibraryModel *model() const;
    LibraryView *view() const;

private slots:
    void addToPlaylist(QModelIndex idx);

signals:
    void addToPlaylist(const QHash<QString, QString> hash);

private:
    LibraryModel *libraryModel;
    LibraryView *libraryView;
    QLabel *label;
};
