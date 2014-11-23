#pragma once

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

class Player : public QWidget {
    Q_OBJECT

public:
    Player(QWidget *parent=0);
    ~Player();

signals:
    // no signals so far

private slots:
    void open(); 

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
    void seek(int seconds);
    void jump(const QModelIndex &index);
    void playlistPositionChanged(int);

    // General Player status slots
    void statusChanged(QMediaPlayer::MediaStatus status);
    void bufferingProgress(int progress);
    void audioAvailableChanged(bool available);

    void displayErrorMessage();

    // playlist basic controls
    void addToPlaylist(const QStringList &fileNames);
    //void removeFromPlaylist(const QStringList &fileNames);

private:
    void setTrackInfo(const QString &info);
    void setStatusInfo(const QString &info);
    void handleCursor(QMediaPlayer::MediaStatus status);
    void updateDurationInfo(qint64 currentInfo);

    QMediaPlayer *player;
    QMediaPlaylist *playlist;
    QLabel *coverLabel;
    QSlider *slider;
    QLabel *labelDuration;
    PlaylistModel *playlistModel;
    PlaylistTable *playlistView;
    QString trackInfo;
    QString statusInfo;
    qint64 duration;
};
    
