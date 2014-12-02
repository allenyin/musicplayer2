#include "playercontrols.h"

#include <QBoxLayout>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QComboBox>

// constructor
PlayerControls::PlayerControls(QWidget *parent) :
    QWidget(parent),
    playerState(QMediaPlayer::StoppedState),
    playerMuted(false),
    playButton(NULL),
    stopButton(NULL),
    nextButton(NULL),
    previousButton(NULL),
    muteButton(NULL),
    volumeSlider(NULL),
    rateBox(NULL)
{
    playButton = new QToolButton(this);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(playButton, SIGNAL(clicked()), this, SLOT(playClicked()));

    stopButton = new QToolButton(this);
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setEnabled(false);
    connect(stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));

    nextButton = new QToolButton(this);
    nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    connect(nextButton, SIGNAL(clicked()), this, SIGNAL(next()));

    previousButton = new QToolButton(this);
    previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    connect(previousButton, SIGNAL(clicked()), this, SIGNAL(previous()));

    muteButton = new QToolButton(this);
    muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    connect(muteButton, SIGNAL(clicked()), this, SLOT(muteClicked()));

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setFixedSize(volumeSlider->sizeHint());
    connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SIGNAL(changeVolume(int)));

    // playback rate box chooser
    /* Works like the following:
     * 1) whenver the box is "activiated" - user choosing something, call updateRate()
     * 2) updateRate() calls changeRate() on playbackRate();
     *      - playbackRate() is used to figure out which one is selected from the rateBox
     * 3) The changeRate() signal is caught by the setPlaybackRate() slot.
     * 4) The argment to setPlaybackRate() is the index of the item chosen. The slot then
     *    compares the index with the values in the box and set it.
     */
    rateBox = new QComboBox(this);
    rateBox->addItem("0.5x", QVariant(0.5));
    rateBox->addItem("1.0x", QVariant(1.0));
    rateBox->addItem("2.0x", QVariant(2.0));
    rateBox->setCurrentIndex(1);
    rateBox->setFixedSize(rateBox->sizeHint());
    connect(rateBox, SIGNAL(activated(int)), SLOT(updateRate()));

    // layout stuff
    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(stopButton);
    layout->addWidget(previousButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
    layout->addStretch();
    layout->setSpacing(0);

    QBoxLayout *rightEndLayout = new QHBoxLayout;
    rightEndLayout->setDirection(QBoxLayout::RightToLeft);
    rightEndLayout->addWidget(rateBox);
    rightEndLayout->setSpacing(2);
    rightEndLayout->addWidget(volumeSlider);
    rightEndLayout->addWidget(muteButton);

    layout->addLayout(rightEndLayout);
    setLayout(layout);
}

PlayerControls::~PlayerControls() {
    delete playButton;
    delete stopButton;
    delete nextButton;
    delete previousButton;
    delete muteButton;
    delete volumeSlider;
    delete rateBox;
}

QMediaPlayer::State PlayerControls::state() const {
    return playerState;
}

void PlayerControls::setState(QMediaPlayer::State state) {
    // change button styles on different states
    if (state != playerState) {
        playerState = state;

        switch(state) {
            case QMediaPlayer::StoppedState:
                stopButton->setEnabled(false);
                playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
                break;
            case QMediaPlayer::PlayingState:
                stopButton->setEnabled(true);
                playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
                break;
            case QMediaPlayer::PausedState:
                stopButton->setEnabled(true);
                playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
                break;
        }
    }
}

int PlayerControls::volume() const {
    return volumeSlider ? volumeSlider->value() : 0;
}

void PlayerControls::setVolume(int volume) {
    if (volumeSlider) 
        volumeSlider->setValue(volume);
}

bool PlayerControls::isMuted() const {
    return playerMuted;
}

void PlayerControls::setMuted(bool muted) {
    if (muted != playerMuted) {
        playerMuted = muted;

        muteButton->setIcon(style()->standardIcon(muted ? 
                    QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
    }
}

void PlayerControls::playClicked() {
    switch (playerState) {
        case QMediaPlayer::StoppedState:
        case QMediaPlayer::PausedState:
            emit play();
            break;
        case QMediaPlayer::PlayingState:
            emit pause();
            break;
    }
}

void PlayerControls::muteClicked() {
    emit changeMuting(!playerMuted);
}

qreal PlayerControls::playbackRate() const {
    return rateBox->itemData(rateBox->currentIndex()).toDouble();
}

void PlayerControls::setPlaybackRate(float rate) {
    for (int i=0; i<rateBox->count(); ++i) {
        if (qFuzzyCompare(rate, float(rateBox->itemData(i).toDouble()))) {
            rateBox->setCurrentIndex(i);
            return;
        }
    }

    rateBox->addItem(QString("%1x").arg(rate),QVariant(rate));
    rateBox->setCurrentIndex(rateBox->count() - 1);
}

void PlayerControls::updateRate() {
    emit changeRate(playbackRate());
}

    
