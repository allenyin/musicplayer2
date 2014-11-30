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

LibraryModel::LibraryModel(QObject *parent) : QAbstractItemModel(parent) {
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
    
    // for iniital testing.
    // addTestEntries();
    
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
    //db.setDatabaseName("AAMusicPlayer_library.db3");
    db.setDatabaseName(":memory:"); // not persistent yet
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

    // add the separate artist nodes to root
    while (q.next()) {
        QString Artist = q.value(0).toString();
        QHash<QString, QString> hash;
        hash["Artist"] = Artist;
        rootItem->addChild(TreeItem::ARTIST, hash);
        // count how many songs per artist
        QSqlQuery q2(db);
        if (!q2.exec(QString("SELECT COUNT(*) FROM MUSICLIBRARY WHERE Artist='%1'").arg(Artist))) {
            return q2.lastError();
        }
        
        q2.next();
        int songCount = q2.value(0).toInt();
        qDebug() << "In populateModel, artist=" << Artist << " songcount=" << songCount;
        item_counts[Artist] = songCount;
    }
    return QSqlError();
}

void LibraryModel::showError(const QSqlError &err, const QString msg) {
    QMessageBox msgBox;
    msgBox.setText(msg + " Error with database: " + err.text());
    msgBox.exec();
}

void LibraryModel::addTestEntries() {
    QSqlQuery q(db);
    if (!q.prepare("INSERT INTO MUSICLIBRARY(absFilePath, fileName, Title, Artist, Album, Length) "
              "VALUES (:absFilePath, :fileName, :Title, :Artist, :Album, :Length)")) {
        qDebug() << "Error at preoparing query: " << q.lastError();
    }

    addEntry(q, "/home/file1", "file1", "song1", "singer1", "ablum1", 121);
    addEntry(q, "/home/file2", "file2", "song2", "singe2", "album2", 122);
    addEntry(q, "/home/file3", "file3", "song3", "singer3", "album3", 123);
    addEntry(q, "/home/file4", "file4", "song4", "singer4", "album4", 124);
    addEntry(q, "/home/file5", "file5", "song5", "singer5", "album5", 125);

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
        qDebug() << q.lastError();
        return false;
    }

    if (!item_counts.contains(artist)) {
        QHash<QString, QString> hash;
        hash["Artist"] = artist;
        rootItem->addChild(TreeItem::ARTIST, hash);
    }
    item_counts[artist] += 1;
    return false;
}

// Protected methods
int LibraryModel::rowCount(const QModelIndex &parent) const {
    //qDebug() << "In rowCount:";
    //qDebug() << "QModelIndex &parent is: " << parent << ", row=" << parent.row() << " col=" << parent.column();
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
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

bool LibraryModel::canFetchMore(const QModelIndex &parent) const {
    TreeItem *parentItem = getItem(parent);
    switch (parentItem->getItemType()) {
        case TreeItem::ROOT:
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

void LibraryModel::fetchMore(const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    // if we need to display more artists. 
    if (parentItem->getItemType() == TreeItem::ROOT) {
        int remainder = item_counts.size()-(parentItem->childCount());
        int itemsToFetch = qMin(100, remainder);
        QSqlQuery q(db);
        if (!q.exec(QLatin1String("SELECT DISTINCT Artist FROM MUSICLIBRARY ORDER BY Artist ASC"))) {
            qDebug() << q.lastError();
            return;
        }
        beginInsertRows(parent, item_counts.size(), item_counts.size()+itemsToFetch-1);
        while (q.next()) {
            QString Artist = q.value(0).toString();
            if (!item_counts.contains(Artist)) {
                QHash<QString, QString> hash;
                hash["Artist"] = Artist;
                parentItem->addChild(TreeItem::ARTIST, hash);
                item_counts[Artist] = 1;
            }
        }
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
            qDebug() << q.lastError();
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
        endInsertRows();
    }
}

void LibraryModel::addFromDir(const QString & dir) {
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        //qDebug() << "Current file: " << it.next();
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
            continue;
        }
        if (fileInfo.isDir() && fileInfo.isReadable() && !fileInfo.isHidden()) {
            addFromDir(fileInfo.canonicalFilePath());
        }
        QString suffix = fileInfo.completeSuffix();
        if (suffix == "mp3" || suffix == "ogg" || suffix == "raw" || suffix == "wav" || suffix == "wma") {
            beginInsertRows(QModelIndex(), item_counts.size(), 1);
            addMusicFile(fileInfo);
            endInsertRows();
        }
    }
    qDebug() << "Finishing importing from folder";
}

bool LibraryModel::addMusicFile(QFileInfo & fileInfo) {
    // return true if insertion successfull,
    // false if not, or if there's duplicate already.
    QString absFilePath = fileInfo.canonicalFilePath();
    QString fileName = fileInfo.fileName();
    QString title, artist, album;
    int length;

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
    QSqlQuery q(db);
    if (!q.prepare("INSERT INTO MUSICLIBRARY(absFilePath, fileName, Title, Artist, Album, Length) "
              "VALUES (:absFilePath, :fileName, :Title, :Artist, :Album, :Length)")) {
        qDebug() << "Error at addMusicFile() - preoparing query: " << q.lastError();
    }
    return addEntry(q, absFilePath, fileName, title, artist, album, length);
}
