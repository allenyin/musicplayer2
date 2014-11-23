#include "playlistmodel.h"
#include <sstream>
#include <iomanip>
#include <QFileInfo>
#include <QUrl>
#include <QMediaPlaylist>

PlaylistModel::PlaylistModel(QObject *parent) 
    : QAbstractTableModel(parent), m_playlist(NULL) {
    columns = 4;
    connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(changeMetaData(QModelIndex)));
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
        return Qt::ItemIsEnabled;
    }

    if (index.column() == 3) {
        // clicking on length doesn't do anything
        return QAbstractTableModel::flags(index);
    }

    else {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        int row = index.row();
        QHash<QString, QString> h = m_data.value(row);
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

QMediaPlaylist *PlaylistModel::playlist() const {
    return m_playlist;
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

}
