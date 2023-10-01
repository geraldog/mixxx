#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"

class Stem : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Stem(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMixer* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Stem() override = default;
    QString stemName;
  signals:
    void stemPlaying(int stemNumber);
  public slots:
    void slotStemPlay(TrackPointer pTrack);
    void slotMuteDeck1();
    void slotMuteDeck2();
    void slotMuteDeck3();
    void slotMuteDeck4();
};
