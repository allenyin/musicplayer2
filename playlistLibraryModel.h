#pragma once
#include <QtSql/QtSql>
#include <QStandardItemModel>
#include <QModelIndex>

class PlaylistLibraryModel : public QStandardItemModel {
    Q_OBJECT

public:
    PlaylistLibraryModel(QWidget *parent = 0);
    ~PlaylistLibraryModel();
    void addFromDir(const QString &dir); 
    void loadPlaylist(const QModelIndex &idx); 
    void deletePlaylist(const QModelIndex &idx);
    void changePlaylistName(const QModelIndex &idx, QString newName);

protected:

private slots:
    void addToModelAndDB(QFileInfo fileInfo);
    void refresh();
    void addNewlyCreatedPlaylist(QString absFilePath, QString fileName);

signals:
    void playlistItemAdded();   // connect to proxymodel for sorting
    void loadPlaylist(QString);

private:
    void getImportDirs();
    QSqlError initDb();
    QSqlError populateModel();
    QSqlError populateFromDirs();
    void addToModelOnly(QFileInfo &fileInfo);
    void showError(const QSqlError &err, const QString msg);

    QSqlDatabase db;
    QList<QString> importDirs;

};
