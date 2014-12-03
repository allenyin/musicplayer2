#pragma once
#include "debug.h"
#include "util.h"
#include <QDebug>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QMediaContent>

class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT

public:
    // flags for different playback mode
    const static int NORMAL = 1;    // 0001
    const static int REPEAT1 = 2;   // 0010
    const static int REPEATALL = 4; // 0100
    const static int SHUFFLE = 8;   // 1000

    // constructor
    PlaylistModel(QObject *parent = 0);
    ~PlaylistModel();

    // getters
    int getCurMediaIdx() const;
    QModelIndex getCurMediaModelIndex() const;
    int getColumns() const;
    bool keepPlaying() const;
    
    // compulsory inherited methods
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation Orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    // needed for drag and drop
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimetypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    void swapSong(int to, QList<int> fromlist, int offset);
    
    // playlist management and integration with player.
    void addMedia(const QStringList& fileNames);
    void addMedia(const QHash<QString, QString>  libraryItem);
    void addMediaList(const QList<QHash<QString, QString> > libraryItemList);
    void removeMedia(int start, int end);
    const QMediaContent setCurMedia(int row);
    const QMediaContent nextMedia(void);
    const QMediaContent pressNextMedia(void);
    const QMediaContent previousMedia(void);
    const QMediaContent currentMedia(void);
    void setMode(int newMode, bool checked);
    void clear();
    const QString getCurAlbumArtist(void) const;
    const QString getCurTitle(void) const;

private slots:

    void beginRemoveItems(int start, int end);
    void endRemoveItems();
    void changeItems(int start, int end);
    void changeMetaData(QModelIndex index);
    void libraryMetaDataChanged(int dataType, QString arg1, QString arg2);

signals:
   void mediaAddedToPlaylist(QString);
   void mediaAvailable();
   void playlistMetaDataChange(QHash<QString, QString>);  // should be caught by library to update database.
   void currentIndexChanged(int);
   void curMediaRemoved(int);
   void resetPlaylist();

private:
    /* m_data is a list of dictionary containing the following,
     * indexed by the column
     *          (fileName: bla)
     *          (Title: bla)
     *          (Artist: bla)
     *          (Album: bla)
     *          (Length: bla)
     * QMap keeps the order of the dictionary keys.
     */
    QList<QHash<QString, QString> > m_data;
    int curMediaIdx;
    Util *u;
    int mode;
    int columns;
    bool finishedPlaylist;


};

