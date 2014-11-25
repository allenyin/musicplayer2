#include "playlistmodel.h"
#include <sstream>
#include <iomanip>
#include <QFileInfo>
#include <QUrl>
#include <QMediaPlaylist>
#include <QColor>
#include <QBrush>
#include <QMimeData>

PlaylistModel::PlaylistModel(QObject *parent) 
    : QAbstractTableModel(parent), m_playlist(NULL) {
    columns = 4;
    connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(changeMetaData(QModelIndex)));
}

PlaylistModel::~PlaylistModel() {
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    return (m_playlist && !parent.isValid()) ? m_playlist->mediaCount() : 0;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const {
    return (!parent.isValid()) ? columns : 0;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const {
    return (m_playlist && !parent.isValid()
            && row >= 0 && row < m_playlist->mediaCount()
            && column >= 0 && column < columns) ?
            createIndex(row, column) : QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &child) const {
    Q_UNUSED(child);

    return QModelIndex();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_playlist->mediaCount() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        const QHash<QString, QString> h = m_data.at(index.row());
        switch(index.column()) {
            case 0:
                // title
                return h["Title"];
            case 1:
                // artist
                return h["Artist"];
            case 2:
                // album
                return h["Album"];
            case 3:
                // length
                return h["Length"];
            default:
               return QVariant();
        } 
    }

    if (role == Qt::EditRole) {
        const QHash<QString, QString> h = m_data.at(index.row());
        switch(index.column()) {
            case 0:
                // title
                return h["Title"];
            case 1:
                // artist
                return h["Artist"];
            case 2:
                // album
                return h["Album"];
            case 3:
                // length
                return h["Length"];
            default:
               return QVariant();
        } 
    }

    return QVariant();
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Title");
            case 1:
                return tr("Artist");
            case 2:
                return tr("Album");
            case 3:
                return tr("Length");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }

    if (index.column() == 3) {
        // clicking on length doesn't do anything
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
    }

    else {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;

    }
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        int row = index.row();
        switch(index.column()) {
        case 0:
            // title
            m_data[row]["Title"] = value.toString();
            emit(QAbstractItemModel::dataChanged(index, index));
            break;
        case 1:
            // artist
            m_data[row]["Artist"] = value.toString();
            emit(QAbstractItemModel::dataChanged(index, index));
            break;
        case 2:
            // album
            m_data[row]["Album"] = value.toString();
            emit(QAbstractItemModel::dataChanged(index, index));
            break;
        default:
            break; 
        } 
        return true;
    }
    return false;
}


Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList PlaylistModel::mimetypes() const {
    QStringList types;
    types << "playlistItem";
    return types;
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QModelIndex index = indexes[0];
    int row = -1;
    
    foreach(index, indexes) {
        if (index.isValid()) {
            qDebug()<< "row for index is: " << row;
            if (index.row() != row) {
                row = index.row();
                stream << row;
            }
        }
    }
    mimeData->setData("playlistItem", encodedData);
    qDebug()<< "Called mimeData with indexes=" << indexes;
    return mimeData;
}

void PlaylistModel::swapSong(int to, QList<int> fromlist, int offset) {
    // swap songs with playlist row indicated by the arguments.
    disconnect_playlist();
    qDebug() << "Before swapping, playlist is: ";
    printPlaylist();
    if (fromlist.first() >= 0) {
        m_playlist->removeMedia(fromlist.first(), fromlist.last());
    }

    foreach(int from, fromlist) {
        // then we swap the entry in m_data, then add it back to playlist
        qDebug() << "m_data item to move: " << m_data[from];
        if (to == -1) {
            m_data.move(from, m_data.size()-1);
        }
        else {
            m_data.move(from, from+offset);
        }
    }
    //addDataToPlaylist(to, fromlist.size());
    remakePlaylist();
    qDebug() << "After swapping, playlist is: ";
    printPlaylist();
    reconnect_playlist();
}

void PlaylistModel::addDataToPlaylist(int row, int count) {
    // given the row of a media inside m_data, add the media item
    // into the m_playlist.
    for (int i=0; i<count; i++) {
        QUrl url = QUrl::fromLocalFile(m_data[row+count]["absFilePath"]);
        bool result = m_playlist->insertMedia(row+count, url);
    }
    qDebug() << "playlist is now, after adding from data: ";
}

void PlaylistModel::remakePlaylist() {
    m_playlist->clear();
    for (int i=0; i < m_data.size(); i++) {
        QUrl url = QUrl::fromLocalFile(m_data[i]["absFilePath"]);
        m_playlist->addMedia(url);
    }
}

void PlaylistModel::printPlaylist() {
    for (int i=0; i<m_playlist->mediaCount(); i++) {
        qDebug() << m_playlist->media(i).canonicalUrl();
    }
}


QMediaPlaylist *PlaylistModel::playlist() const {
    return m_playlist;
}

void PlaylistModel::disconnect_playlist() {
    disconnect(m_playlist, SIGNAL(mediaAboutToBeInserted(int,int)), this, SLOT(beginInsertItems(int,int)));
    disconnect(m_playlist, SIGNAL(mediaInserted(int,int)), this, SLOT(endInsertItems()));
    disconnect(m_playlist, SIGNAL(mediaAboutToBeRemoved(int,int)), this, SLOT(beginRemoveItems(int,int)));
    disconnect(m_playlist, SIGNAL(mediaRemoved(int,int)), this, SLOT(endRemoveItems()));
    disconnect(m_playlist, SIGNAL(mediaChanged(int,int)), this, SLOT(changeItems(int,int)));
}

