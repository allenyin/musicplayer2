#include "libraryModel.h"
#include <assert.h>
#include <QMimeData>
#include <QtWidgets>
#include <QSqlQuery>
#include <QFileInfo>
#include <QUrl>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <sstream>

LibraryModel::LibraryModel(QObject *parent) : QAbstractItemModel(parent) {
    u = new Util();
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        QMessageBox msgBox;
        msgBox.setText("Unable to load database, Library needs the SQLITE driver");
        msgBox.exec();
    }
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err, "Database initialization failed");
        return;
    }
    
    err = populateModel();
    if (err.type() != QSqlError::NoError) {
        showError(err, "Populating model failed");
        return;
    }

}

LibraryModel::~LibraryModel() {
    delete rootItem;    
    db.close();
}

QSqlError LibraryModel::initDb() {
    
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("AAMusicPlayer_library.db3");
    //db.setDatabaseName(":memory:"); // not persistent yet
    if (!db.open()) {
        return db.lastError();
    }

    QStringList tables = db.tables();
    // not persistent yet
    if (tables.contains("MUSICLIBRARY", Qt::CaseInsensitive)) {
        return QSqlError();
    }

    QSqlQuery q(db);
    q.prepare("CREATE TABLE IF NOT EXISTS MUSICLIBRARY(id integer primary key, absFilePath varchar(200) UNIQUE, fileName varchar, Title varchar, Artist varchar, Album varchar, Length int)");
    if (!q.exec()) {
        // error if table creation not successfull
        qDebug() << "Table creation error?";
        return q.lastError();
    }

    return QSqlError();
}

QSqlError LibraryModel::populateModel() {
    qDebug() << "Populate the Model!";
    
    // make root
    QSqlQuery q(db);
    if (!q.exec(QLatin1String("SELECT DISTINCT Artist FROM MUSICLIBRARY ORDER BY Artist ASC"))) {
        return q.lastError();
    }
    rootItem = new TreeItem(QHash<QString, QString>(), TreeItem::ROOT);

    // populate the artist and their song nodes one by one
    int artistCount = 0;
    while (q.next()) {
        QString Artist = q.value(0).toString();
        beginInsertRows(QModelIndex(), artistCount, artistCount);
        QHash<QString, QString> hash;
        hash["Artist"] = Artist;
        rootItem->addChild(TreeItem::ARTIST, hash);
        endInsertRows();

        // populate the song nodes
        QSqlQuery q2(db);
        if (!q2.exec(QString("SELECT COUNT(*) FROM MUSICLIBRARY WHERE Artist='%1'").arg(Artist))) {
            return q2.lastError();
        }
        q2.next();
        int songCount = q2.value(0).toInt();
        qDebug() << "In populateModel, artist=" << Artist << " songcount=" << songCount;
        QModelIndex artistIndex = index(artistCount,0);
        beginInsertRows(artistIndex, 0, songCount-1);
        if (!q2.exec(QString("SELECT absFilePath, Title FROM MUSICLIBRARY WHERE Artist='%1' ORDER BY Title ASC").arg(Artist))) {
            return q2.lastError();
        }
        while (q2.next()) {
            QHash<QString, QString> hash;
            hash["absFilePath"] = q2.value(0).toString();
            hash["Title"] = q2.value(1).toString();
            rootItem->child(artistCount)->addChild(TreeItem::SONG, hash);
        }
        item_counts[Artist] = songCount;
        endInsertRows();
        artistCount++;
    }
    return QSqlError();
}

void LibraryModel::showError(const QSqlError &err, const QString msg) {
    QMessageBox msgBox;
    msgBox.setText(msg + " Error with database: " + err.text());
    msgBox.exec();
}


