#pragma once
#include "libraryModel.h"
#include "libraryView.h"
#include "playlistLibraryModel.h"
#include "playlistLibraryView.h"
#include <QWidget>
#include <QTreeView>
#include <QLabel>

class LibraryModel;
class LibraryView;
class PlaylistLibraryModel;
class PlaylistLibraryView;

class Library : public QWidget {
    Q_OBJECT

public:
    Library(QWidget *parent = 0);
    ~Library();
    LibraryModel *model() const;
    LibraryView *view() const;
    PlaylistLibraryModel *model_pl() const;
    PlaylistLibraryView *view_pl() const;

private slots:
    void addToPlaylist(QModelIndex idx);

signals:
    void addSongToPlaylist(const QHash<QString, QString> hash);
    void addArtistToPlaylist(const QList<QHash<QString, QString> > hashList);

private:
    QLabel *libraryLabel;
    QLabel *playlistLabel;
    LibraryModel *libraryModel;
    LibraryView *libraryView;
    PlaylistLibraryModel *plModel;
    PlaylistLibraryView *plView;
};
