#include "player.h"
#include "playlistmodel.h"
#include "playercontrols.h"
#include "playlistTable.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QAudioProbe>
#include <QMediaMetaData>
#include <QtWidgets>
#include <QHeaderView>

Player::Player(QWidget *parent) :QWidget(parent), coverLabel(0), slider(0) {
    player = new QMediaPlayer(this);

    //-----------playlist model-view setup------------
    //playlist = new QMediaPlaylist();
    //player->setPlaylist(playlist);

    playlistModel = new PlaylistModel(this);
    //playlistModel->setPlaylist(playlist);

    // need to figure the correct column playlist view
    playlistView = new PlaylistTable(this);
    playlistView->setModel(playlistModel);
    playlistView->setCurrentIndex(playlistModel->index(playlistModel->getCurMediaIdx(),0));
    playlistView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    for (int c=1; c < playlistModel->columns; c++) {
        playlistView->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
    }

        // connect playlist signals to the player slots.
        connect(playlistView, SIGNAL(activated(QModelIndex)), this, SLOT(jump(QModelIndex)));
        connect(playlistModel, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));

        //------------Playback UI setup------------
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, player->duration()/1000);

        labelDuration = new QLabel(this);
        connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));

        QPushButton *openButton = new QPushButton(tr("Open"), this);
        connect(openButton, SIGNAL(clicked()), this, SLOT(open()));

        PlayerControls *controls = new PlayerControls(this);
        controls->setState(player->state());
        controls->setVolume(player->volume());
        controls->setMuted(controls->isMuted());

        connect(controls, SIGNAL(play()), player, SLOT(play()));
        connect(controls, SIGNAL(pause()), player, SLOT(pause()));
        connect(controls, SIGNAL(stop()), player, SLOT(stop()));
        connect(controls, SIGNAL(next()), this, SLOT(next()));
        connect(controls, SIGNAL(previous()), this, SLOT(previousClicked()));
        connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
        connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));
        connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));
        connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));
        connect(controls, SIGNAL(changeRate(qreal)), player, SLOT(setPlaybackRate(qreal)));
        connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),
                controls, SLOT(setState(QMediaPlayer::State)));

        //--------------player media signals connection--------
        connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
        connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
        connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
        connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
        connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
        connect(player, SIGNAL(audioAvailableChanged(bool)), this, SLOT(audioAvailableChanged(bool)));
        connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));

        //----------------- UI Layout ------------------
        QBoxLayout *displayLayout = new QHBoxLayout;
        displayLayout->addWidget(playlistView);

        QBoxLayout *controlLayout = new QHBoxLayout;
        controlLayout->setMargin(0);
        controlLayout->addWidget(openButton);
        controlLayout->addStretch(1);
        controlLayout->addWidget(controls);
        controlLayout->addStretch(1);

        QBoxLayout *layout = new QVBoxLayout;
        layout->addLayout(displayLayout);

        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->addWidget(slider);
        hLayout->addWidget(labelDuration);

        layout->addLayout(hLayout);
        layout->addLayout(controlLayout);

        setLayout(layout);

        //---------------- Initialization stuff ------------
        if (!player->isAvailable()) {
            QMessageBox::warning(this, tr("service not available"),
                    tr("The QMediaPlayer object does not have a valid service.\n"\
                        "Please check the media service plugins are installed."));
            controls->setEnabled(false);
            playlistView->setEnabled(false);
            openButton->setEnabled(false);
        }

        metaDataChanged();
        /*
        QStringList arguments = qApp->arguments();
        arguments.removeAt(0);
        // can invoke player in command line with files that want to be played.
        addToPlaylist(arguments);
        */
}

Player::~Player() {
    delete playlistView;
    delete playlistModel;
    //delete playlist;
    delete labelDuration;
    delete slider;
    delete coverLabel;
    delete player;
}


//--------------------Slots---------------------
void Player::open() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Files"));
    playlistModel->addMedia(fileNames);
}

void Player::durationChanged(qint64 duration) {
    this->duration = duration/1000;
    slider->setMaximum(duration/1000);
}

void Player::positionChanged(qint64 progress) {
    if (!slider->isSliderDown()) {
        slider->setValue(progress/1000);
    }
    updateDurationInfo(progress/1000);
}


void Player::previousClicked() {
    // Go to previous track if we are within the first 5 seconds of playback
    // otherwise, seek to the beginning
            
    if (player->position() <= 5000) {
        player->setMedia(playlistModel->previousMedia());
        player->play();
    }
    else {
        player->setPosition(0);
    }
}

void Player::jump(const QModelIndex &index) {
    if (index.isValid()) {
        player->setMedia(playlistModel->setCurMedia(index.row()));
        player->play();
    }
}

void Player::next() {
    player->setMedia(playlistModel->nextMedia());
    player->play();
}

void Player::playlistPositionChanged(int currentItem) {
    playlistView->setCurrentIndex(playlistModel->index(currentItem,0));
}

void Player::seek(int seconds) {
    player->setPosition(seconds * 1000);
}

void Player::statusChanged(QMediaPlayer::MediaStatus status) {
    handleCursor(status);

    switch(status) {
        case QMediaPlayer::UnknownMediaStatus:
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::LoadedMedia:
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::BufferedMedia:
            setStatusInfo(QString());
            break;
        case QMediaPlayer::LoadingMedia:
            setStatusInfo(tr("Loading..."));
            break;
        case QMediaPlayer::StalledMedia:
            qDebug() << "Media stalled";
            setStatusInfo(tr("Media Stalled"));
            break;
        case QMediaPlayer::EndOfMedia:
            player->setMedia(playlistModel->nextMedia());
            player->play();
            break;
        case QMediaPlayer::InvalidMedia:
            displayErrorMessage();
            break;
    }
}

void Player::handleCursor(QMediaPlayer::MediaStatus status) {
#ifndef QT_NO_CURSOR
    if (//status == QMediaPlayer::LoadedMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void Player::bufferingProgress(int progress) {
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void Player::audioAvailableChanged(bool available)
{
    Q_UNUSED(available);
    // Don't know what to implement yet...
    // for when audio becomes available or not
}

void Player::metaDataChanged() {
    qDebug() << "metaDataChanged()";
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(playlistModel->getCurAlbumArtist())
                .arg(playlistModel->getCurTitle()));
    }
}

void Player::setTrackInfo(const QString &info) {
    trackInfo = info;
    if (!statusInfo.isEmpty()) {
        emit(changeTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo)));
    }
    else {
        emit(changeTitle(trackInfo));
    }
}



void Player::setStatusInfo(const QString &info)
{
    statusInfo = info;
    if (!statusInfo.isEmpty())
        emit(changeTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo)));
    else
        emit(changeTitle(trackInfo));
}

void Player::displayErrorMessage()
{
    setStatusInfo(player->errorString());
}

void Player::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
        QString format = "mm:ss";
        if (duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}