bool LibraryModel::addEntry(QSqlQuery &q, const QString &absFilePath, const QString &fileName,
        const QString &title, const QString &artist, const QString &album, const int length) {
    q.bindValue(":absFilePath", absFilePath);
    q.bindValue(":fileName", fileName);
    q.bindValue(":Title", title);
    q.bindValue(":Artist", artist);
    q.bindValue(":Album", album);
    q.bindValue(":Length", length);
    if (!q.exec()) {
        qDebug() << "Error at addEntry() " <<  q.lastError();
        return false;
    }

    if (!item_counts.contains(artist)) {
        QHash<QString, QString> hash;
        hash["Artist"] = artist;
        //updateLibrary(QModelIndex());
        fetchMore(QModelIndex());
        return true;
    } else {
        // do i need to do updateLibrary here???
        item_counts[artist] += 1;
        return true;
    }
    return false;
}

// Protected methods
int LibraryModel::rowCount(const QModelIndex &parent) const {
    //qDebug() << "In rowCount:";
    //qDebug() << "QModelIndex &parent is: " << parent << ", row=" << parent.row() << " col=" << parent.column();
    TreeItem *parentItem = getItem(parent);
    return parentItem->ChildCount();
}

int LibraryModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    //qDebug() << "In columnCount():";
    //qDebug() << "QModelIndex &parent is: " << parent << ", row=" << parent.row() << " col=" << parent.column();
    return rootItem->columnCount();
}

QModelIndex LibraryModel::index(int row, int column, const QModelIndex &parent) const {
    //qDebug() << "In index():";
    //qDebug() << "QModelIndex &parent is: " << parent << ", row=" << parent.row() << " col=" << parent.column();

    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }

    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}

QModelIndex LibraryModel::parent(const QModelIndex &index) const {
    //qDebug() << "In parent():";
    //qDebug() << "QModelIndex &index is: " << index << ", row=" << index.row() << " col=" << index.column();

    if (!index.isValid()) {
        return QModelIndex();
    }
    
    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();
    if (parentItem == rootItem) {
        return QModelIndex();
    }
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const {
    //qDebug() << "In data():";
    //qDebug() << "QModelIndex &index is: " << index << ", row=" << index.row() << " col=" << index.column();
    
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    TreeItem *item = getItem(index);
    return item->data();
}

bool LibraryModel::hasChildren(const QModelIndex &parent) const {
    TreeItem *parentItem = getItem(parent);
    //qDebug() << "Calling hasChildren on node type=" << parentItem->getItemType();
    switch (parentItem->getItemType()) {
        case TreeItem::ROOT:
            return item_counts.size() > 0;
        case TreeItem::ARTIST:
            return true;
        case TreeItem::SONG:
            return false;
        default:
            return false;
    }
}

TreeItem *LibraryModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return rootItem;
}

/*
bool LibraryModel::canFetchMore(const QModelIndex &parent) const {
    TreeItem *parentItem = getItem(parent);
    switch (parentItem->getItemType()) {
        case TreeItem::ROOT:
            //qDebug() << "canFetchMore ROOT";
            //qDebug() << "item_counts: " << item_counts.size() << " childCount()" << parentItem->childCount();
            return (item_counts.size() > parentItem->childCount()) ? true : false;
        case TreeItem::ARTIST: {
            QString Artist = parentItem->getItemData()["Artist"];
            qDebug() << "canFetchMore Artist=" << Artist;
            qDebug() << "item_counts[" << Artist << "]=" << item_counts[Artist];
            return (item_counts[Artist] > parentItem->childCount());
                               }
        case TreeItem::SONG:
            return false;
        default:
            return false;
    }
}
*/

