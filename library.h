#pragma once
#include "libraryModel.h"
#include <QWidget>
#include <QTreeView>
#include <QLabel>

class LibraryModel;

class Library : public QWidget {
    Q_OBJECT

public:
    Library(QWidget *parent = 0);
    ~Library();

private:
    LibraryModel *libraryModel;
    QTreeView *view;
    QLabel *label;
};
