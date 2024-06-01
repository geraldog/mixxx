#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "control/controlproxy.h"
#include "engine/channels/enginechannel.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/class.h"

#include "mixer/playermanager.h"
#include "mixer/stem.h"

class ControlPushButton;
class TrackCollectionManager;
class PlayerManagerInterface;
class BaseTrackPlayer;
class PlaylistTableModel;
typedef QList<QModelIndex> QModelIndexList;

class DeckAttributes : public QObject {
    Q_OBJECT
  public:
    DeckAttributes(int index,
            BaseTrackPlayer* pPlayer);
    virtual ~DeckAttributes();

    bool isLeft() const {
        return m_orientation.get() == static_cast<double>(EngineChannel::LEFT);
    }

    bool isRight() const {
        return m_orientation.get() == static_cast<double>(EngineChannel::RIGHT);
    }

    bool isPlaying() const {
        return m_play.toBool();
    }

    void stop() {
        m_play.set(0.0);
    }

    void play() {
        m_play.set(1.0);
    }

    double playPosition() const {
        return m_playPos.get();
    }

    void setPlayPosition(double playpos) {
        m_playPos.set(playpos);
    }

    bool isRepeat() const {
        return m_repeat.toBool();
    }

    void setRepeat(bool enabled) {
        m_repeat.set(enabled ? 1.0 : 0.0);
    }

    mixxx::audio::FramePos introStartPosition() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_introStartPos.get());
    }

    mixxx::audio::FramePos introEndPosition() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_introEndPos.get());
    }

    mixxx::audio::FramePos outroStartPosition() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_outroStartPos.get());
    }

    mixxx::audio::FramePos outroEndPosition() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_outroEndPos.get());
    }

    mixxx::audio::SampleRate sampleRate() const {
        return mixxx::audio::SampleRate::fromDouble(m_sampleRate.get());
    }

    mixxx::audio::FramePos trackEndPosition() const {
        return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_trackSamples.get());
    }

    double rateRatio() const {
        return m_rateRatio.get();
    }

    TrackPointer getLoadedTrack() const;

  signals:
    void playChanged(DeckAttributes* pDeck, bool playing);
    void playPositionChanged(DeckAttributes* pDeck, double playPosition);
    void introStartPositionChanged(DeckAttributes* pDeck, double introStartPosition);
    void introEndPositionChanged(DeckAttributes* pDeck, double introEndPosition);
    void outroStartPositionChanged(DeckAttributes* pDeck, double outtroStartPosition);
    void outroEndPositionChanged(DeckAttributes* pDeck, double outroEndPosition);
    void trackLoaded(DeckAttributes* pDeck, TrackPointer pTrack);
    void loadingTrack(DeckAttributes* pDeck, TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty(DeckAttributes* pDeck);
    void rateChanged(DeckAttributes* pDeck);

  private slots:
    void slotPlayPosChanged(double v);
    void slotPlayChanged(double v);
    void slotIntroStartPositionChanged(double v);
    void slotIntroEndPositionChanged(double v);
    void slotOutroStartPositionChanged(double v);
    void slotOutroEndPositionChanged(double v);
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotPlayerEmpty();
    void slotRateChanged(double v);

  public:
    int index;
    QString group;
    double startPos;     // Set in toDeck nature
    double fadeBeginPos; // set in fromDeck nature
    double fadeEndPos;   // set in fromDeck nature
    bool isFromDeck;
    bool loading; // The data is inconsistent during loading a deck

  private:
    ControlProxy m_orientation;
    ControlProxy m_playPos;
    ControlProxy m_play;
    ControlProxy m_repeat;
    ControlProxy m_introStartPos;
    ControlProxy m_introEndPos;
    ControlProxy m_outroStartPos;
    ControlProxy m_outroEndPos;
    ControlProxy m_trackSamples;
    ControlProxy m_sampleRate;
    ControlProxy m_rateRatio;
    BaseTrackPlayer* m_pPlayer;
};

class AutoDJProcessor : public QObject {
    Q_OBJECT
  public:
    enum AutoDJState {
        ADJ_IDLE = 0,
        ADJ_LEFT_FADING,
        ADJ_RIGHT_FADING,
        ADJ_ENABLE_P1LOADED,
        ADJ_ENABLE_P1PLAYING,
        ADJ_DISABLED
    };