/*
void LibraryModel::fetchMore(const QModelIndex &parent) {
    // we are assumping that we don't have too many items here, s
    // updating all isn't super bad.
    TreeItem *parentItem = getItem(parent);
    // if we need to display more artists. 
    if (parentItem->getItemType() == TreeItem::ROOT) {
        qDebug() << "fetchMore(ROOT)";
        int remainder = item_counts.size() - (parentItem->childCount());
        int itemsToFetch = qMin(100, remainder);
        qDebug() << "item_counts.size()=" << item_counts.size();
        qDebug() << "itemsToFetch=" << itemsToFetch;
        qDebug() << "item_counts.size()+itemsToFetch-1" << item_counts.size()+itemsToFetch-1;
        QSqlQuery q(db);
        if (!q.exec(QLatin1String("SELECT DISTINCT Artist FROM MUSICLIBRARY ORDER BY Artist ASC"))) {
            qDebug() << "At fetchMore (ROOT): " << q.lastError();
            return;
        }
        beginInsertRows(parent, 0, item_counts.size()+itemsToFetch-1);
        while (q.next()) {
            QString Artist = q.value(0).toString();
            if (!item_counts.contains(Artist)) {
                QHash<QString, QString> hash;
                hash["Artist"] = Artist;
                parentItem->addChild(TreeItem::ARTIST, hash);
                item_counts[Artist] = 1;
            }
        }
        parentItem->sortChildren();
        qDebug()<<"fetchMore endInsertRows()";
        endInsertRows();
    }

    // if we need to display more songs.
    if (parentItem->getItemType() == TreeItem::ARTIST) {
        // check if the number of children=mysql table results.
        QString Artist = parentItem->getItemData()["Artist"];
        int remainder = item_counts[Artist] - parentItem->childCount();
        int itemsToFetch = qMin(100, remainder);
        QSqlQuery q(db);
        if (!q.exec(QString("SELECT Title, absFilePath  FROM MUSICLIBRARY WHERE Artist='%1' ORDER BY Title ASC").arg(Artist))) {
            qDebug() << "At fetchMore(ARTIST) " << q.lastError();
            return;
        }
        beginInsertRows(parent, item_counts[Artist], item_counts[Artist]+itemsToFetch-1);
        while (q.next()) {
            QString absFilePath = q.value(1).toString();
            if (!parentItem->getItemData().contains(absFilePath)) {
                // add the extra item
                QString Title = q.value(0).toString();
                QHash<QString, QString> hash;
                hash["Title"] = Title;
                hash["absFilePath"] = absFilePath;
                parentItem->addChild(TreeItem::SONG, hash);
                item_counts[Artist] += 1;
            }
        }
        parentItem->sortChildren();
        qDebug()<<"fetchMore endInsertRows()";
        endInsertRows();
    }
}
*/

void LibraryModel::addFromDir(const QString & dir) {
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
            continue;
        }
        if (fileInfo.isDir() && fileInfo.isReadable() && !fileInfo.isHidden()) {
            addFromDir(fileInfo.canonicalFilePath());
        }
        QString suffix = fileInfo.completeSuffix();
        if (suffix == "mp3" || suffix == "ogg" || suffix == "raw" || suffix == "wav" || suffix == "wma" || suffix == "mpg") {
            addMusicFromFile(fileInfo);
        }
    }
    qDebug() << "Finishing importing from folder";
}

void LibraryModel::addMusicFromPlaylist(const QString absFilePath) {
    // slot used to add music files that was added to playlist by loading them directly
    QFileInfo fileInfo(absFilePath);
    addMusicFromFile(fileInfo);
}


bool LibraryModel::addMusicFromFile(QFileInfo & fileInfo) {
    // return true if insertion successfull,
    // false if not, or if there's duplicate already.
    QString absFilePath = fileInfo.canonicalFilePath();
    QString fileName = fileInfo.fileName();
    QString title, artist, album;
    int length = 0;

    QByteArray byteArray = absFilePath.toUtf8();
    const char* cString = byteArray.constData();
    TagLib::FileRef f(cString);
    if (f.isNull()) {
        qDebug() << "Can't read file's tags!";
        return false;
    }
    if (!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        title = QString::fromStdString(tag->title().toCString(true));
        title = title.isEmpty() ? fileName : title;
        artist = QString::fromStdString(tag->artist().toCString(true));
        artist = artist.isEmpty() ? "Unknown" : artist;
        album = QString::fromStdString(tag->album().toCString(true));
        album = album.isEmpty() ? "Unknown" : album;
    }
    assert(f.audioProperties());
    if (!f.isNull() && f.audioProperties()) {
        TagLib::AudioProperties *properties = f.audioProperties();
        length = properties->length();
    }
    return addEntryToModel(absFilePath, fileName, title, artist, album, length);
}

