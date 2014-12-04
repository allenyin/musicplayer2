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
    getImportDirs();    // populate importDirs with preferred music directories.
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        QMessageBox msgBox;
        msgBox.setText("Unable to load database, Library needs the SQLITE driver");
        msgBox.exec();
        return;
    }
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err, "Database initialization failed");
        return;
    }
    
    // populate library from database, while deleting invalid database entries.
    err = populateModel(); 
    if (err.type() != QSqlError::NoError) {
        showError(err, "Populating model failed");
        return;
    }

    // update the library and database from the preferred dirs, in case new files are added.
    err = populateFromDirs();
    assert(err.type() == QSqlError::NoError);
}

LibraryModel::~LibraryModel() {
    delete rootItem;    
    db.close();
}

void LibraryModel::getImportDirs() {
    QFile config_file("AAMusicPlayer_CONFIG.txt");
    if (!config_file.exists()) {
        // create one if not there
        config_file.open(QIODevice::WriteOnly | QIODevice::Text);
        return;
    }
    // read it otherwise
    assert(config_file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&config_file);
    while (!in.atEnd()) {
        importDirs.append(in.readLine());
    }
}

void LibraryModel::addImportDirs(const QString &dir) {
    if (!importDirs.contains(dir)) {
        QFile config_file("AAMusicPlayer_CONFIG.txt");
        assert(config_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
        QTextStream out(&config_file);
        out << dir << "\n";
    }
}
    
QSqlError LibraryModel::initDb() {
    
    db = QSqlDatabase::addDatabase("QSQLITE", "libraryConnection");
    db.setDatabaseName("AAMusicPlayer_library.db3");
    //db.setDatabaseName(":memory:"); // not persistent yet
    if (!db.open()) {
        qDebug() << "initDb(): Can't open database!";
        return db.lastError();
    }

    QStringList tables = db.tables();
    if (tables.contains("MUSICLIBRARY", Qt::CaseInsensitive)) {
        return QSqlError();
    }

    QSqlQuery q(db);
    q.prepare("CREATE TABLE IF NOT EXISTS MUSICLIBRARY(id integer primary key, absFilePath varchar(200) UNIQUE, fileName varchar, Title varchar, Artist varchar, Album varchar, Length int)");
    if (!q.exec()) {
        // error if table creation not successfull
        qDebug() << "Music Table creation error";
        return q.lastError();
    }
    
    return QSqlError();
}


QSqlError LibraryModel::populateModel() {
    qDebug() << "Populate the Model from database";

    // make root
    QSqlQuery q(db);    // query for distinct artists
    QSqlQuery q2(db);   // query for songs with the same artist
    QSqlQuery q3(db);   // query to delete invalid database entries
    QList<QHash<QString, QString> > validSongs;
    if (!q.exec(QString("SELECT DISTINCT Artist FROM MUSICLIBRARY ORDER BY Artist ASC"))) {
        qDebug() << "PopulateModel(): select artist failed!";
        return q.lastError();
    }
    rootItem = new TreeItem(QHash<QString, QString>(), TreeItem::ROOT);

    // populate the artist and song nodes
    int artistCount = 0;
    while (q.next()) {
        validSongs.clear();
        // find artist first
        QString Artist = q.value(0).toString();
        
        // find and check how many of its children are valid.
        if (!q2.exec(QString("SELECT absFilePath, Title FROM MUSICLIBRARY WHERE Artist='%1' ORDER BY Title ASC").arg(Artist))) {
            qDebug() << "PopulateModel(): Selecting SONGS with Artist=" << Artist << " failed!";
            return q2.lastError();
        }
        while (q2.next()) {
            QFileInfo f(q2.value(0).toString());
            if (!f.exists()) {
                // if it doesn't exist, remove database entry
                qDebug() << "Want to remove item!";
                if (!q3.exec(QString("DELETE FROM MUSICLIBRARY WHERE absFilePath='%1'")
                                    .arg(q2.value(0).toString()))) {
                        qDebug() << "PopulateModel(): Removing invalid DB entry with absFilePath=" << q2.value(0).toString() << " failed!";
                        return q.lastError();
                }
                continue;
            }
            // otherwise add this to validSongs list
            QHash<QString, QString> hash;
            hash["absFilePath"] = q2.value(0).toString();
            hash["Title"] = q2.value(1).toString();
            validSongs.append(hash);
        }
        if (validSongs.size() > 0) {
            // if there are valid songs left, add to library
            addArtistAndSongs(artistCount, Artist, validSongs);
            artistCount++;
        }
    }
    return QSqlError();
}

void LibraryModel::addArtistAndSongs(int artistCount, QString Artist, QList<QHash<QString, QString> > &validSongs) {
    // helper for populateModel(), should not be used by other functions
    
    // Add to rootItem at row=artistCount an ARTIST node named "Artist".
    beginInsertRows(QModelIndex(), artistCount, artistCount);
    QHash<QString, QString> hash;
    hash["Artist"] = Artist;
    rootItem->addChild(TreeItem::ARTIST, hash);
    endInsertRows();

    // Add the validSongs as child SONG nodes to the new ARTIST node
    beginInsertRows(index(artistCount,0), 0, validSongs.size()-1);
    foreach(hash, validSongs) {
        rootItem->child(artistCount)->addChild(TreeItem::SONG, hash);
    }
    // update the item_counts entry
    item_counts[Artist] = validSongs.size();
    endInsertRows();
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
    
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        TreeItem *item = getItem(index);
        return item->data();
    }

    return QVariant();
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

QSqlError LibraryModel::populateFromDirs() {
    QString dir;
    foreach(dir, importDirs) {
        addFromDir(dir, false);
    }
    return QSqlError();
}

void LibraryModel::refreshLibrary() {
    // reconstruct the library items and sync database with folder.
    qDebug() << "Refreshing library";
    
    qDebug() << "Clearing library...";
    beginRemoveRows(QModelIndex(), 0, rootItem->ChildCount()-1);
    delete rootItem;
    item_counts.clear();
    endRemoveRows();
    
    qDebug() << "Repopulating library...";
    QSqlError err = populateModel(); 
    if (err.type() != QSqlError::NoError) {
        showError(err, "Populating model failed");
        return;
    }

    // update the library and database from the preferred dirs, in case new files are added.
    err = populateFromDirs();
    assert(err.type() == QSqlError::NoError);
}

void LibraryModel::addFromDir(const QString & dir, bool addToImportDirs) {
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

    // add the given dir to import directory list unless specified
    if (addToImportDirs) {
        qDebug() << "Adding new directory to CONFIG file";
        addImportDirs(dir);
    }
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
        insertArtistNode(artist);
        
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
            qSort(existingSongs);
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

bool LibraryModel::removeEntryFromModel(QString &absFilePath, QString &artist) {
    QSqlQuery q(db);
    if (!q.exec(QString("DELETE FROM MUSICLIBRARY WHERE absFilePath='%1'").arg(absFilePath))) {
        qDebug() << "Error removeEntryFromModel() - Executing query: " << q.lastError();
        return false;
    }
    // delete the node associated with it from library
    return removeSongNode(artist, absFilePath);
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
        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    }
}

bool LibraryModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        if (index.parent() == QModelIndex()) {
            // clicked on an artist node
            QString newArtist = value.toString();
            QString oldArtist = data(index).toString();
            
            // for each song node, get their information
            TreeItem *artistItem = getItem(index);
            QList<QString> absFilePathList;
            TreeItem *item;
            foreach(item, artistItem->getChildItems()) {
                absFilePathList.append(item->getItemData()["absFilePath"]);
            }

            for (int i=0; i < absFilePathList.size(); i++) {
                // change the metadata
                changeMetaData(1, absFilePathList[i], newArtist);
            }
            // update the database entries
            QSqlQuery q(db);
            if (!q.exec(QString("UPDATE MUSICLIBRARY SET Artist='%1' WHERE Artist='%2'").arg(newArtist).arg(oldArtist))) {
                qDebug() << "Error@setData(): Batch updating database entries artist columns failed: " << q.lastError();
                return false;
            }
            
            // move all the nodes over and delete the oldArtist node
            if (batchMoveSongNodes(newArtist, artistItem, index, absFilePathList.size())) {
                emit(libraryMetaDataChanged(1, oldArtist, newArtist));
                return true;
            }
        }
        else {
            // clicked on a song node
            TreeItem *item = getItem(index);
            QString absFilePath = item->getItemData()["absFilePath"];
            
            // change MetaData of the actual file
            changeMetaData(0, absFilePath, value.toString());

            // delete the node and database entry associated.
            QString artist = item->parent()->data().toString();
            if (removeEntryFromModel(absFilePath, artist)) {
                // add the new node
                QFileInfo fileInfo(absFilePath);
                if (addMusicFromFile(fileInfo)) {
                    emit(libraryMetaDataChanged(0, absFilePath, value.toString()));
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

bool LibraryModel::batchMoveSongNodes(QString newArtist, TreeItem *oldArtistNode, const QModelIndex &oldArtistIndex, int numSongs) {
    beginRemoveRows(oldArtistIndex, 0, numSongs-1);
    // move the pointers to children song node to temporary storage.
    QList<TreeItem *> orphans;
    oldArtistNode->getChildItems().swap(orphans); // orphans now contains pointers to the songNodes
    endRemoveRows();
    
    beginRemoveRows(QModelIndex(), oldArtistIndex.row(), oldArtistIndex.row());
    // remove library record for oldArtist
    item_counts.remove(oldArtistNode->data().toString());
    // remove old artist node
    rootItem->removeChild(oldArtistIndex.row());
    endRemoveRows();

    // at this point the orphans' parentItem pointers are invalid,
    // this will be fixed in the next for loop.

    // create the newArtistNode if doesn't exist
    insertArtistNode(newArtist);

    // vars needed in loop to finish connecting the orphans back
    QModelIndex newArtistIdx = index(rootItem->findChildIndex(newArtist),0);
    TreeItem *newArtistNode = rootItem->findChildNode(newArtist);
    TreeItem *item;
    QList<QString> existingSongs;
    int newSongIdx;
    foreach(item, orphans) {
        existingSongs = newArtistNode->childrenData();
        existingSongs.append(item->data().toString());
        qSort(existingSongs);
        newSongIdx = existingSongs.indexOf(item->data().toString());
        beginInsertRows(newArtistIdx, newSongIdx, newSongIdx);
        newArtistNode->insertChildItem(newSongIdx, TreeItem::SONG, item);
        item->setParentItem(newArtistNode);
        item_counts[newArtist]++;
        endInsertRows();
    }
    return true;
}

bool LibraryModel::insertArtistNode(QString newArtist) {
    // insert an Artist node if it doesn't exist already, set item_counts[newArtist] = 0 after
    if (!item_counts.contains(newArtist)) {
        QList<QString> keys = item_counts.keys();
        keys.append(newArtist);
        qSort(keys);
        int newArtistIdx = keys.indexOf(newArtist);
        beginInsertRows(QModelIndex(), newArtistIdx, newArtistIdx);
        QHash<QString, QString> hash;
        hash["Artist"] = newArtist;
        rootItem->insertChild(newArtistIdx, TreeItem::ARTIST, hash);
        item_counts[newArtist] = 0;
        endInsertRows();
        return true;
    }
    return true;
}

void LibraryModel::changeMetaData(int field, QString absFilePath, QString value) {
    // field=0 -> change Title
    // field=1 -> change Artist
    QByteArray byteArray = absFilePath.toUtf8();
    const char* cString = byteArray.constData();
    TagLib::String tagValue = TagLib::String(value.toUtf8().constData());
    TagLib::FileRef f(cString);
    if (!f.isNull() && f.tag()) {
        switch(field) {
            case 0:
                    //change title
                    f.tag()->setTitle(tagValue);
                    break;
            case 1:
                    // change artist
                    f.tag()->setArtist(tagValue);
                    break;
            default:
                    return;
        }
        f.file()->save();
        return;
    }
}
        