    enum AutoDJError {
        ADJ_OK = 0,
        ADJ_IS_INACTIVE,
        ADJ_QUEUE_EMPTY,
        ADJ_BOTH_DECKS_PLAYING,
        ADJ_DECKS_3_4_PLAYING,
        ADJ_NOT_TWO_DECKS
    };

    enum class TransitionMode {
        FullIntroOutro,
        FadeAtOutroStart,
        FixedFullTrack,
        FixedSkipSilence
    };

    AutoDJProcessor(QObject* pParent,
                    UserSettingsPointer pConfig,
                    PlayerManagerInterface* pPlayerManager,
                    TrackCollectionManager* pTrackCollectionManager,
                    int iAutoDJPlaylistId);
    virtual ~AutoDJProcessor();

    AutoDJState getState() const {
        return m_eState;
    }

    double getTransitionTime() const {
        return m_transitionTime;
    }

    TransitionMode getTransitionMode() const {
        return m_transitionMode;
    }

    PlaylistTableModel* getTableModel() const {
        return m_pAutoDJTableModel;
    }

    bool nextTrackLoaded();

    void setTransitionTime(int seconds);

    void setTransitionMode(TransitionMode newMode);

    AutoDJError shufflePlaylist(const QModelIndexList& selectedIndices);
    AutoDJError skipNext();
    void fadeNow();
    AutoDJError toggleAutoDJ(bool enable);

