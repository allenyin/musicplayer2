#pragma once
#include <QAbstractTableModel>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <QModelIndex>

class QMediaPlaylist;

class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT

public:
    int columns;
    // constructor
    PlaylistModel(QObject *parent = 0);
    ~PlaylistModel();
    
    // compulsory inherited methods
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation Orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    QMediaPlaylist *playlist() const;
    void setPlaylist(QMediaPlaylist *playlist);
    // needed for drag and drop
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimetypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    void swapSong(int to, QList<int> fromlist, int offset);

private slots:
    void beginInsertItems(int start, int end);
    void endInsertItems();
    void beginRemoveItems(int start, int end);
    void endRemoveItems();
    void changeItems(int start, int end);
    void changeMetaData(QModelIndex index);
   
private:
    QMediaPlaylist *m_playlist;
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
    int insert_start;
    int insert_end;
    void get_metaData(int row, QString path);
    /* Used for drag and drop, temporary disconnect so we don't
       invoke the get_metaData method.
   */
    void disconnect_playlist();
    void reconnect_playlist();
    void addDataToPlaylist(int row, int count); 
    void printPlaylist();
    void remakePlaylist();
};

