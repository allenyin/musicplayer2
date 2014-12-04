#include "playlistLibraryModel.h"
#include <QStandardItemModel>
#include <assert.h>
#include <QtWidgets>
#include <QSqlQuery>
#include <QFileInfo>
#include <QMessageBox>

PlaylistLibraryModel::PlaylistLibraryModel(QWidget *parent) : QStandardItemModel(0,2,parent) {
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        QMessageBox msgBox;
        msgBox.setText("Unable to load database, Library needs the SQLITE driver");
        msgBox.exec();
        return;
    }
    
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err, "PlaylistLibrary: Database initialization failed");
        return;
    }
    
    getImportDirs();

    // populate model with playlists from database, while deleting invalid database entries.
    err = populateModel();
    if (err.type() != QSqlError::NoError) {
        showError(err, "PlaylistLibrary: Populating model failed");
        return;
    }

    // update the librayr and database from the preferred dirs, in case new files are added
    err = populateFromDirs();
    assert(err.type() == QSqlError::NoError);
}

PlaylistLibraryModel::~PlaylistLibraryModel() {
    db.close();
}

void PlaylistLibraryModel::getImportDirs() {
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
    qDebug() << "PlaylistLibraryModel::getImportDirs(): finished reading import dirs: " << importDirs;
}

QSqlError PlaylistLibraryModel::initDb() {
    db = QSqlDatabase::addDatabase("QSQLITE", "playlistConnection");
    db.setDatabaseName("AAMusicPlayer_library.db3");
    if (!db.open()) {
        qDebug() << "PlaylistLibraryModel::initDb(): Can't open database!";
        return db.lastError();
    }

    QStringList tables = db.tables();
    if (tables.contains("PLAYLISTLIBRARY", Qt::CaseInsensitive)) {
        return QSqlError();
    }

    QSqlQuery q(db);
    if (!q.exec(QString("CREATE TABLE IF NOT EXISTS PLAYLISTLIBRARY(id integer primary key, absFilePath varchar(500) UNIQUE)"))) {
        qDebug() << "PlaylistLibraryModel::initDb(): Can't create table!";
        return q.lastError();
    }
    return QSqlError();
}

QSqlError PlaylistLibraryModel::populateModel() {
    qDebug() << "Populate playlist-library model from database";
    QSqlQuery q(db);    // fetching playlist entries
    QSqlQuery q2(db);   // deleting invalid playlist entries
    if (!q.exec(QString("SELECT absFilePath FROM PLAYLISTLIBRARY"))) {
        qDebug() << "PlaylistLibraryModel::populateModel() failed at fetching playlists from database";
        return q.lastError();
    }
    while (q.next()) {
        QFileInfo fileInfo = QFileInfo(q.value(0).toString());
        if (!fileInfo.exists()) {
            // remove nonexistent playlist file
            if (!q2.exec(QString("DELETE FROM PLAYLISTLIBRARY WHERE absFilePath='%1'").arg(q.value(0).toString()))) {
                qDebug() << "PlaylistLibraryModel::populateModel() failed@Remvoing invalid DB entry with absFilePath=" << q.value(0).toString();
                return q.lastError();
            }
            // remove invalid model item if there exists...
            QList<QStandardItem *> invalidItems = findItems(q.value(0).toString(), Qt::MatchExactly, 1);
            if (!invalidItems.isEmpty()) {
               QStandardItem *item;
               foreach(item, invalidItems) {
                   int row = item->row();
                   removeRows(row,1);
               }
            }
            continue;
        }
        // otherwise add this to model
        addToModelOnly(fileInfo);
    }
    return QSqlError();
}

QSqlError PlaylistLibraryModel::populateFromDirs() {
    QString dir;
    foreach(dir, importDirs) {
        addFromDir(dir);
    }
    return QSqlError();
}

void PlaylistLibraryModel::addFromDir(const QString &dir) {
    qDebug() << "PlaylistLibraryModel()::addFromDir()";
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
        if (fileInfo.suffix() == "m3u") {
            qDebug() << "PlaylistLibraryModel()::addFromDir: Found a playlist file";
            addToModelAndDB(fileInfo);
        }
    }
}

