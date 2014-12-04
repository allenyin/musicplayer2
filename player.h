#pragma once
#include "debug.h"
#include "util.h"
#include "playlistmodel.h"
#include "playercontrols.h"
#include "playlistTable.h"

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>

class QAbstractItemView;
class QLabel;
class QMediaPlayer;
class QModelIndex;
class QPushButton;
class QSlider;
class QAudioProbe;

class PlaylistModel;
class PlaylistTable;
class PlaylistProxyModel;

class Player : public QWidget {
    Q_OBJECT

public:
    Player(QWidget *parent=0);
    ~Player();

    // getters
    PlaylistModel *model();

signals:
    // no signals so far
    void changeTitle(QString);

private slots:
    void open(); 
    void addSongFromLibrary(const QHash<QString, QString> hash);
    void addArtistFromLibrary(const QList<QHash<QString, QString> > hashList);

    /* For Mediaplayer signals
     * durationChanged: Change of total playback time in ms of current media.
     * positionChanged: Change of value in current playback position.
     * metaDataChanged: Change of media object's meta-data (from qMediaObject);
     */
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void metaDataChanged();
    
    // Player control needing extra handling
    void previousClicked();

    // Player control for playlists
    void play();
    void seek(int seconds);
    void jump(const QModelIndex &index);
    void playlistPositionChanged(int currentItem);
    void next();
    void curMediaRemoved(int newCurMediaIdx);
    void mediaAvailable();
    void stop();
    void clearPlaylist();
    void setRepeatOne(bool);
    void setRepeatAll(bool);
    void setShuffle(bool);

    // General Player status slots
    void statusChanged(QMediaPlayer::MediaStatus status);
    void bufferingProgress(int progress);
    void audioAvailableChanged(bool available);

    void displayErrorMessage();

    // playlist management
    void savePlaylist();
    void changePlaylistLabel(QString absFilePath);

private:
    void setTrackInfo(const QString &info);
    void setStatusInfo(const QString &info);
    void handleCursor(QMediaPlayer::MediaStatus status);
    void updateDurationInfo(qint64 currentInfo);

    QMediaPlayer *player;
    QLabel *coverLabel;
    QSlider *slider;
    QLabel *labelDuration;
    QLabel *curPlaylistLabel;
    PlaylistModel *playlistModel;
    PlaylistTable *playlistView;
    QString trackInfo;
    QString statusInfo;
    qint64 duration;
};
    