  signals:
    void loadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play);
    void autoDJStateChanged(AutoDJProcessor::AutoDJState state);
    void autoDJError(AutoDJProcessor::AutoDJError error);
    void transitionTimeChanged(int time);
    void randomTrackRequested(int tracksToAdd);

  private slots:
    void crossfaderChanged(double value);
    void playerPositionChanged(DeckAttributes* pDeck, double position);
    void playerPlayChanged(DeckAttributes* pDeck, bool playing);
    void playerIntroStartChanged(DeckAttributes* pDeck, double position);
    void playerIntroEndChanged(DeckAttributes* pDeck, double position);
    void playerOutroStartChanged(DeckAttributes* pDeck, double position);
    void playerOutroEndChanged(DeckAttributes* pDeck, double position);
    void playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack);
    void playerLoadingTrack(DeckAttributes* pDeck, TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerEmpty(DeckAttributes* pDeck);
    void playerRateChanged(DeckAttributes* pDeck);

    void controlEnableChangeRequest(double value);
    void controlFadeNow(double value);
    void controlShuffle(double value);
    void controlSkipNext(double value);
    void controlAddRandomTrack(double value);

    void slotStemPlaying(int stemNumber);

  protected:
    // The following virtual signal wrappers are used for testing
    virtual void emitLoadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play) {
        emit loadTrackToPlayer(pTrack, group, play);
    }
    virtual void emitAutoDJStateChanged(AutoDJProcessor::AutoDJState state) {
        emit autoDJStateChanged(state);
    }

  private:
    // Gets or sets the crossfader position while normalizing it so that -1 is
    // all the way mixed to the left side and 1 is all the way mixed to the
    // right side. (prevents AutoDJ logic from having to check for hamster mode
    // every time)
    double getCrossfader() const;
    void setCrossfader(double value);

    PlayerManager* m_pPlayerManager;

    bool wuwei = false;

    QString controlmixxx = "1";
    QString controlmixxx_lock = "1";
    QString confirmixxx = "1";
    QString mixxxposition1 = "1";
    QString mixxxposition2 = "1";
    QString mixxxlooping1beatcounter = "1";
    QString mixxxlooping2beatcounter = "1";

    uint64_t counter = 0;
    uint64_t DECK_1_Q_M_C_V = 0;
    uint64_t DECK_1_Q_M_O_V = 0;
    uint64_t DECK_1_Q_L_C_V = 0;
    uint64_t DECK_1_Q_L_O_V = 0;
    uint64_t DECK_1_Q_H_C_V = 0;
    uint64_t DECK_1_Q_H_O_V = 0;

    uint64_t DECK_2_Q_M_C_V = 0;
    uint64_t DECK_2_Q_M_O_V = 0;
    uint64_t DECK_2_Q_L_C_V = 0;
    uint64_t DECK_2_Q_L_O_V = 0;
    uint64_t DECK_2_Q_H_C_V = 0;
    uint64_t DECK_2_Q_H_O_V = 0;

    uint64_t STEM_1_S_V_C_V = 0;
    uint64_t STEM_1_S_V_O_V = 0;
    uint64_t STEM_2_S_V_C_V = 0;
    uint64_t STEM_2_S_V_O_V = 0;
    uint64_t STEM_3_S_V_C_V = 0;
    uint64_t STEM_3_S_V_O_V = 0;
    uint64_t STEM_4_S_V_C_V = 0;
    uint64_t STEM_4_S_V_O_V = 0;

    uint64_t STEM_5_S_V_C_V = 0;
    uint64_t STEM_5_S_V_O_V = 0;
    uint64_t STEM_6_S_V_C_V = 0;
    uint64_t STEM_6_S_V_O_V = 0;
    uint64_t STEM_7_S_V_C_V = 0;
    uint64_t STEM_7_S_V_O_V = 0;
    uint64_t STEM_8_S_V_C_V = 0;
    uint64_t STEM_8_S_V_O_V = 0;

    uint64_t CROSSFADER_X_V = 0;

    int Playing1Queue = 0;
    int Playing2Queue = 0;

    bool DECK_1_Q_M_C_B = false;
    bool DECK_1_Q_M_O_B = false;
    bool DECK_1_Q_L_C_B = false;
    bool DECK_1_Q_L_O_B = false;
    bool DECK_1_Q_H_C_B = false;
    bool DECK_1_Q_H_O_B = false;

    bool DECK_2_Q_M_C_B = false;
    bool DECK_2_Q_M_O_B = false;
    bool DECK_2_Q_L_C_B = false;
    bool DECK_2_Q_L_O_B = false;
    bool DECK_2_Q_H_C_B = false;
    bool DECK_2_Q_H_O_B = false;

    bool STEM_1_S_V_C_B = false;
    bool STEM_1_S_V_O_B = false;
    bool STEM_2_S_V_C_B = false;
    bool STEM_2_S_V_O_B = false;
    bool STEM_3_S_V_C_B = false;
    bool STEM_3_S_V_O_B = false;
    bool STEM_4_S_V_C_B = false;
    bool STEM_4_S_V_O_B = false;

    bool STEM_5_S_V_C_B = false;
    bool STEM_5_S_V_O_B = false;
    bool STEM_6_S_V_C_B = false;
    bool STEM_6_S_V_O_B = false;
    bool STEM_7_S_V_C_B = false;
    bool STEM_7_S_V_O_B = false;
    bool STEM_8_S_V_C_B = false;
    bool STEM_8_S_V_O_B = false;

    bool CROSSFADER_X_L_B = false;
    bool CROSSFADER_X_R_B = false;

    bool CROSSFADER_M_L_B = false;
    bool CROSSFADER_M_R_B = false;

    bool CROSSFADER_L_R_B = false;
    bool CROSSFADER_R_L_B = false;

    double diminuendo_EQ_1_MID;
    double crescendo_EQ_1_MID;
    double diminuendo_EQ_1_LOW;
    double crescendo_EQ_1_LOW;
    double diminuendo_EQ_1_HIGH;
    double crescendo_EQ_1_HIGH;

    double diminuendo_EQ_2_MID;
    double crescendo_EQ_2_MID;
    double diminuendo_EQ_2_LOW;
    double crescendo_EQ_2_LOW;
    double diminuendo_EQ_2_HIGH;
    double crescendo_EQ_2_HIGH;

    double diminuendo_STEM_1_VOLUME;
    double crescendo_STEM_1_VOLUME;
    double diminuendo_STEM_2_VOLUME;
    double crescendo_STEM_2_VOLUME;
    double diminuendo_STEM_3_VOLUME;
    double crescendo_STEM_3_VOLUME;
    double diminuendo_STEM_4_VOLUME;
    double crescendo_STEM_4_VOLUME;

    double diminuendo_STEM_5_VOLUME;
    double crescendo_STEM_5_VOLUME;
    double diminuendo_STEM_6_VOLUME;
    double crescendo_STEM_6_VOLUME;
    double diminuendo_STEM_7_VOLUME;
    double crescendo_STEM_7_VOLUME;
    double diminuendo_STEM_8_VOLUME;
    double crescendo_STEM_8_VOLUME;

    double crescendo_CROSS_X;
    double diminuendo_CROSS_X;

    bool LOCK = false;

    int WIP1 = 0;
    double gambi_loopin1;
    double gambi_loopout1;
    double gambi_hotcue1;
    double gambi_hotcueNumber1;

    int WIP2 = 0;
    double gambi_loopin2;
    double gambi_loopout2;
    double gambi_hotcue2;
    double gambi_hotcueNumber2;

    QString pathToSong1 = "1";
    QString pathToSong2 = "1";

    TrackPointer track1Loaded;
    TrackPointer track2Loaded;

    bool deck1Loading = false;
    bool deck2Loading = false;

    bool stemsDeck1Playing = false;
    bool stemsDeck2Playing = false;

    bool stem1Playing = false;
    bool stem2Playing = false;
    bool stem3Playing = false;
    bool stem4Playing = false;
    bool stem5Playing = false;
    bool stem6Playing = false;
    bool stem7Playing = false;
    bool stem8Playing = false;

    bool stemsDeck1Requested = false;
    bool stemsDeck2Requested = false;

    bool stemsDeck1Scratching = false;
    bool stemsDeck2Scratching = false;

    unsigned long long looping1BeatCounter = 0;
    unsigned long long looping2BeatCounter = 0;

    double loopingBeatDistance1;
    double loopingBeatDistance2;

    double m_PlayPositionDesired1 = 0;
    double m_PlayPositionDesired2 = 0;
    // Following functions return seconds computed from samples or -1 if
    // track in deck has invalid sample rate (<= 0)
    double getIntroStartSecond(DeckAttributes* pDeck);
    double getIntroEndSecond(DeckAttributes* pDeck);
    double getOutroStartSecond(DeckAttributes* pDeck);
    double getOutroEndSecond(DeckAttributes* pDeck);
    double getFirstSoundSecond(DeckAttributes* pDeck);
    double getLastSoundSecond(DeckAttributes* pDeck);
    double getEndSecond(DeckAttributes* pDeck);
    double framePositionToSeconds(mixxx::audio::FramePos position, DeckAttributes* pDeck);

    TrackPointer getNextTrackFromQueue();
    bool loadNextTrackFromQueue(const DeckAttributes& pDeck, bool play = false);
    void calculateTransition(DeckAttributes* pFromDeck,
            DeckAttributes* pToDeck,
            bool seekToStartPoint);
    void useFixedFadeTime(
            DeckAttributes* pFromDeck,
            DeckAttributes* pToDeck,
            double fromDeckSecond,
            double fadeEndSecond,
            double toDeckStartSecond);
    DeckAttributes* getLeftDeck();
    DeckAttributes* getRightDeck();
    DeckAttributes* getOtherDeck(const DeckAttributes* pThisDeck);
    DeckAttributes* getFromDeck();

    // Removes the track loaded to the player group from the top of the AutoDJ
    // queue if it is present.
    bool removeLoadedTrackFromTopOfQueue(const DeckAttributes& deck);

    // Removes the provided track from the top of the AutoDJ queue if it is
    // present.
    bool removeTrackFromTopOfQueue(TrackPointer pTrack);
    void maybeFillRandomTracks();
    UserSettingsPointer m_pConfig;
    PlaylistTableModel* m_pAutoDJTableModel;

    AutoDJState m_eState;
    double m_transitionProgress;
    double m_transitionTime; // the desired value set by the user
    TransitionMode m_transitionMode;

    QList<DeckAttributes*> m_decks;

    ControlProxy* m_pCOCrossfader;
    ControlProxy* m_pCOCrossfaderReverse;

    ControlPushButton* m_pSkipNext;
    ControlPushButton* m_pAddRandomTrack;
    ControlPushButton* m_pFadeNow;
    ControlPushButton* m_pShufflePlaylist;
    ControlPushButton* m_pEnabledAutoDJ;

    DISALLOW_COPY_AND_ASSIGN(AutoDJProcessor);
};