void PlaylistLibraryModel::addNewlyCreatedPlaylist(QString absFilePath, QString fileName) {
    // only add the database item, need to refresh for the new playlist to show up.
    Q_UNUSED(absFilePath)
    QSqlQuery q(db);
    if (q.exec(QString("INSERT INTO PLAYLISTLIBRARY(absFilePath) VALUES ('%1')").arg(absFilePath))) {
        return;
    }
}

void PlaylistLibraryModel::addToModelAndDB(QFileInfo fileInfo) {
    QString absFilePath = fileInfo.canonicalFilePath();
    qDebug() << "Adding playlist to model and DB, absFilePath is: " << absFilePath;
    QSqlQuery q(db);
    if (q.exec(QString("INSERT INTO PLAYLISTLIBRARY(absFilePath) VALUES ('%1')").arg(absFilePath))) {
        addToModelOnly(fileInfo);
        return;
    }
    qDebug() << "PlaylistLibraryModel::addToModelAndDB(): adding database entry failed: " << q.lastError();
}

void PlaylistLibraryModel::addToModelOnly(QFileInfo &fileInfo) {
    // add the playlist to model only, no database entry
    bool a = insertRow(0);
    qDebug() << "PlaylistLibraryModel::addToModelOnly(): " << fileInfo.canonicalFilePath();
    setData(index(0,0), fileInfo.baseName());
    setData(index(0,1), fileInfo.canonicalFilePath());
    qDebug() << "Insert suceeded? " << a;
    emit(playlistItemAdded());
}

void PlaylistLibraryModel::refresh() {
    qDebug() << "Refreshing PlaylistLibraryModel";
    //clear();
    removeRows(0, rowCount());
    importDirs.clear();
    
    getImportDirs();
    QSqlError err;
    
    // populate model with playlists from database, while deleting invalid database entries.
    err = populateModel();
    if (err.type() != QSqlError::NoError) {
        showError(err, "PlaylistLibrary: Populating model failed");
        return;
    }

    // update the library and database from the preferred dirs, in case new files are added
    err = populateFromDirs();
    assert(err.type() == QSqlError::NoError);
}

void PlaylistLibraryModel::showError(const QSqlError &err, const QString msg) {
    QMessageBox msgBox;
    msgBox.setText(msg + " Error with database: " + err.text());
    msgBox.exec();
}

void PlaylistLibraryModel::loadPlaylist(const QModelIndex &idx) {
    QString absFilePath = data(index(idx.row(),1)).toString();
    emit(loadPlaylist(absFilePath));
}

void PlaylistLibraryModel::deletePlaylist(const QModelIndex &idx) {
    QString absFilePath = data(index(idx.row(),1)).toString();
    // remove node
    removeRows(idx.row(), 1); 
    // remove database item
    QSqlQuery q(db);
    if (!q.exec(QString("DELETE FROM PLAYLISTLIBRARY WHERE absFilePath='%1'").arg(absFilePath))) {
        qDebug() << "PlaylistLibraryModel: deleting playlist failed: " << q.lastError();
        return;
    }
    // remove actual file
    if (!QFile::remove(absFilePath)) {
        qDebug() << "Unable to remove playlist file!";
    }
}

void PlaylistLibraryModel::changePlaylistName(const QModelIndex &idx, QString newName) {
    qDebug() << "Want to change playlistName, which is: " << data(index(idx.row(),0)).toString();

    // get new file name
    QString absFilePath = data(index(idx.row(),1)).toString();
    QFileInfo original(absFilePath);
    QString newPath = original.canonicalPath() + QDir::separator() + newName + ".m3u";

    // change playlist's name
    QFile file(absFilePath);
    file.rename(newPath);
    file.close();
    
    // remove node
    removeRows(idx.row(), 1);

    // delete database entry
    QSqlQuery q(db);
    if (!q.exec(QString("DELETE FROM PLAYLISTLIBRARY WHERE absFilePath='%1'").arg(absFilePath))) {
        qDebug() << "PlaylistLibraryModel: deleting playlist failed: " << q.lastError();
        return;
    }

    QFileInfo reNamed(newPath);
    addToModelAndDB(reNamed);
}