void PlaylistModel::reconnect_playlist() {
    beginResetModel();
    connect(m_playlist, SIGNAL(mediaAboutToBeInserted(int,int)), this, SLOT(beginInsertItems(int,int)));
    connect(m_playlist, SIGNAL(mediaInserted(int,int)), this, SLOT(endInsertItems()));
    connect(m_playlist, SIGNAL(mediaAboutToBeRemoved(int,int)), this, SLOT(beginRemoveItems(int,int)));
    connect(m_playlist, SIGNAL(mediaRemoved(int,int)), this, SLOT(endRemoveItems()));
    connect(m_playlist, SIGNAL(mediaChanged(int,int)), this, SLOT(changeItems(int,int)));
    endResetModel();
}

void PlaylistModel::setPlaylist(QMediaPlaylist *playlist) {
    if (m_playlist) {
        disconnect(m_playlist, SIGNAL(mediaAboutToBeInserted(int,int)), this, SLOT(beginInsertItems(int,int)));
        disconnect(m_playlist, SIGNAL(mediaInserted(int,int)), this, SLOT(endInsertItems()));
        disconnect(m_playlist, SIGNAL(mediaAboutToBeRemoved(int,int)), this, SLOT(beginRemoveItems(int,int)));
        disconnect(m_playlist, SIGNAL(mediaRemoved(int,int)), this, SLOT(endRemoveItems()));
        disconnect(m_playlist, SIGNAL(mediaChanged(int,int)), this, SLOT(changeItems(int,int)));
    }

    beginResetModel();
    m_playlist = playlist;

    if (m_playlist) {
        connect(m_playlist, SIGNAL(mediaAboutToBeInserted(int,int)), this, SLOT(beginInsertItems(int,int)));
        connect(m_playlist, SIGNAL(mediaInserted(int,int)), this, SLOT(endInsertItems()));
        connect(m_playlist, SIGNAL(mediaAboutToBeRemoved(int,int)), this, SLOT(beginRemoveItems(int,int)));
        connect(m_playlist, SIGNAL(mediaRemoved(int,int)), this, SLOT(endRemoveItems()));
        connect(m_playlist, SIGNAL(mediaChanged(int,int)), this, SLOT(changeItems(int,int)));
    }

    endResetModel();
}

void PlaylistModel::beginInsertItems(int start, int end) {
    beginInsertRows(QModelIndex(), start, end);
    for (int row=start; row < end+1; row++) {
        QHash<QString, QString> hash;
        hash["fileName"] = QString("");
        hash["Title"] = QString("");
        hash["Artist"] = QString("");
        hash["Album"] = QString("");
        hash["Length"] = QString("");
    }
    insert_start = start;
    insert_end = end;
}

void PlaylistModel::endInsertItems() {
    for (int row=insert_start; row<insert_end+1; row++) {
        QUrl location = m_playlist->media(row).canonicalUrl();
        QString path = location.path();
        QHash<QString, QString> hash;
        m_data.insert(row, hash);
        m_data[row]["absFilePath"] = QFileInfo(path).absoluteFilePath();
        m_data[row]["fileName"] = QFileInfo(path).fileName();
        get_metaData(row, path);
    }
    endInsertRows();
}

void PlaylistModel::beginRemoveItems(int start, int end) {
    beginRemoveRows(QModelIndex(), start, end);
    for (int row=0; row < end+1; row++) {
       m_data.removeAt(row);
    } 
}

void PlaylistModel::endRemoveItems() {
    endInsertRows();
}

void PlaylistModel::changeItems(int start, int end) {
    emit dataChanged(index(start,0), index(end,columns));
}

void PlaylistModel::get_metaData(int row, QString path) {
    QByteArray byteArray = path.toUtf8();
    const char* cString = byteArray.constData();
    TagLib::FileRef f(cString);
    if (!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        QString title = QString::fromStdString(tag->title().toCString(true));
        QString artist = QString::fromStdString(tag->artist().toCString(true));
        QString album = QString::fromStdString(tag->album().toCString(true));
        m_data[row]["Title"] = title.isEmpty() ? m_data[row]["fileName"] : title;
        m_data[row]["Artist"] = artist.isEmpty() ? QString("Unknown") : artist;
        m_data[row]["Album"] = album.isEmpty() ? QString("Unknown") : album;
    }
    if (!f.isNull() && f.audioProperties()) {
        TagLib::AudioProperties *properties = f.audioProperties();
        int seconds = properties->length() % 60;
        int minutes = (properties->length() - seconds)/60;
        std::stringstream ss;
        ss << minutes << ":" << std::setfill('0')<<std::setw(2)<<seconds;
        QString l = QString::fromStdString(ss.str());
        m_data[row]["Length"] = l;
    }
}

void PlaylistModel::changeMetaData(QModelIndex index) {
    int row = index.row();
    int col = index.column();
    // get path of the associated file
    QUrl location = m_playlist->media(row).canonicalUrl();
    QString path = location.path();
    QByteArray byteArray = path.toUtf8();
    const char* cString = byteArray.constData();
    
    TagLib::FileRef f(cString);
    if (!f.isNull() && f.tag()) {
        switch (col) {
            case 0: {
                // change title
                TagLib::String title = TagLib::String(m_data[row]["Title"].toUtf8().constData());
                f.tag()->setTitle(title);
                f.file()->save();
                break;
                    }
            case 1: {
                // change artist
                TagLib::String artist = TagLib::String(m_data[row]["Artist"].toUtf8().constData());
                f.tag()->setArtist(artist);
                f.file()->save();
                break;
                    }
            case 2: {
                // change album
                TagLib::String album = TagLib::String(m_data[row]["Album"].toUtf8().constData());
                f.tag()->setAlbum(album);
                f.file()->save();
                break;
                    }
            default:
                break;
        }
    }
    return;
}



