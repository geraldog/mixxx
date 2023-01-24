#pragma once

#include <QObject>

#include "mixer/basetrackplayer.h"

class Deck : public BaseTrackPlayerImpl {
    Q_OBJECT
  public:
    Deck(PlayerManager* pParent,
            UserSettingsPointer pConfig,
            EngineMaster* pMixingEngine,
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
