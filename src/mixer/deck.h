#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"
#include "mixer/stem.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMixer* pMixingEngine,
            EffectsManager* pEffectsManager,
            EngineChannel::ChannelOrientation defaultOrientation,
            const ChannelHandleAndGroup& handleGroup);
    ~Deck() override;
  private slots:
    void slotStemEnabled(double v);
  private:
    ControlObject* m_pStemControl;
    QString deckName;
};
