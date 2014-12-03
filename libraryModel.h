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
    void getImportDirs();
    void addImportDirs(const QString &dir);
    // for file imports
    void addFromDir(const QString &dir, bool addToImportDirs=true); 
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

    // for dynamic fetching...kinda obsolete
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    // drag and drop support with playlist, and editing library entries
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    void changeMetaData(int field, QString absFilePath, QString value);

private slots:
    void addMusicFromPlaylist(const QString absFilePath);
    void playlistMetaDataChange(QHash<QString,QString> newHash);
    void refreshLibrary();

signals:
    void libraryMetaDataChanged(int, QString, QString);
    
private:
    QSqlError initDb();
    QSqlError populateModel();
    QSqlError populateFromDirs();
    void addArtistAndSongs(int artistCount, QString Artist, QList<QHash<QString, QString> > &validSongs);
    void showError(const QSqlError &err, const QString msg);
    /*
     * addEntry is adding an entry to database.
     * addMusicFile is create a database entry from an actual file.
     */
    bool addEntry(QSqlQuery &q, const QString &absFilePath, const QString &fileName,
                  const QString &title, const QString &artist, const QString &album, const int length);
    bool addMusicFromFile(QFileInfo &fileInfo);
    bool addEntryToModel(QString &absFilePath, QString &fileName, QString &title, QString &artist, QString &album, int length);
    bool removeEntryFromModel(QString &absFilePath, QString &artist);
    bool removeSongNode(const QString &artist, const QString &absFilePath);
    bool batchMoveSongNodes(QString newArtist, TreeItem *oldArtistNode, const QModelIndex &oldArtistIndex, int numSongs);
    bool insertArtistNode(QString newArtist);
    Util *u;
    TreeItem *rootItem;
    QSqlDatabase db;
    QHash<QString, int> item_counts;
    QList<QString> importDirs;
};