bool LibraryModel::addEntryToModel(QString &absFilePath, QString &fileName, QString &title,
                                   QString &artist, QString &album, int length) {
    // insert entry to database
    QSqlQuery q(db);
    if (q.exec(QString("INSERT INTO MUSICLIBRARY(absFilePath, fileName, Title, Artist, Album, Length) VALUES ('%1', '%2', '%3', '%4', '%5', %6)")
                .arg(absFilePath).arg(fileName).arg(title).arg(artist).arg(album).arg(length))) {
        // entry inserted successfully, check if there are items in the model already
        if (!item_counts.contains(artist)) {
            // insert artist node if doesn't exist
            QList<QString> keys = item_counts.keys();
            keys.append(artist);
            qSort(keys);
            int newArtistIdx = keys.indexOf(artist);
            //qDebug() << "Inserting new artist, artist=" << artist;
            //qDebug() << "Sorted keys including artist: " << keys;
            //qDebug() << "newArtistIdx is: " << newArtistIdx;
            beginInsertRows(QModelIndex(), newArtistIdx, newArtistIdx);
            QHash<QString, QString> hash;
            hash["Artist"] = artist;
            rootItem->insertChild(newArtistIdx, TreeItem::ARTIST, hash);
            item_counts[artist] = 0;
            endInsertRows();
        }
        // insert song node by:
        // find the artistNode
        int artistIndex = rootItem->findChildIndex(artist);
        QModelIndex artistModelIndex = index(artistIndex,0);
        TreeItem *artistNode = rootItem->child(artistIndex);
        // find where to insert the songNode
        int songIndex;
        if (artistNode->ChildCount() == 0) {
            songIndex = 0;
        }
        else {
            QList<QString> existingSongs = artistNode->childrenData();
            existingSongs.append(title);
            songIndex = existingSongs.indexOf(title);
        }
        // insert the songNode
        beginInsertRows(artistModelIndex, songIndex, songIndex);
        QHash<QString, QString> hash;
        hash["Title"] = title;
        hash["absFilePath"] = absFilePath;
        artistNode->insertChild(songIndex, TreeItem::SONG, hash);
        item_counts[artist]++;
        endInsertRows();
        return true;
    }
    qDebug() << "Error@ addEntryToModel executing query: " << q.lastError();
    return false;
}

void LibraryModel::playlistMetaDataChange(QHash<QString, QString> newHash) {
    // metadata has been changed in playlist
    // delete the database entry associated with item.
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT Artist, Length FROM MUSICLIBRARY WHERE absFilePath='%1'").arg(newHash["absFilePath"]))) {
        qDebug() << "Error SELECT Artist in SLOT:playlistMetaDataChange() - Executing query: " << q.lastError();
        return;
    }
    q.next();
    QString oldArtist = q.value(0).toString();
    int oldLength = q.value(1).toInt();
    if (!q.exec(QString("DELETE FROM MUSICLIBRARY WHERE absFilePath='%1'").arg(newHash["absFilePath"]))) {
        qDebug() << "Error DELETING old entry in SLOT:playlistMetaDataChange() - Executing query: " << q.lastError();
        return;
    }
    // delete the node associated with it from library
    if (removeSongNode(oldArtist, newHash["absFilePath"])) {
        // successfully removed song node
        // add new node from database
        if (addEntryToModel(newHash["absFilePath"], newHash["fileName"], newHash["Title"], newHash["Artist"],
                            newHash["Album"], oldLength)) {
            // new itme added successfully
            return;
        }
        qDebug() << "Error in SLOT:playlistMetaDataChange() - adding new entry failed!";
    }
    qDebug() << "Error in SLOT:playlistMetaDataChange() - removing old song node failed!";
}

