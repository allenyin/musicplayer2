#include "playlistmodel.h"
#include <QColor>
#include <QBrush>
#include <QMimeData>

PlaylistModel::PlaylistModel(QObject *parent) 
    : QAbstractTableModel(parent){
    columns = 4;
    m_data = QList<QHash<QString, QString> >();
    curMediaIdx = -1;
    finishedPlaylist = false;
    u = new Util();
    mode = NORMAL;
}

PlaylistModel::~PlaylistModel() {
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    return (!parent.isValid()) ? m_data.size() : 0;
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
        qDebug() << "playlistModel::setData()";
        emit(playlistMetaDataChange(m_data[row]));
        return true;
    }
    return false;
}


Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList PlaylistModel::mimetypes() const {
    QStringList types;
    types << "myMediaItem";
    return types;
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const {
    //qDebug() << "PlaylistModel::mimeData() called with indexes=" << indexes;
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QModelIndex index = indexes[0];
    int row = -1;
    QString header = "playlistItem";
    stream << header;  // mimeData header;
    
    foreach(index, indexes) {
        if (index.isValid()) {
#if DEBUG_PLAYLIST
            qDebug()<< "row for index is: " << row;
#endif
            if (index.row() != row) {
                row = index.row();
                stream << row;
            }
        }
    }
    mimeData->setData("myMediaItem", encodedData);
#if DEBUG_PLAYLIST
    qDebug()<< "Called mimeData with indexes=" << indexes;
#endif
    return mimeData;
}

void PlaylistModel::swapSong(int to, QList<int> fromList, int offset) {
    //moving songs down
    if (to == -1) {
#if DEBUG_PLAYLIST
        qDebug() << "Moving to end";
#endif
        for (int i=0; i < fromList.size(); i++) {
            m_data.append(m_data[fromList[0]]);
            m_data.removeAt(fromList[0]);
            if (curMediaIdx == fromList[i]) {
                curMediaIdx = (m_data.size()-1)-(fromList.size()-1)+i;
            }
        }
    }
    if (offset > 0) {
#if DEBUG_PLAYLIST  
        qDebug() << "Moving down";
#endif
        for (int i=0; i < fromList.size(); i++) {
            m_data.move(fromList[0], to);
            if (curMediaIdx == fromList[i]) {
                curMediaIdx = to-(fromList.size()-1)+i;
            }
        }
    }

    if (offset < 0 && to>=0) {
        // moving songs up
        if (to == 0) {
#if DEBUG_PLAYLIST 
            qDebug() << "Moving to beginning";
#endif
            m_data.prepend(m_data[fromList[0]]);
            m_data.removeAt(fromList[0]+1);
            if (curMediaIdx == fromList[0]) {
                curMediaIdx = 0;
            }
            for (int i=1; i < fromList.size(); i++) {
                m_data.move(fromList[i], 1);
                if (curMediaIdx == fromList[i]) {
                    curMediaIdx = i;
                }
            }
        }
        else {
#if DEBUG_PLAYLIST
            qDebug() << "Moving up";
#endif
            for (int i=0; i < fromList.size(); i++) {
                m_data.move(fromList[i], to+i);
                if (curMediaIdx == fromList[i]) {
                    curMediaIdx = to+i;
                }
            }
        }
    }
    emit(dataChanged(index(0,0), index(m_data.size()-1, columns)));
}

// open() button uses this
void PlaylistModel::addMedia(const QStringList& fileNames) {
    // append media to end of m_data
    int start = m_data.size();
    foreach(QString const &path, fileNames) {
        QHash<QString, QString> hash;
        u->get_metaData(path, hash);
        if (!hash.empty()) {
            beginInsertRows(QModelIndex(), start, start);
            m_data.append(hash);
            start = start+1;
            emit(mediaAddedToPlaylist(hash["absFilePath"]));
            endInsertRows();
        }
    }
    if (curMediaIdx < 0 && start > 0) {
        curMediaIdx = 0;
        emit(mediaAvailable());
    }
}

// library doubleclick uses this
void PlaylistModel::addMedia(const QHash<QString, QString> libraryItem) {
    beginInsertRows(QModelIndex(), m_data.size(), m_data.size()); 
    m_data.append(libraryItem);
    endInsertRows();
    if (curMediaIdx < 0) {
        curMediaIdx = 0;
        emit(mediaAvailable());
    }
}

void PlaylistModel::addMediaList(const QList<QHash<QString, QString> > libraryItemList) {
    QHash<QString, QString> item;
    foreach(item, libraryItemList) {
        addMedia(item);
    }
}

void PlaylistModel::removeMedia(int start, int end) {
    beginRemoveRows(QModelIndex(), start, end);
    for (int i=0; i < end-start+1; i++) {
        m_data.removeAt(start);
        if (curMediaIdx == start+i) {
            curMediaIdx = (m_data.size()>0) ? 0 : -1;
            emit(curMediaRemoved(curMediaIdx));
        }
    }
    endRemoveRows();
    if (m_data.size() == 0) {
        curMediaIdx = -1;
        emit(curMediaRemoved(curMediaIdx));
    }
}

const QMediaContent PlaylistModel::setCurMedia(int row){
    // set current media to row
    if (row >= 0 && row < m_data.size()) {
        curMediaIdx = row;
        QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
        emit(currentIndexChanged(curMediaIdx));
        return url;
    }
    else {
        return QMediaContent();
    }
}

const QMediaContent PlaylistModel::nextMedia(){
    // set current media to the next entry, when called after the previous one finished playing
    // and returns the media content.
    finishedPlaylist = false;
    if (m_data.size() > 0) {
        if (curMediaIdx == m_data.size()-1) {
            finishedPlaylist = true;
        }
        if ( mode & REPEAT1 ) {
            // mode is repeat 1
            curMediaIdx = curMediaIdx;
         }
        else if (mode & SHUFFLE) {
            curMediaIdx = qrand() % m_data.size();
        }
        else {
            if (finishedPlaylist) {
                curMediaIdx = 0;
            }
            else {
                curMediaIdx++;
            }
        }
        QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
        emit(currentIndexChanged(curMediaIdx));
        return url;
    }
    return QMediaContent();
}

const QMediaContent PlaylistModel::pressNextMedia() {
    finishedPlaylist = false;
    if (m_data.size() > 0) {
        if (mode & SHUFFLE) {
            curMediaIdx = qrand() % m_data.size();
        }
        else {
            curMediaIdx = (curMediaIdx+1) % m_data.size();
        }
        QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
        emit(currentIndexChanged(curMediaIdx));
        return url;
    }
    return QMediaContent();
}

const QMediaContent PlaylistModel::currentMedia() {
    if (curMediaIdx >= 0) {
        qDebug() << "Getting current media";
        QUrl url = QUrl::fromLocalFile(m_data[curMediaIdx]["absFilePath"]);
        return url;
    }
    else {
        return QMediaContent();
    }
}

const QMediaContent PlaylistModel::previousMedia() {
    // set and return the previous entry
    if (m_data.size() > 0) {
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
    return QMediaContent();
}

int PlaylistModel::getCurMediaIdx() const{
    return curMediaIdx;
}

QModelIndex PlaylistModel::getCurMediaModelIndex() const {
    return index(curMediaIdx, 0);
}

int PlaylistModel::getColumns() const {
    return columns;
}

bool PlaylistModel::keepPlaying() const {
    // keep playing if not mode repeatall or shuffle or repeat1
    return !(finishedPlaylist && (mode == NORMAL));
}

void PlaylistModel::setMode(int newMode, bool checked) {
    if (checked) {
        // set mode
        mode |= newMode;
    }
    else {
        mode &= ~newMode;
    }
}

void PlaylistModel::clear() {
    // delete all the playlist entries and rest curMediaIdx;
    beginRemoveRows(QModelIndex(), 0, m_data.size()-1);
    m_data.clear();
    endRemoveRows();
    curMediaIdx = -1;
}

const QString PlaylistModel::getCurAlbumArtist() const {
    //qDebug() << "getCurAlbumArtist(): idx=" << curMediaIdx;
    if (curMediaIdx >= 0) {
        return QString("%1 - %2")
            .arg(m_data[curMediaIdx]["Album"])
            .arg(m_data[curMediaIdx]["Artist"]);
    }
    return QString();
}

const QString PlaylistModel::getCurTitle() const {
    //qDebug() << "getCurTitle(): idx=" << curMediaIdx;
    if (curMediaIdx >= 0) {
        return m_data[curMediaIdx]["Title"];
    }
    return QString();
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


