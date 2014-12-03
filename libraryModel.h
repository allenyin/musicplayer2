#pragma once
#include "debug.h"
#include "util.h"
#include "treeItem.h"
#include <QtSql/QtSql>
#include <QAbstractItemModel>
#include <QModelIndex>

/*
 * QSqlDatabase db;
 * db = QSqlDatabase::addDatabase("QSQLITE");
 * db.setDatabaseName("fileName.db3");
 * db.open();
 */

class LibraryModel : public QAbstractItemModel {
    Q_OBJECT

public:
    LibraryModel(QObject *parent = 0);
    ~LibraryModel();
    // for file imports
    void addFromDir(const QString &dir); 
    TreeItem *getItem(const QModelIndex &index) const;
    QHash<QString, QString> getSongInfo(const QModelIndex idx) const;
    QList<QHash<QString, QString> > getArtistSongInfo(const QModelIndex idx) const;

protected:
    // inherited from QAbstractItemModel
    virtual int rowCount(const QModelIndex &parent) const; 
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    // for dynamic fetching
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    //virtual bool canFetchMore(const QModelIndex &parent) const;
    //virtual void fetchMore(const QModelIndex &parent);

    // drag and drop support with playlist
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void addMusicFromPlaylist(const QString absFilePath);
    void playlistMetaDataChange(QHash<QString,QString> newHash);
    
private:
    QSqlError initDb();
    QSqlError populateModel();
    void addArtistAndSongs(int artistCount, QString Artist, QList<QHash<QString, QString> > &validSongs);
    void showError(const QSqlError &err, const QString msg);
    void addTestEntries();
    /*
     * addEntry is adding an entry to database.
     * addMusicFile is create a database entry from an actual file.
     */
    bool addEntry(QSqlQuery &q, const QString &absFilePath, const QString &fileName,
                  const QString &title, const QString &artist, const QString &album, const int length);
    bool addMusicFromFile(QFileInfo &fileInfo);
    bool addEntryToModel(QString &absFilePath, QString &fileName, QString &title, QString &artist, QString &album, int length);
    bool removeSongNode(const QString &artist, const QString &absFilePath);
    Util *u;
    TreeItem *rootItem;
    QSqlDatabase db;
    QHash<QString, int> item_counts;
};
