#pragma once

//#include <QAbstractItemModel>
#include <QAbstractTableModel>

class QMediaPlaylist;

class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column
    {
        Title = 0,
        ColumnCount = 3
    };

    PlaylistModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    
    // tablemodel specifics
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);


    QMediaPlaylist *playlist() const;
    void setPlaylist(QMediaPlaylist *playlist);


    private slots:
        void beginInsertItems(int start, int end);
        void endInsertItems();
        void beginRemoveItems(int start, int end);
        void endRemoveItems();
        void changeItems(int start, int end);

    private:
        QMediaPlaylist *m_playlist;
        QMap<QModelIndex, QVariant> m_data;
};