bool LibraryModel::removeSongNode(const QString &artist, const QString &absFilePath) {
    // find the artist node
    int artistIndex = rootItem->findChildIndex(artist);
    QModelIndex artistModelIndex = index(artistIndex, 0);
    TreeItem *artistNode = rootItem->child(artistIndex);
    
    // find the song node
    int songNode = artistNode->findChildIndex(absFilePath);
    
    // remove the song node
    beginRemoveRows(artistModelIndex, songNode, songNode);
    artistNode->removeChild(songNode);
    item_counts[artist]--;
    endRemoveRows();
    
    // remove artist node if it no longer contains any songs
    if (item_counts[artist] == 0) {
        beginRemoveRows(QModelIndex(), artistIndex, artistIndex);
        rootItem->removeChild(artistIndex);
        item_counts.remove(artist);
        endRemoveRows();
    }
    return true;
}

QHash<QString, QString> LibraryModel::getSongInfo(const QModelIndex idx) const {
    QHash<QString, QString> hash;
    TreeItem *item = getItem(idx);
    QString absFilePath = item->getItemData()["absFilePath"];

    // query database
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT fileName, Title, Artist, Album, Length FROM MUSICLIBRARY WHERE absFilePath='%1'").arg(absFilePath))) {
        qDebug() << "Error at getSongInfo() - Executing query: " << q.lastError();
    }
    q.next();
    //qDebug() << "Here: " << q.lastError();
    hash["absFilePath"] = absFilePath;
    hash["fileName"] = q.value(0).toString();
    hash["Title"] = q.value(1).toString();
    hash["Artist"] = q.value(2).toString();
    hash["Album"] = q.value(3).toString();
    hash["Length"] = u->convert_length_format(q.value(4).toInt());
    return hash;
}

QList<QHash<QString, QString> > LibraryModel::getArtistSongInfo(const QModelIndex idx) const{
    QList<QHash<QString, QString> > hashList;
    TreeItem *item = getItem(idx);
    // Query database to get all songs by this artist
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT absFilePath, fileName, Title, Artist, Album, Length from MUSICLIBRARY WHERE Artist='%1' ORDER BY Title ASC").arg(item->data().toString()))) {
        qDebug() << "Error at getArtistSongInfo(() - Executing query: " << q.lastError();
    }
    while (q.next()) {
        QHash<QString, QString> hash;
        hash["absFilePath"] = q.value(0).toString();
        hash["fileName"] = q.value(1).toString();
        hash["Title"] = q.value(2).toString();
        hash["Artist"] = q.value(3).toString();
        hash["Album"] = q.value(4).toString();
        hash["Length"] = u->convert_length_format(q.value(5).toInt());
        hashList.append(hash);
    }
    return hashList;
}

QMimeData *LibraryModel::mimeData(const QModelIndexList &indexes) const {
    qDebug() << "Calling libraryModel mimeData";
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QString header = "libraryItem";
    stream << header; // mimeData header

    QModelIndex index;
    TreeItem *item;
    foreach(index, indexes) {
        if (index.isValid()) {
            item = getItem(index);
            if (item->getItemType() == TreeItem::ARTIST) {
                // do stuff
                QList<QHash<QString, QString> > songList = getArtistSongInfo(index);
                QHash<QString, QString> hash;
                foreach(hash, songList) {
                    stream << hash;
                }
             }
            else if (item->getItemType() == TreeItem::SONG) {
                // do stuff
                QHash<QString, QString> song = getSongInfo(index);
                stream << song;
            }
        }
    }
    mimeData->setData("myMediaItem", encodedData);
    qDebug() << "Called mimeData with indexes=" << indexes;
    return mimeData;
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    else {
        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
    }
}
        

