#pragma once
#include <QAbstractTableModel>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

class QMediaPlaylist;

class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT

public:
    int columns;
    // constructor
    PlaylistModel(QObject *parent = 0);
    
    // compulsory inherited methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation Orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    //bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    //bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    QMediaPlaylist *playlist() const;
    void setPlaylist(QMediaPlaylist *playlist);

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
};

