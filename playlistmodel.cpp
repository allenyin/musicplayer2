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
    : QAbstractTableModel(parent){
    columns = 4;
    m_data = QList<QHash<QString, QString> >();
    curMediaIdx = -1;
}

PlaylistModel::~PlaylistModel() {
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    return (!parent.isValid()) ? m_data.size() : 0;
    //return (m_playlist && !parent.isValid()) ? m_playlist->mediaCount() : 0;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const {
    return (!parent.isValid()) ? columns : 0;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const {
    return (!parent.isValid()
            && row >= 0 && row < m_data.size() 
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

    if (index.row() >= m_data.size() || index.row() < 0) {
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
        changeMetaData(index);
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
    // if one of the songs being swapped is the current song, the next song
    // will be the one after that song in the new position.
    foreach(int from, fromlist) {
        // then we swap the entry in m_data, then add it back to playlist
        qDebug() << "m_data item to move: " << m_data[from];
        if (to == -1) {
            qDebug() << "move to back";
            m_data.move(from, m_data.size()-1);
            if (curMediaIdx == from) {
                curMediaIdx = m_data.size()-1;
            }
        }
        else {
            qDebug() << "move to other index, from=" << from << " offset=" << offset;
            if (offset < 0) {
                m_data.move(from, from+offset+1);
                if (curMediaIdx == from) {
                    curMediaIdx = from+offset+1;
                }
            }
            else {
                m_data.move(from, from+offset);
                if (curMediaIdx == from) {
                    curMediaIdx = from+offset;
                }
            }
        }
    }
    emit(dataChanged(index(0,0), index(m_data.size()-1, columns)));
}

void PlaylistModel::addMedia(const QStringList& fileNames) {
    // append media to end of m_data
    int start = m_data.size();
    int end = start + fileNames.size()-1;
    beginInsertRows(QModelIndex(), start, end);
    foreach (QString const &path, fileNames) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            QHash<QString, QString> hash;
            m_data.append(hash);
            m_data[start]["absFilePath"] = fileInfo.absoluteFilePath();
            m_data[start]["fileName"] = fileInfo.fileName();
            get_metaData(start, path);
            start = start+1;
        }
    }
    if (curMediaIdx < 0) {
        curMediaIdx = 0;
    }
    endInsertRows();
}

void PlaylistModel::removeMedia(int start, int end) {
    beginRemoveRows(QModelIndex(), start, end);
    for (int i=0; i < end-start+1; i++) {
        m_data.removeAt(start);
    }
    endRemoveRows();
}

const QUrl PlaylistModel::setCurMedia(int row){
    // set current media to row
    if (row >= 0 && row < m_data.size()) {
        curMediaIdx = row;
        QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
        emit(currentIndexChanged(curMediaIdx));
        return url;
    }
    else {
        return QUrl();
    }
}

const QUrl PlaylistModel::nextMedia(){
    // set current media to the next entry and return url
    if (curMediaIdx+1 == m_data.size()) {
        curMediaIdx = 0;
    }
    else {
        curMediaIdx = curMediaIdx+1;
    }
    QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
    emit(currentIndexChanged(curMediaIdx));
    return url;
}

const QUrl PlaylistModel::previousMedia() {
    // set and return the previous entry
    if (curMediaIdx == 0) {
        curMediaIdx = m_data.size()-1;
    }
    else {
        curMediaIdx = curMediaIdx-1;
    }
    QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
    emit(currentIndexChanged(curMediaIdx));
    return url;
}

int PlaylistModel::getCurMediaIdx() const{
    return curMediaIdx;
}

QString PlaylistModel::getCurAlbumArtist() const {
    return QString("%1 - %2")
        .arg(m_data[curMediaIdx]["Album"])
        .arg(m_data[curMediaIdx]["Artist"]);
}

QString PlaylistModel::getCurTitle() const {
    return m_data[curMediaIdx]["Title"];
}




//void PlaylistModel::removeMedia(int start, int end) {
//}

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
    //QUrl location = m_playlist->media(row).canonicalUrl();
    //QString path = location.path();
    QByteArray byteArray = m_data[row]["absFilePath"].toUtf8();
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
