#include "library/autodj/autodjprocessor.h"

#include <stdio.h>

#include <QThread>
#include <fstream>
#include <iostream>
#include <string>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/engine.h"
#include "library/trackcollection.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playermanager.h"
#include "moc_autodjprocessor.cpp"
#include "track/track.h"
#include "util/math.h"

#include "effects/effectparameterslotbase.h"
#include "effects/effectknobparameterslot.h"
#include "effects/chains/equalizereffectchain.h"

#define kConfigKey "[Auto DJ]"
namespace {
const char* kTransitionPreferenceName = "Transition";
const char* kTransitionModePreferenceName = "TransitionMode";
constexpr double kTransitionPreferenceDefault = 10.0;
constexpr double kKeepPosition = -1.0;

// A track needs to be longer than two callbacks to not stop AutoDJ
constexpr double kMinimumTrackDurationSec = 0.2;

constexpr bool sDebug = false;
} // anonymous namespace

DeckAttributes::DeckAttributes(int index,
        BaseTrackPlayer* pPlayer)
        : index(index),
          group(pPlayer->getGroup()),
          startPos(kKeepPosition),
          fadeBeginPos(1.0),
          fadeEndPos(1.0),
          isFromDeck(false),
          loading(false),
          m_orientation(group, "orientation"),
          m_playPos(group, "playposition"),
          m_play(group, "play"),
          m_repeat(group, "repeat"),
          m_introStartPos(group, "intro_start_position"),
          m_introEndPos(group, "intro_end_position"),
          m_outroStartPos(group, "outro_start_position"),
          m_outroEndPos(group, "outro_end_position"),
          m_trackSamples(group, "track_samples"),
          m_sampleRate(group, "track_samplerate"),
          m_rateRatio(group, "rate_ratio"),
          m_pPlayer(pPlayer) {
    connect(m_pPlayer, &BaseTrackPlayer::newTrackLoaded,
            this, &DeckAttributes::slotTrackLoaded);
    connect(m_pPlayer, &BaseTrackPlayer::loadingTrack,
            this, &DeckAttributes::slotLoadingTrack);
    connect(m_pPlayer, &BaseTrackPlayer::playerEmpty,
            this, &DeckAttributes::slotPlayerEmpty);
    m_playPos.connectValueChanged(this, &DeckAttributes::slotPlayPosChanged);
    m_play.connectValueChanged(this, &DeckAttributes::slotPlayChanged);
    m_introStartPos.connectValueChanged(this, &DeckAttributes::slotIntroStartPositionChanged);
    m_introEndPos.connectValueChanged(this, &DeckAttributes::slotIntroEndPositionChanged);
    m_outroStartPos.connectValueChanged(this, &DeckAttributes::slotOutroStartPositionChanged);
    m_outroEndPos.connectValueChanged(this, &DeckAttributes::slotOutroEndPositionChanged);
    m_rateRatio.connectValueChanged(this, &DeckAttributes::slotRateChanged);
}

DeckAttributes::~DeckAttributes() {
}

void DeckAttributes::slotPlayChanged(double v) {
    emit playChanged(this, v > 0.0);
}

void DeckAttributes::slotPlayPosChanged(double v) {
    emit playPositionChanged(this, v);
}

void DeckAttributes::slotIntroStartPositionChanged(double v) {
    emit introStartPositionChanged(this, v);
}

void DeckAttributes::slotIntroEndPositionChanged(double v) {
    emit introEndPositionChanged(this, v);
}

void DeckAttributes::slotOutroStartPositionChanged(double v) {
    emit outroStartPositionChanged(this, v);
}

void DeckAttributes::slotOutroEndPositionChanged(double v) {
    emit outroEndPositionChanged(this, v);
}

void DeckAttributes::slotTrackLoaded(TrackPointer pTrack) {
    emit trackLoaded(this, pTrack);
}

void DeckAttributes::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    //qDebug() << "DeckAttributes::slotLoadingTrack";
    emit loadingTrack(this, pNewTrack, pOldTrack);
}

void DeckAttributes::slotPlayerEmpty() {
    emit playerEmpty(this);
}

void DeckAttributes::slotRateChanged(double v) {
    Q_UNUSED(v);
    emit rateChanged(this);
}

TrackPointer DeckAttributes::getLoadedTrack() const {
    return m_pPlayer != nullptr ? m_pPlayer->getLoadedTrack() : TrackPointer();
}

AutoDJProcessor::AutoDJProcessor(
        QObject* pParent,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager,
        TrackCollectionManager* pTrackCollectionManager,
        int iAutoDJPlaylistId)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pAutoDJTableModel(nullptr),
          m_eState(ADJ_DISABLED),
          m_transitionProgress(0.0),
          m_transitionTime(kTransitionPreferenceDefault) {
    m_pAutoDJTableModel = new PlaylistTableModel(this, pTrackCollectionManager,
                                                 "mixxx.db.model.autodj");
    m_pAutoDJTableModel->setTableModel(iAutoDJPlaylistId);
    m_pAutoDJTableModel->select();

    m_pShufflePlaylist = new ControlPushButton(
            ConfigKey("[AutoDJ]", "shuffle_playlist"));
    connect(m_pShufflePlaylist, &ControlPushButton::valueChanged,
            this, &AutoDJProcessor::controlShuffle);

    m_pSkipNext = new ControlPushButton(
            ConfigKey("[AutoDJ]", "skip_next"));
    connect(m_pSkipNext, &ControlObject::valueChanged,
            this, &AutoDJProcessor::controlSkipNext);

    m_pAddRandomTrack = new ControlPushButton(
            ConfigKey("[AutoDJ]", "add_random_track"));
    connect(m_pAddRandomTrack,
            &ControlObject::valueChanged,
            this,
            &AutoDJProcessor::controlAddRandomTrack);

    m_pFadeNow = new ControlPushButton(
            ConfigKey("[AutoDJ]", "fade_now"));
    connect(m_pFadeNow, &ControlObject::valueChanged,
            this, &AutoDJProcessor::controlFadeNow);

    m_pEnabledAutoDJ = new ControlPushButton(
            ConfigKey("[AutoDJ]", "enabled"));
    m_pEnabledAutoDJ->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pEnabledAutoDJ, &ControlObject::valueChanged,
            this, &AutoDJProcessor::controlEnable);

    // TODO(rryan) listen to signals from PlayerManager and add/remove as decks
    // are created.
    for (unsigned int i = 0; i < pPlayerManager->numberOfDecks(); ++i) {
        QString group = PlayerManager::groupForDeck(i);
        BaseTrackPlayer* pPlayer = pPlayerManager->getPlayer(group);
        // Shouldn't be possible.
        VERIFY_OR_DEBUG_ASSERT(pPlayer) {
            continue;
        }
        m_decks.append(new DeckAttributes(i, pPlayer));
    }

    m_pPlayerManager = (PlayerManager*)pPlayerManager;

    // Auto-DJ needs at least two decks
    DEBUG_ASSERT(m_decks.length() > 1);

    m_pCOCrossfader = new ControlProxy("[Master]", "crossfader");
    m_pCOCrossfaderReverse = new ControlProxy("[Mixer Profile]", "xFaderReverse");

    QString str_autoDjTransition = m_pConfig->getValueString(
            ConfigKey(kConfigKey, kTransitionPreferenceName));
    if (!str_autoDjTransition.isEmpty()) {
        m_transitionTime = str_autoDjTransition.toDouble();
    }

    int configuredTransitionMode = m_pConfig->getValue(
            ConfigKey(kConfigKey, kTransitionModePreferenceName),
            static_cast<int>(TransitionMode::FullIntroOutro));
    m_transitionMode = static_cast<TransitionMode>(configuredTransitionMode);
}

AutoDJProcessor::~AutoDJProcessor() {
/*    delete m_Key1;
    delete m_Key2;

    delete m_LoopIn1;
    delete m_LoopOut1;
    delete m_LoopToggle1;

    delete m_LoopIn2;
    delete m_LoopOut2;
    delete m_LoopToggle2;

    delete m_PlayPosition1;
    delete m_Playing1;

    delete m_PlayPosition2;
    delete m_Playing2;

    delete m_EQ_1_LOW;
    delete m_EQ_1_MID;
    delete m_EQ_1_HIGH;

    delete m_EQ_2_LOW;
    delete m_EQ_2_MID;
    delete m_EQ_2_HIGH;*/

    qDeleteAll(m_decks);
    m_decks.clear();

    delete m_pCOCrossfader;
    delete m_pCOCrossfaderReverse;

    delete m_pSkipNext;
    delete m_pAddRandomTrack;
    delete m_pShufflePlaylist;
    delete m_pEnabledAutoDJ;
    delete m_pFadeNow;

    delete m_pAutoDJTableModel;
}

double AutoDJProcessor::getCrossfader() const {
    if (m_pCOCrossfaderReverse->toBool()) {
        return m_pCOCrossfader->get() * -1.0;
    }
    return m_pCOCrossfader->get();
}

void AutoDJProcessor::setCrossfader(double value) {
    if (m_pCOCrossfaderReverse->toBool()) {
        value *= -1.0;
    }
    m_pCOCrossfader->set(value);
}

AutoDJProcessor::AutoDJError AutoDJProcessor::shufflePlaylist(
        const QModelIndexList& selectedIndices) {
    QModelIndex exclude;
    if (m_eState != ADJ_DISABLED) {
        exclude = m_pAutoDJTableModel->index(0, 0);
    }
    m_pAutoDJTableModel->shuffleTracks(selectedIndices, exclude);
    return ADJ_OK;
}

void AutoDJProcessor::fadeNow() {
    if (m_eState != ADJ_IDLE) {
        // we cannot fade if AutoDj is disabled or already fading
        return;
    }

    double crossfader = getCrossfader();
    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();
    if (!pLeftDeck || !pRightDeck) {
        // User has changed the orientation, disable Auto DJ
        toggleAutoDJ(false);
        return;
    }

    DeckAttributes* pFromDeck;
    DeckAttributes* pToDeck;

    if (pLeftDeck->isPlaying() &&
            (!pRightDeck->isPlaying() || crossfader < 0.0)) {
        pFromDeck = pLeftDeck;
        pToDeck = pRightDeck;
    } else if (pRightDeck->isPlaying()) {
        pFromDeck = pRightDeck;
        pToDeck = pLeftDeck;
    } else {
        // Neither deck is playing. Fading now makes no sense.
        return;
    }

    pFromDeck->setRepeat(false);
    pFromDeck->isFromDeck = true;
    pToDeck->isFromDeck = false;

    const double fromDeckEndSecond = getEndSecond(pFromDeck);
    const double toDeckEndSecond = getEndSecond(pToDeck);
    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration. Use this alias for better readability.
    const double fromDeckDuration = fromDeckEndSecond;
    const double toDeckDuration = toDeckEndSecond;
    if (toDeckDuration < kMinimumTrackDurationSec) {
        // Deck is empty or track too short, disable AutoDJ
        // This happens only if the user has changed deck orientation to such deck.
        toggleAutoDJ(false);
        return;
    }

    // playPosition() is in the range of 0..1
    const double fromDeckCurrentSecond = fromDeckDuration * pFromDeck->playPosition();
    const double toDeckCurrentSecond = toDeckDuration * pToDeck->playPosition();

    if (toDeckDuration - toDeckCurrentSecond < kMinimumTrackDurationSec) {
        // Remaining Track time is too short, user has has seeked near the end
        // Re-cue the track
        pToDeck->setPlayPosition(pToDeck->startPos);
    }

    pFromDeck->fadeBeginPos = fromDeckCurrentSecond;
    // Do not seek to a calculated start point; start the to deck from wherever
    // it is if the user has seeked since loading the track.
    pToDeck->startPos = toDeckCurrentSecond;

    // If the user presses "Fade now", assume they want to fade *now*, not later.
    // So if the spinbox time is negative, do not insert silence.
    double spinboxTime = fabs(m_transitionTime);

    double fadeTime;
    if (m_transitionMode == TransitionMode::FullIntroOutro ||
            m_transitionMode == TransitionMode::FadeAtOutroStart) {
        // Use the intro length as the transition time. If the user has seeked
        // away from the intro start since the track was loaded, start from
        // there and do not seek back to the intro start. If they have seeked
        // past the introEnd or the introEnd is not marked, fall back to the
        // spinbox time.
        double outroEnd = getOutroEndSecond(pFromDeck);
        double introEnd = getIntroEndSecond(pToDeck);
        double introStart = getIntroStartSecond(pToDeck);
        double timeUntilOutroEnd = outroEnd - fromDeckCurrentSecond;

        // IntroStart ends up being equal to introEnd when pToDeck is
        // paused and its introEnd marker is not set. getIntroEndSecond returns
        // introStart thus the two end up having equal values
        if (toDeckCurrentSecond >= introStart &&
                toDeckCurrentSecond <= introEnd &&
                introStart != introEnd) {
            double timeUntilIntroEnd = introEnd - toDeckCurrentSecond;
            // The fade must end by the outro end at the latest.
            fadeTime = math_min(timeUntilIntroEnd, timeUntilOutroEnd);
        } else {
            // If this is true, the fade should have already been started
            // so the user should not have been able to press the Fade button.
            VERIFY_OR_DEBUG_ASSERT(timeUntilOutroEnd > 0) {
                timeUntilOutroEnd = 0;
            }
            fadeTime = math_min(spinboxTime, timeUntilOutroEnd);
        }
    } else {
        fadeTime = spinboxTime;
    }

    fadeTime = math_min(fadeTime, fromDeckEndSecond - fromDeckCurrentSecond);
    fadeTime = math_min(fadeTime,
            (toDeckEndSecond - toDeckCurrentSecond) / 2); // for fade in and out

    pFromDeck->fadeEndPos = fromDeckCurrentSecond + fadeTime;

    // These are expected to be a fraction of the track length.
    pFromDeck->fadeBeginPos /= fromDeckDuration;
    pFromDeck->fadeEndPos /= fromDeckDuration;
    pToDeck->startPos /= toDeckDuration;

    VERIFY_OR_DEBUG_ASSERT(pFromDeck->fadeBeginPos <= 1) {
        pFromDeck->fadeBeginPos = 1;
    }
}

AutoDJProcessor::AutoDJError AutoDJProcessor::skipNext() {
    if (m_eState == ADJ_DISABLED) {
        return ADJ_IS_INACTIVE;
    }
    // Load the next song from the queue.
    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();
    if (!pLeftDeck || !pRightDeck) {
        // User has changed the orientation, disable Auto DJ
        toggleAutoDJ(false);
        return ADJ_NOT_TWO_DECKS;
    }

    if (!pLeftDeck->isPlaying()) {
        removeLoadedTrackFromTopOfQueue(*pLeftDeck);
        loadNextTrackFromQueue(*pLeftDeck);
    } else if (!pRightDeck->isPlaying()) {
        removeLoadedTrackFromTopOfQueue(*pRightDeck);
        loadNextTrackFromQueue(*pRightDeck);
    } else {
        // If both decks are playing remove next track in playlist
        TrackId nextId = m_pAutoDJTableModel->getTrackId(m_pAutoDJTableModel->index(0, 0));
        TrackId leftId = pLeftDeck->getLoadedTrack()->getId();
        TrackId rightId = pRightDeck->getLoadedTrack()->getId();
        if (nextId == leftId || nextId == rightId) {
        // One of the playing tracks is still on top of playlist, remove second item
            m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(1, 0));
        } else {
            m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));
        }
        maybeFillRandomTracks();
    }
    return ADJ_OK;
}

AutoDJProcessor::AutoDJError AutoDJProcessor::toggleAutoDJ(bool enable) {
    if (enable) { // Enable Auto DJ
        DeckAttributes* pLeftDeck = getLeftDeck();
        DeckAttributes* pRightDeck = getRightDeck();
        if (!pLeftDeck || !pRightDeck) {
            // Keep the current state.
            emitAutoDJStateChanged(m_eState);
            return ADJ_NOT_TWO_DECKS;
        }

        bool leftDeckPlaying = pLeftDeck->isPlaying();
        bool rightDeckPlaying = pRightDeck->isPlaying();

        if (leftDeckPlaying && rightDeckPlaying) {
            qDebug() << "One deck must be stopped before enabling Auto DJ mode";
            // Keep the current state.
            emitAutoDJStateChanged(m_eState);
            return ADJ_BOTH_DECKS_PLAYING;
        }

        // TODO: This is a total bandaid for making Auto DJ work with decks 3
        // and 4.  We should design a nicer way to handle this.
        for (int i = 2; i < m_decks.length(); ++i) {
            if (m_decks[i] && m_decks[i]->isPlaying()) {
                // Keep the current state.
                return ADJ_DECKS_3_4_PLAYING;
            }
        }

        // Never load the same track if it is already playing
        if (leftDeckPlaying) {
            removeLoadedTrackFromTopOfQueue(*pLeftDeck);
        } else if (rightDeckPlaying) {
            removeLoadedTrackFromTopOfQueue(*pRightDeck);
        } else {
            // If the first track is already cued at a position in the first
            // 2/3 in on of the Auto DJ decks, start it.
            // If the track is paused at a later position, it is probably too
            // close to the end. In this case it is loaded again at the stored
            // cue point.
            if (pLeftDeck->playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(*pLeftDeck)) {
                pLeftDeck->play();
                leftDeckPlaying = true;
            } else if (pRightDeck->playPosition() < 0.66 &&
                    removeLoadedTrackFromTopOfQueue(*pRightDeck)) {
                pRightDeck->play();
                rightDeckPlaying = true;
            }
        }

        TrackPointer nextTrack = getNextTrackFromQueue();
        if (!nextTrack) {
            qDebug() << "Queue is empty now";
            if (m_pEnabledAutoDJ->get() != 0.0) {
                m_pEnabledAutoDJ->set(0.0);
            }
            emitAutoDJStateChanged(m_eState);
            return ADJ_QUEUE_EMPTY;
        }

        // Track is available so GO
        if (m_pEnabledAutoDJ->get() != 1.0) {
            m_pEnabledAutoDJ->set(1.0);
        }
        qDebug() << "Auto DJ enabled";

        m_pCOCrossfader->connectValueChanged(this, &AutoDJProcessor::crossfaderChanged);

        connect(pLeftDeck,
                &DeckAttributes::playPositionChanged,
                this,
                &AutoDJProcessor::playerPositionChanged);
        connect(pRightDeck,
                &DeckAttributes::playPositionChanged,
                this,
                &AutoDJProcessor::playerPositionChanged);

        connect(pLeftDeck,
                &DeckAttributes::playChanged,
                this,
                &AutoDJProcessor::playerPlayChanged);
        connect(pRightDeck,
                &DeckAttributes::playChanged,
                this,
                &AutoDJProcessor::playerPlayChanged);

        connect(pLeftDeck,
                &DeckAttributes::introStartPositionChanged,
                this,
                &AutoDJProcessor::playerIntroStartChanged);
        connect(pRightDeck,
                &DeckAttributes::introStartPositionChanged,
                this,
                &AutoDJProcessor::playerIntroStartChanged);

        connect(pLeftDeck,
                &DeckAttributes::introEndPositionChanged,
                this,
                &AutoDJProcessor::playerIntroEndChanged);
        connect(pRightDeck,
                &DeckAttributes::introEndPositionChanged,
                this,
                &AutoDJProcessor::playerIntroEndChanged);

        connect(pLeftDeck,
                &DeckAttributes::outroStartPositionChanged,
                this,
                &AutoDJProcessor::playerOutroStartChanged);
        connect(pRightDeck,
                &DeckAttributes::outroStartPositionChanged,
                this,
                &AutoDJProcessor::playerOutroStartChanged);

        connect(pLeftDeck,
                &DeckAttributes::outroEndPositionChanged,
                this,
                &AutoDJProcessor::playerOutroEndChanged);
        connect(pRightDeck,
                &DeckAttributes::outroEndPositionChanged,
                this,
                &AutoDJProcessor::playerOutroEndChanged);

        connect(pLeftDeck,
                &DeckAttributes::trackLoaded,
                this,
                &AutoDJProcessor::playerTrackLoaded);
        connect(pRightDeck,
                &DeckAttributes::trackLoaded,
                this,
                &AutoDJProcessor::playerTrackLoaded);

        connect(pLeftDeck,
                &DeckAttributes::loadingTrack,
                this,
                &AutoDJProcessor::playerLoadingTrack);
        connect(pRightDeck,
                &DeckAttributes::loadingTrack,
                this,
                &AutoDJProcessor::playerLoadingTrack);

        connect(pLeftDeck,
                &DeckAttributes::playerEmpty,
                this,
                &AutoDJProcessor::playerEmpty);
        connect(pRightDeck,
                &DeckAttributes::playerEmpty,
                this,
                &AutoDJProcessor::playerEmpty);

        connect(pLeftDeck,
                &DeckAttributes::rateChanged,
                this,
                &AutoDJProcessor::playerRateChanged);
        connect(pRightDeck,
                &DeckAttributes::rateChanged,
                this,
                &AutoDJProcessor::playerRateChanged);

        if (!leftDeckPlaying && !rightDeckPlaying) {
            // Both decks are stopped. Load a track into deck 1 and start it
            // playing. Instruct playerPositionChanged to wait for a
            // playposition update from deck 1. playerPositionChanged for
            // ADJ_ENABLE_P1LOADED will set the crossfader left and remove the
            // loaded track from the queue and wait for the next call to
            // playerPositionChanged for deck1 after the track is loaded.
            m_eState = ADJ_ENABLE_P1LOADED;

            // Move crossfader to the left.
            setCrossfader(-1.0);

            // Load track into the left deck and play. Once it starts playing,
            // we will receive a playerPositionChanged update for deck 1 which
            // will load a track into the right deck and switch to IDLE mode.
            emitLoadTrackToPlayer(nextTrack, pLeftDeck->group, true);
        } else {
            // One of the two decks is playing. Switch into IDLE mode and wait
            // until the playing deck crosses posThreshold to start fading.
            m_eState = ADJ_IDLE;
            if (leftDeckPlaying) {
                // Load track into the right deck.
                emitLoadTrackToPlayer(nextTrack, pRightDeck->group, false);
                // Move crossfader to the left.
                setCrossfader(-1.0);
            } else {
                // Load track into the left deck.
                emitLoadTrackToPlayer(nextTrack, pLeftDeck->group, false);
                // Move crossfader to the right.
                setCrossfader(1.0);
            }
        }
        emitAutoDJStateChanged(m_eState);
    } else { // Disable Auto DJ
        m_pEnabledAutoDJ->set(0.0);
        qDebug() << "Auto DJ disabled";
        m_eState = ADJ_DISABLED;
        disconnect(m_pCOCrossfader,
                &ControlProxy::valueChanged,
                this,
                &AutoDJProcessor::crossfaderChanged);
        for (const auto& pDeck : std::as_const(m_decks)) {
            pDeck->disconnect(this);
        }
        emitAutoDJStateChanged(m_eState);
    }
    return ADJ_OK;
}

void AutoDJProcessor::controlEnable(double value) {
    toggleAutoDJ(value > 0.0);
}

void AutoDJProcessor::controlFadeNow(double value) {
    if (value > 0.0) {
        fadeNow();
    }
}

void AutoDJProcessor::controlShuffle(double value) {
    if (value > 0.0) {
        shufflePlaylist(QModelIndexList());
    }
}

void AutoDJProcessor::controlSkipNext(double value) {
    if (value > 0.0) {
        skipNext();
    }
}

void AutoDJProcessor::controlAddRandomTrack(double value) {
    if (value > 0.0) {
        emit randomTrackRequested(1);
    }
}

void AutoDJProcessor::crossfaderChanged(double value) {
    if (m_eState == ADJ_IDLE) {
        // The user is changing the crossfader manually. If the user has
        // moved it all the way to the other side, make the deck faded away
        // from the new "to deck" by loading the next track into it.
        DeckAttributes* pFromDeck = getFromDeck();
        VERIFY_OR_DEBUG_ASSERT(pFromDeck) {
            // we have always a from deck in case of state IDLE
            return;
        }

        DeckAttributes* pToDeck = getOtherDeck(pFromDeck);
        if (!pToDeck) {
            // we have always a from deck in case of state IDLE
            // if the user has not changed the deck orientation
            return;
        }

        double crossfaderPosition = value * (m_pCOCrossfaderReverse->toBool() ? -1 : 1);
        if ((crossfaderPosition == 1.0 && pFromDeck->isLeft()) ||       // crossfader right
                (crossfaderPosition == -1.0 && pFromDeck->isRight())) { // crossfader left
            if (!pToDeck->isPlaying()) {
                if (getEndSecond(pToDeck) >= kMinimumTrackDurationSec) {
                    // Re-cue the track if the user has seeked it to the very end
                    if (pToDeck->playPosition() >= pToDeck->fadeBeginPos) {
                        pToDeck->setPlayPosition(pToDeck->startPos);
                    }
                    pToDeck->play();
                } else {
                    toggleAutoDJ(false);
                    return;
                }
            }
            pFromDeck->stop();

            // Now that we have started the other deck playing, remove the track
            // that was "on deck" from the top of the queue.
            removeLoadedTrackFromTopOfQueue(*pToDeck);
            loadNextTrackFromQueue(*pFromDeck);
        }
    }
}

void AutoDJProcessor::playerPositionChanged(DeckAttributes* pAttributes,
                                            double thisPlayPosition) {
    // qDebug() << "player" << pAttributes->group << "PositionChanged(" << value << ")";
    if (m_eState == ADJ_DISABLED) {
        // nothing to do
        return;
    }

    DeckAttributes* thisDeck = pAttributes;
    DeckAttributes* otherDeck = getOtherDeck(thisDeck);
    if (!otherDeck) {
        // This happens if this deck has no orientation or
        // there is no deck with the opposite orientation
        return;
    }

    std::ofstream confirmado;

    ControlProxy* m_PlayPosition1 = new ControlProxy(QString("[Channel1]"), "playposition");
    ControlProxy* m_Playing1 = new ControlProxy(QString("[Channel1]"), "play");

    ControlProxy* m_PlayPosition2 = new ControlProxy(QString("[Channel2]"), "playposition");
    ControlProxy* m_Playing2 = new ControlProxy(QString("[Channel2]"), "play");

    DeckAttributes& leftDecko = *m_decks[0];
    DeckAttributes& rightDecko = *m_decks[1];

    ControlProxy* m_LoopIn1 = new ControlProxy(QString("[Channel1]"), "loop_in");
    ControlProxy* m_LoopOut1 = new ControlProxy(QString("[Channel1]"), "loop_out");
    ControlProxy* m_LoopToggle1 = new ControlProxy(QString("[Channel1]"), "reloop_toggle");

    ControlProxy* m_LoopIn2 = new ControlProxy(QString("[Channel2]"), "loop_in");
    ControlProxy* m_LoopOut2 = new ControlProxy(QString("[Channel2]"), "loop_out");
    ControlProxy* m_LoopToggle2 = new ControlProxy(QString("[Channel2]"), "reloop_toggle");

    ControlProxy* m_pCue1 = new ControlProxy(QString("[Channel1]"), "cue_gotoandplay");
    ControlProxy* m_pCue2 = new ControlProxy(QString("[Channel2]"), "cue_gotoandplay");

    ControlProxy* m_photCue11Set = new ControlProxy(QString("[Channel1]"), "hotcue_1_set");
    ControlProxy* m_photCue21Set = new ControlProxy(QString("[Channel2]"), "hotcue_1_set");

    ControlProxy* m_photCue11Clear = new ControlProxy(QString("[Channel1]"), "hotcue_1_clear");
    ControlProxy* m_photCue21Clear = new ControlProxy(QString("[Channel2]"), "hotcue_1_clear");

    ControlProxy* m_trackSamples1 = new ControlProxy(QString("[Channel1]"), "track_samples");
    ControlProxy* m_trackSamples2 = new ControlProxy(QString("[Channel2]"), "track_samples");

    const QString EQ_group_1 = EqualizerEffectChain::formatEffectSlotGroup(
            QString("[Channel1]"));

    const QString EQ_group_2 = EqualizerEffectChain::formatEffectSlotGroup(
            QString("[Channel2]"));

    ControlProxy* m_EQ_1_LOW = new ControlProxy(EQ_group_1, EffectKnobParameterSlot::formatItemPrefix(0));
    ControlProxy* m_EQ_1_MID = new ControlProxy(EQ_group_1, EffectKnobParameterSlot::formatItemPrefix(1));
    ControlProxy* m_EQ_1_HIGH = new ControlProxy(EQ_group_1, EffectKnobParameterSlot::formatItemPrefix(2));

    ControlProxy* m_EQ_2_LOW = new ControlProxy(EQ_group_2, EffectKnobParameterSlot::formatItemPrefix(0));
    ControlProxy* m_EQ_2_MID = new ControlProxy(EQ_group_2, EffectKnobParameterSlot::formatItemPrefix(1));
    ControlProxy* m_EQ_2_HIGH = new ControlProxy(EQ_group_2, EffectKnobParameterSlot::formatItemPrefix(2));

    std::ifstream controlbaby;

    std::string comando;
    std::string contador;

    uint64_t contagiros;

    std::string slope;
    double slopenumerico;

    std::string loopin1;
    std::string loopout1;

    std::string loopin2;
    std::string loopout2;

    double loopin_double1;
    double loopout_double1;

    double loopin_double2;
    double loopout_double2;


    if (this->wuwei == true) {
        std::cout << "OH NO I'VE ENTERED WUWEI MODE. NOW ALL I DO IS STARE AT "
                     "WALLS WHILE YOU STARE AT SCREENS!\n";
        m_Playing1->set(0.0);
        m_Playing2->set(0.0);
        goto clean_exit;
    }


    if (this->LOCK == true) {
        if (this->WIP1 == 1) {
            goto clean_exit;
        }

        else if (this->WIP1 == 2) {
            m_LoopIn1->set(1);
            m_LoopIn1->set(0);
            this->WIP1 = 3;

            goto clean_exit;
        }

        else if (this->WIP1 == 3) {
            m_PlayPosition1->set(this->gambi_loopout1);
            this->WIP1 = 4;

            goto clean_exit;
        }

        else if (this->WIP1 == 4) {
            if (m_PlayPosition1->get() >= this->gambi_loopout1) {
                m_LoopOut1->set(1);
                m_LoopOut1->set(0);

                confirmado.open("/home/dumbo/confirmixxx.txt");
                confirmado << std::to_string(this->counter) + "\n";
                confirmado.close();

                this->counter++;

                this->WIP1 = 0;
                this->LOCK = false;

                goto clean_exit;

            }

            else {
                goto clean_exit;
            }
        }

        else if (this->WIP1 == 5) {
            m_PlayPosition1->set(this->gambi_loopin1);
            this->WIP1 = 2;

            goto clean_exit;
        }

        if (this->WIP2 == 1) {
            goto clean_exit;
        }

        else if (this->WIP2 == 2) {
            m_LoopIn2->set(1);
            m_LoopIn2->set(0);
            this->WIP2 = 3;

            goto clean_exit;
        }

        else if (this->WIP2 == 3) {
            m_PlayPosition2->set(this->gambi_loopout2);
            this->WIP2 = 4;

            goto clean_exit;
        }

        else if (this->WIP2 == 4) {
            if (m_PlayPosition2->get() >= this->gambi_loopout2) {
                m_LoopOut2->set(1);
                m_LoopOut2->set(0);

                confirmado.open("/home/dumbo/confirmixxx.txt");
                confirmado << std::to_string(this->counter) + "\n";
                confirmado.close();

                this->counter++;

                this->WIP2 = 0;
                this->LOCK = false;

                goto clean_exit;

            }

            else {
                goto clean_exit;
            }
        }

        else if (this->WIP2 == 5) {
            m_PlayPosition2->set(this->gambi_loopin2);
            this->WIP2 = 2;

            goto clean_exit;
        }

        std::cout << this->pathToSong1.toStdString() + "\n";

        if (Playing1Queue == 2 && leftDecko.getLoadedTrack()->getLocation() == this->pathToSong1) {

            uint64_t totalSamples1 =
                    (unsigned long long)((double)this->track1Loaded
                                                 ->getSampleRate() *
                            this->track1Loaded->getDuration() * 2);

            bool hasMainCue1 = false;
	    QList<CuePointer> track1CueList = this->track1Loaded->getCuePoints();
            QList<CuePointer>::iterator i;

            for (i = track1CueList.begin(); i != track1CueList.end(); ++i) {
                CuePointer mainCue1 = *i;
                if (mainCue1.get()->getType() == mixxx::CueType::MainCue) {
                    mainCue1.get()->
		        setStartAndEndPosition(
			    mixxx::audio::FramePos::fromEngineSamplePos(
			        this->m_PlayPositionDesired1 * totalSamples1),
			    mixxx::audio::FramePos::fromEngineSamplePos(
			        this->m_PlayPositionDesired1 * totalSamples1));
		    hasMainCue1 = true;
                }
            }

	    if (hasMainCue1 == false) {
	        this->track1Loaded->createAndAddCue(
                    mixxx::CueType::MainCue,
                    Cue::kNoHotCue,
                    mixxx::audio::FramePos::fromEngineSamplePos(
                        this->m_PlayPositionDesired1 * totalSamples1),
                    mixxx::audio::FramePos::fromEngineSamplePos(
                        this->m_PlayPositionDesired1 * totalSamples1));
            }

            Playing1Queue = 3;
            goto clean_exit;
        }

        else if (Playing1Queue == 3 &&
                leftDecko.getLoadedTrack()->getLocation() ==
                        this->pathToSong1) {
            Playing1Queue = 4;

            m_pCue1->set(1.0);

            goto clean_exit;
        }

        else if (Playing1Queue == 4 &&
                leftDecko.getLoadedTrack()->getLocation() ==
                        this->pathToSong1) {

            if (m_PlayPosition1->get() < this->m_PlayPositionDesired1) {
                m_pCue1->set(1.0);
                goto clean_exit;
            }

            confirmado.open("/home/dumbo/confirmixxx.txt");
            confirmado << std::to_string(this->counter) + "\n";
            confirmado.close();

            this->counter++;

            Playing1Queue = 0;
            this->LOCK = false;
            goto clean_exit;

        }

        else if (Playing1Queue == 1 &&
                leftDecko.getLoadedTrack()->getLocation() ==
                        this->pathToSong1) {
            Playing1Queue = 2;

            goto clean_exit;
        }

        if (Playing2Queue == 2 && rightDecko.getLoadedTrack()->getLocation() == this->pathToSong2) {

            uint64_t totalSamples2 =
                    (unsigned long long)((double)this->track2Loaded
                                                 ->getSampleRate() *
                            this->track2Loaded->getDuration() * 2);

            bool hasMainCue2 = false;
	    QList<CuePointer> track2CueList = this->track2Loaded->getCuePoints();
            QList<CuePointer>::iterator i;

            for (i = track2CueList.begin(); i != track2CueList.end(); ++i) {
                CuePointer mainCue2 = *i;
                if (mainCue2.get()->getType() == mixxx::CueType::MainCue) {
                    mainCue2.get()->setStartAndEndPosition(
		        mixxx::audio::FramePos::fromEngineSamplePos(
			    this->m_PlayPositionDesired2 * totalSamples2),
                        mixxx::audio::FramePos::fromEngineSamplePos(
			    this->m_PlayPositionDesired2 * totalSamples2));
                hasMainCue2 = true;
                }
            }

	    if (hasMainCue2 == false) {
	        this->track2Loaded->createAndAddCue(
                    mixxx::CueType::MainCue,
                    Cue::kNoHotCue,
                    mixxx::audio::FramePos::fromEngineSamplePos(
                        this->m_PlayPositionDesired2 * totalSamples2),
                    mixxx::audio::FramePos::fromEngineSamplePos(
                        this->m_PlayPositionDesired2 * totalSamples2));
            }

            Playing2Queue = 3;
            goto clean_exit;
        }

        else if (Playing2Queue == 3 &&
                rightDecko.getLoadedTrack()->getLocation() ==
                        this->pathToSong2) {
            m_pCue2->set(1.0);

            Playing2Queue = 4;

            goto clean_exit;
        }

        else if (Playing2Queue == 4 &&
                rightDecko.getLoadedTrack()->getLocation() ==
                        this->pathToSong2) {

            if (m_PlayPosition2->get() < m_PlayPositionDesired2) {
                m_pCue2->set(1.0);
                goto clean_exit;
            }

            confirmado.open("/home/dumbo/confirmixxx.txt");
            confirmado << std::to_string(this->counter) + "\n";
            confirmado.close();

            this->counter++;

            Playing2Queue = 0;
            this->LOCK = false;

            goto clean_exit;
        }

        else if (Playing2Queue == 1 &&
                rightDecko.getLoadedTrack()->getLocation() ==
                        this->pathToSong2) {
            Playing2Queue = 2;

            goto clean_exit;
        }

        goto clean_exit;
    }

    if (FILE* file = fopen("/home/dumbo/controlmixxx.txt.lock", "r")) {
        fclose(file);
        goto clean_exit;
    }

    controlbaby.open("/home/dumbo/controlmixxx.txt");
    std::getline(controlbaby, comando);
    std::getline(controlbaby, contador);
    controlbaby.close();

    try {
        contagiros = std::stoull(contador);
    }

    catch (const std::invalid_argument& e) {
        this->wuwei = true;
        std::cout << "INVALID ARGUMENT AT CONTAGIROS WAS \n" + contador;
        goto clean_exit;
    }

    catch (const std::out_of_range& e) {
        this->wuwei = true;
        std::cout << "OUT OF RANGE ARGUMENT FOR CONTAGIROS\n";
        goto clean_exit;
    }

    if (this->wuwei == false) {
        if (contagiros > this->counter) {
            this->LOCK = true;

            if (comando[0] == 'Z') {
                if (comando[1] == '1') {
                    ControlProxy* m_pStemEngage = new ControlProxy("[Channel1]", "LoadStems");
		    m_pStemEngage->set(1.0);

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
		    delete m_pStemEngage;
                    goto clean_exit;
                }

                else if (comando[1] == '2') {
                    ControlProxy* m_pStemEngage = new ControlProxy("[Channel2]", "LoadStems");
		    m_pStemEngage->set(1.0);

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
		    delete m_pStemEngage;
                    goto clean_exit;
                }
            }

            else if (comando[0] == 'C') {
                if (comando[1] == '1') {
                    m_LoopToggle1->set(1);

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
                    goto clean_exit;
                }

                if (comando[1] == '2') {
                    m_LoopToggle2->set(1);

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
                    goto clean_exit;
                }
            }

            else if (comando[0] == 'K') {
                if (comando[1] == '1') {
                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        slope = slope + comando[usecamisinha];
                    }

                    try {
                        slopenumerico = std::stod(slope);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR KEY 1 SLOPE\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT FOR KEY 1 SLOPE\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    ControlProxy* m_Key1 = new ControlProxy("[Channel1]", "key");
                    m_Key1->set(slopenumerico);
                    delete m_Key1;

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
                    goto clean_exit;
                }

                else if (comando[1] == '2') {
                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        slope = slope + comando[usecamisinha];
                    }

                    try {
                        slopenumerico = std::stod(slope);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR KEY 2 SLOPE\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT FOR 2 KEY SLOPE\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    ControlProxy* m_Key2 = new ControlProxy("[Channel2]", "key");
                    m_Key2->set(slopenumerico);
                    delete m_Key2;

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
                    goto clean_exit;
                }

            }

            else if (comando[0] == 'F') {
                if (comando[1] == 'X') {
                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        slope = slope + comando[usecamisinha];
                    }

                    try {
                        slopenumerico = std::stod(slope);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR CROSSFADER INVERSION SLOPE\n";
                        this->CROSSFADER_X_L_B = false;
                        this->CROSSFADER_X_R_B = false;
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT CROSSFADER INVERSION SLOPE\n";
                        this->CROSSFADER_X_L_B = false;
                        this->CROSSFADER_X_R_B = false;
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    if (m_pCOCrossfader->get() <= 0) {
                        this->CROSSFADER_X_R_B = true;

                        this->crescendo_CROSS_X = slopenumerico / 1000;
                        this->CROSSFADER_X_V = 10000;

                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;

                        this->LOCK = false;
                        goto clean_exit;
                    }

                    else if (m_pCOCrossfader->get() >= 0) {
                        this->CROSSFADER_X_L_B = true;

                        this->diminuendo_CROSS_X = slopenumerico / 1000;
                        this->CROSSFADER_X_V = 10000;

                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;

                        this->LOCK = false;
                        goto clean_exit;
                    }
                }
            }

            else if (comando[0] == 'T') {
                if (comando[1] == '1') {
                    if (m_Playing1->get() != 1) {
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    std::string bpmAsked1;
                    double bpm_double1;

                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        bpmAsked1 = bpmAsked1 + comando[usecamisinha];
                    }

                    try {
                        bpm_double1 = std::stod(bpmAsked1);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR DECK 1 TEMPO\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT FOR FOR DECK 1 TEMPO\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    ControlProxy* m_Bpm1 = new ControlProxy(QString("[Channel1]"), "rate");
                    ControlProxy* m_FileBpm1 = new ControlProxy(QString("[Channel1]"), "file_bpm");

                    ControlProxy* m_pRateDir = new ControlProxy(QString("[Channel1]"), "rate_dir");
                    ControlProxy* m_pRateRange = new ControlProxy(
                            QString("[Channel1]"), "rateRange");
                    double rateScale = m_pRateDir->get() * m_pRateRange->get();

                    if (m_FileBpm1->get() == 0.0 || rateScale == 0.0) {
                        std::cout << "BPM IS UNDEFINED\n";
                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;

                        this->LOCK = false;

                        delete m_Bpm1;
			delete m_FileBpm1;
			delete m_pRateDir;
			delete m_pRateRange;

                        goto clean_exit;
                    }

                    double dRateSlider = (bpm_double1 / m_FileBpm1->get() - 1.0) / rateScale;

                    if (bpm_double1 > m_FileBpm1->get() && bpm_double1 <= m_FileBpm1->get() * 1.9) {
                        std::cout << "FILE BPM IS: " +
                                        std::to_string(m_FileBpm1->get()) +
                                        " SETTING BPM TO: " +
                                        std::to_string(bpm_double1) + "\n";
                        m_Bpm1->set(dRateSlider);

                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
                        delete m_Bpm1;
			delete m_FileBpm1;
			delete m_pRateDir;
			delete m_pRateRange;
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    else if (bpm_double1 < m_FileBpm1->get() &&
                            bpm_double1 >= (m_FileBpm1->get() -
                                                   m_FileBpm1->get() * 0.9)) {
                        std::cout << "FILE BPM IS: " +
                                        std::to_string(m_FileBpm1->get()) +
                                        " SETTING BPM TO: " +
                                        std::to_string(bpm_double1) + "\n";
                        m_Bpm1->set(dRateSlider);

                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
			delete m_Bpm1;
			delete m_FileBpm1;
			delete m_pRateDir;
			delete m_pRateRange;

                        this->LOCK = false;
                        goto clean_exit;
                    }

                    else {
                        std::cout << "BPM IS EITHER THE SAME OR ELSE IT'S "
                                     "BEYOND LIMITS. LEAVING UNCHANGED\n";
                        m_Bpm1->set(0.0);
                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
			delete m_pRateDir;
			delete m_pRateRange;

                        this->LOCK = false;
                        goto clean_exit;
                    }

                }

                else if (comando[1] == '2') {
                    if (m_Playing2->get() != 1) {
                        this->LOCK = false;
                        goto clean_exit;
                    }
                    std::string bpmAsked2;
                    double bpm_double2;

                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        bpmAsked2 = bpmAsked2 + comando[usecamisinha];
                    }

                    try {
                        bpm_double2 = std::stod(bpmAsked2);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR DECK 2 TEMPO\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 TEMPO\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    ControlProxy* m_Bpm2 = new ControlProxy(QString("[Channel2]"), "rate");
                    ControlProxy* m_FileBpm2 = new ControlProxy(QString("[Channel2]"), "file_bpm");

                    ControlProxy* m_pRateDir = new ControlProxy(QString("[Channel2]"), "rate_dir");
                    ControlProxy* m_pRateRange = new ControlProxy(
                            QString("[Channel2]"), "rateRange");
                    double rateScale = m_pRateDir->get() * m_pRateRange->get();

                    if (m_FileBpm2->get() == 0.0 || rateScale == 0.0) {
                        std::cout << "BPM IS UNDEFINED\n";
                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
			delete m_Bpm2;
			delete m_FileBpm2;
			delete m_pRateDir;
			delete m_pRateRange;

                        this->LOCK = false;

                        goto clean_exit;
                    }

                    double dRateSlider = (bpm_double2 / m_FileBpm2->get() - 1.0) / rateScale;

                    if (bpm_double2 > m_FileBpm2->get() && bpm_double2 <= m_FileBpm2->get() * 1.9) {
                        std::cout << "FILE BPM IS: " +
                                        std::to_string(m_FileBpm2->get()) +
                                        " SETTING BPM TO: " +
                                        std::to_string(bpm_double2) + "\n";
                        m_Bpm2->set(dRateSlider);

                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
			delete m_Bpm2;
			delete m_FileBpm2;
			delete m_pRateDir;
			delete m_pRateRange;

                        this->LOCK = false;
                        goto clean_exit;
                    }

                    else if (bpm_double2 < m_FileBpm2->get() &&
                            bpm_double2 >= (m_FileBpm2->get() -
                                                   m_FileBpm2->get() * 0.9)) {
                        std::cout << "FILE BPM IS: " +
                                        std::to_string(m_FileBpm2->get()) +
                                        " SETTING BPM TO: " +
                                        std::to_string(bpm_double2) + "\n";
                        m_Bpm2->set(dRateSlider);

                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
			delete m_Bpm2;
			delete m_FileBpm2;
			delete m_pRateDir;
			delete m_pRateRange;

                        this->LOCK = false;
                        goto clean_exit;
                    }

                    else {
                        std::cout << "BPM IS EITHER THE SAME OR ELSE IT'S "
                                     "BEYOND LIMITS. LEAVING UNCHANGED\n";
                        m_Bpm2->set(0.0);
                        confirmado.open("/home/dumbo/confirmixxx.txt");
                        confirmado << std::to_string(this->counter) + "\n";
                        confirmado.close();

                        this->counter++;
			delete m_Bpm2;
			delete m_FileBpm2;
			delete m_pRateDir;
			delete m_pRateRange;

                        this->LOCK = false;
                        goto clean_exit;
                    }
                }

            }

            else if (comando[0] == 'L') {
                if (comando[1] == '1') {
                    std::cout << "OLD DECK 1 PATH: " + this->pathToSong1.toStdString() + "\n";

                    this->pathToSong1 = "";

                    for (long unsigned int usecamisinha = 2;
                            usecamisinha < comando.length();
                            usecamisinha++) {
                        this->pathToSong1 = this->pathToSong1 + comando[usecamisinha];
                    }

                    std::cout << "NEW DECK 1 PATH: " + this->pathToSong1.toStdString() + "\n";

                    m_Playing1->set(0.0);

                    m_PlayPosition1->set(0);

                    this->track1Loaded =
                            m_pPlayerManager->slotLoadToDeck(
                                    pathToSong1, 1);

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
                    goto clean_exit;
                }

                if (comando[1] == '2') {
                    this->pathToSong2 = "";

                    for (long unsigned int usecamisinha = 2;
                            usecamisinha < comando.length();
                            usecamisinha++) {
                        this->pathToSong2 = this->pathToSong2 + comando[usecamisinha];
                    }

                    m_Playing2->set(0.0);

                    m_PlayPosition2->set(0);

                    this->track2Loaded =
                            m_pPlayerManager->slotLoadToDeck(
                                    pathToSong2, 2);

                    confirmado.open("/home/dumbo/confirmixxx.txt");
                    confirmado << std::to_string(this->counter) + "\n";
                    confirmado.close();

                    this->counter++;

                    this->LOCK = false;
                    goto clean_exit;
                }

            }

            else if (comando[0] == 'P') {
                if (comando[1] == '1') {
                    std::string cuePosition1;
                    double cuePosition1_double;

                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        cuePosition1 = cuePosition1 + comando[usecamisinha];
                    }

                    try {
                        cuePosition1_double = std::stod(cuePosition1);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR PLAY CUE POSITION ON DECK 1\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT FOR PLAY CUE POSITION ON DECK 1\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    this->m_PlayPositionDesired1 = cuePosition1_double;
                    std::cout << "CUE POSITION: " +
                                    std::to_string(m_PlayPosition1->get()) +
                                    " DESIRED POSITION: " +
                                    std::to_string(
                                            this->m_PlayPositionDesired1) +
                                    "\n";

                    Playing1Queue = 1;
                    m_Playing1->set(1.0);
                    goto clean_exit;

                }

                else if (comando[1] == '2') {
                    std::string cuePosition2;
                    double cuePosition2_double;

                    for (long unsigned int usecamisinha = 2;
                            usecamisinha <= comando.length();
                            usecamisinha++) {
                        cuePosition2 = cuePosition2 + comando[usecamisinha];
                    }

                    try {
                        cuePosition2_double = std::stod(cuePosition2);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID ARGUMENT FOR PLAY CUE POSITION ON DECK 2\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE ARGUMENT FOR PLAY CUE POSITION ON DECK 2\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    this->m_PlayPositionDesired2 = cuePosition2_double;

                    std::cout << "CUE POSITION: " +
                                    std::to_string(m_PlayPosition2->get()) +
                                    " DESIRED POSITION: " +
                                    std::to_string(
                                            this->m_PlayPositionDesired2) +
                                    "\n";

                    Playing2Queue = 1;
                    m_Playing2->set(1.0);
                    goto clean_exit;
                }
            }

            else if (comando[0] == 'O') {
                if (comando[1] == '2') {
                    for (long unsigned int usecamisinha = 2; usecamisinha <= 7; usecamisinha++) {
                        loopin2 = loopin2 + comando[usecamisinha];
                    }

                    for (long unsigned int usecamisinha = 8; usecamisinha <= 13; usecamisinha++) {
                        loopout2 = loopout2 + comando[usecamisinha];
                    }

                    try {
                        loopin_double2 = std::stod("0." + loopin2);
                        loopout_double2 = std::stod("0." + loopout2);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID LOOPING POSITIONS FOR DECK 2\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE LOOPING POSITIONS FOR DECK 2\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    this->WIP2 = 1;

                    this->gambi_loopin2 = loopin_double2;
                    this->gambi_loopout2 = loopout_double2;

                    this->WIP2 = 5;

                    goto clean_exit;
                }

                if (comando[1] == '1') {
                    for (long unsigned int usecamisinha = 2; usecamisinha <= 7; usecamisinha++) {
                        loopin1 = loopin1 + comando[usecamisinha];
                    }

                    for (long unsigned int usecamisinha = 8; usecamisinha <= 13; usecamisinha++) {
                        loopout1 = loopout1 + comando[usecamisinha];
                    }

                    try {
                        loopin_double1 = std::stod("0." + loopin1);
                        loopout_double1 = std::stod("0." + loopout1);
                    }

                    catch (const std::invalid_argument& e) {
                        this->wuwei = true;
                        std::cout << "INVALID LOOPING POSITIONS FOR DECK 1\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    catch (const std::out_of_range& e) {
                        this->wuwei = true;
                        std::cout << "OUT OF RANGE LOOPING POSITIONS FOR DECK 1\n";
                        this->LOCK = false;
                        goto clean_exit;
                    }

                    this->WIP1 = 1;

                    this->gambi_loopin1 = loopin_double1;
                    this->gambi_loopout1 = loopout_double1;

                    this->WIP1 = 5;

                    goto clean_exit;
                }
            }

            else if (comando[0] == 'Q') {
                if (comando[1] == '1') {
                    if (comando[2] == 'H') {
                        if (comando[3] == 'O') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 1 EQ HIGH OPEN\n";
                                this->DECK_1_Q_H_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 1 EQ HIGH OPEN\n";
                                this->DECK_1_Q_H_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_1_HIGH->get() < 1) {
                                this->DECK_1_Q_H_O_B = true;

                                this->crescendo_EQ_1_HIGH = slopenumerico / 1000;
                                this->DECK_1_Q_H_O_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }

                        else if (comando[3] == 'C') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 1 EQ HIGH CLOSE\n";
                                this->DECK_1_Q_H_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 1 EQ HIGH CLOSE\n";
                                this->DECK_1_Q_H_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_1_HIGH->get() > 0) {
                                this->DECK_1_Q_H_C_B = true;

                                this->diminuendo_EQ_1_HIGH = slopenumerico / 1000;
                                this->DECK_1_Q_H_C_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }
                    }

                    else if (comando[2] == 'M') {
                        if (comando[3] == 'O') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 1 EQ MID OPEN\n";
                                this->DECK_1_Q_M_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 1 EQ MID OPEN\n";
                                this->DECK_1_Q_M_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_1_MID->get() < 1) {
                                this->DECK_1_Q_M_O_B = true;

                                this->crescendo_EQ_1_MID = slopenumerico / 1000;
                                this->DECK_1_Q_M_O_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }

                        else if (comando[3] == 'C') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 1 EQ MID CLOSE\n";
                                this->DECK_1_Q_M_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 1 EQ MID CLOSE\n";
                                this->DECK_1_Q_M_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_1_MID->get() > 0) {
                                this->DECK_1_Q_M_C_B = true;

                                this->diminuendo_EQ_1_MID = slopenumerico / 1000;
                                this->DECK_1_Q_M_C_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }
                    }

                    else if (comando[2] == 'L') {
                        if (comando[3] == 'O') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 1 EQ LOW OPEN\n";
                                this->DECK_1_Q_L_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 1 EQ LOW OPEN\n";
                                this->DECK_1_Q_L_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_1_LOW->get() < 1) {
                                this->DECK_1_Q_L_O_B = true;

                                this->crescendo_EQ_1_LOW = slopenumerico / 1000;
                                this->DECK_1_Q_L_O_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }

                        else if (comando[3] == 'C') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 1 EQ LOW CLOSE\n";
                                this->DECK_1_Q_L_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 1 EQ LOW CLOSE\n";
                                this->DECK_1_Q_L_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_1_LOW->get() > 0) {
                                this->DECK_1_Q_L_C_B = true;

                                this->diminuendo_EQ_1_LOW = slopenumerico / 1000;
                                this->DECK_1_Q_L_C_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }
                    }
                }

                else if (comando[1] == '2') {
                    if (comando[2] == 'H') {
                        if (comando[3] == 'O') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 2 EQ HIGH OPEN\n";
                                this->DECK_2_Q_H_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ HIGH OPEN\n";
                                this->DECK_2_Q_H_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_2_HIGH->get() < 1) {
                                this->DECK_2_Q_H_O_B = true;

                                this->crescendo_EQ_2_HIGH = slopenumerico / 1000;
                                this->DECK_2_Q_H_O_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }

                        else if (comando[3] == 'C') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 2 EQ HIGH CLOSE\n";
                                this->DECK_2_Q_H_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ HIGH CLOSE\n";
                                this->DECK_2_Q_H_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_2_HIGH->get() > 0) {
                                this->DECK_2_Q_H_C_B = true;

                                this->diminuendo_EQ_2_HIGH = slopenumerico / 1000;
                                this->DECK_2_Q_H_C_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }
                    }

                    else if (comando[2] == 'M') {
                        if (comando[3] == 'O') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 2 EQ MID OPEN\n";
                                this->DECK_2_Q_M_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ MID OPEN\n";
                                this->DECK_2_Q_M_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_2_MID->get() < 1) {
                                this->DECK_2_Q_M_O_B = true;

                                this->crescendo_EQ_2_MID = slopenumerico / 1000;
                                this->DECK_2_Q_M_O_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }

                        else if (comando[3] == 'C') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 2 EQ MID CLOSE\n";
                                this->DECK_2_Q_M_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ MID CLOSE\n";
                                this->DECK_2_Q_M_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_2_MID->get() > 0) {
                                this->DECK_2_Q_M_C_B = true;

                                this->diminuendo_EQ_2_MID = slopenumerico / 1000;
                                this->DECK_2_Q_M_C_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }
                    }

                    else if (comando[2] == 'L') {
                        if (comando[3] == 'O') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "INVALID ARGUMENT FOR DECK 2 EQ LOW OPEN\n";
                                this->DECK_2_Q_L_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ LOW OPEN\n";
                                this->DECK_2_Q_L_O_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_2_LOW->get() < 1) {
                                this->DECK_2_Q_L_O_B = true;

                                this->crescendo_EQ_2_LOW = slopenumerico / 1000;
                                this->DECK_2_Q_L_O_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }

                        else if (comando[3] == 'C') {
                            for (long unsigned int usecamisinha = 4;
                                    usecamisinha <= comando.length();
                                    usecamisinha++) {
                                slope = slope + comando[usecamisinha];
                            }

                            try {
                                slopenumerico = std::stod(slope);
                            }

                            catch (const std::invalid_argument& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ LOW CLOSE\n";
                                this->DECK_2_Q_L_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            catch (const std::out_of_range& e) {
                                this->wuwei = true;
                                std::cout << "OUT OF RANGE ARGUMENT FOR DECK 2 EQ LOW CLOSE\n";
                                this->DECK_2_Q_L_C_B = false;
                                this->LOCK = false;
                                goto clean_exit;
                            }

                            if (m_EQ_2_LOW->get() > 0) {
                                this->DECK_2_Q_L_C_B = true;

                                this->diminuendo_EQ_2_LOW = slopenumerico / 1000;
                                this->DECK_2_Q_L_C_V = 1000;

                                confirmado.open("/home/dumbo/confirmixxx.txt");
                                confirmado << std::to_string(this->counter) + "\n";
                                confirmado.close();

                                this->counter++;
                                this->LOCK = false;
                            }
                        }
                    }
                }
            }
        }
    }

    if (this->DECK_1_Q_L_C_B == true && this->DECK_1_Q_L_C_V > 0) {
        m_EQ_1_LOW->set(m_EQ_1_LOW->get() - this->diminuendo_EQ_1_LOW);
        this->DECK_1_Q_L_C_V--;
    }

    if (this->DECK_1_Q_L_O_B == true && this->DECK_1_Q_L_O_V > 0) {
        m_EQ_1_LOW->set(m_EQ_1_LOW->get() + this->crescendo_EQ_1_LOW);
        this->DECK_1_Q_L_O_V--;
    }

    if (this->DECK_1_Q_M_C_B == true && this->DECK_1_Q_M_C_V > 0) {
        m_EQ_1_MID->set(m_EQ_1_MID->get() - this->diminuendo_EQ_1_MID);
        this->DECK_1_Q_M_C_V--;
    }

    if (this->DECK_1_Q_M_O_B == true && this->DECK_1_Q_M_O_V > 0) {
        m_EQ_1_MID->set(m_EQ_1_MID->get() + this->crescendo_EQ_1_MID);
        this->DECK_1_Q_M_O_V--;
    }

    if (this->DECK_1_Q_H_C_B == true && this->DECK_1_Q_H_C_V > 0) {
        m_EQ_1_HIGH->set(m_EQ_1_HIGH->get() - this->diminuendo_EQ_1_HIGH);
        this->DECK_1_Q_H_C_V--;
    }

    if (this->DECK_1_Q_H_O_B == true && this->DECK_1_Q_H_O_V > 0) {
        m_EQ_1_HIGH->set(m_EQ_1_HIGH->get() + this->crescendo_EQ_1_HIGH);
        this->DECK_1_Q_H_O_V--;
    }

    if (this->DECK_2_Q_L_C_B == true && this->DECK_2_Q_L_C_V > 0) {
        m_EQ_2_LOW->set(m_EQ_2_LOW->get() - this->diminuendo_EQ_2_LOW);
        this->DECK_2_Q_L_C_V--;
    }

    if (this->DECK_2_Q_L_O_B == true && this->DECK_2_Q_L_O_V > 0) {
        m_EQ_2_LOW->set(m_EQ_2_LOW->get() + this->crescendo_EQ_2_LOW);
        this->DECK_2_Q_L_O_V--;
    }

    if (this->DECK_2_Q_M_C_B == true && this->DECK_2_Q_M_C_V > 0) {
        m_EQ_2_MID->set(m_EQ_2_MID->get() - this->diminuendo_EQ_2_MID);
        this->DECK_2_Q_M_C_V--;
    }

    if (this->DECK_2_Q_M_O_B == true && this->DECK_2_Q_M_O_V > 0) {
        m_EQ_2_MID->set(m_EQ_2_MID->get() + this->crescendo_EQ_2_MID);
        this->DECK_2_Q_M_O_V--;
    }

    if (this->DECK_2_Q_H_C_B == true && this->DECK_2_Q_H_C_V > 0) {
        m_EQ_2_HIGH->set(m_EQ_2_HIGH->get() - this->diminuendo_EQ_2_HIGH);
        this->DECK_2_Q_H_C_V--;
    }

    if (this->DECK_2_Q_H_O_B == true && this->DECK_2_Q_H_O_V > 0) {
        m_EQ_2_HIGH->set(m_EQ_2_HIGH->get() + this->crescendo_EQ_2_HIGH);
        this->DECK_2_Q_H_O_V--;
    }

    if (this->CROSSFADER_X_L_B == true && this->CROSSFADER_X_V > 0) {
        if (m_pCOCrossfader->get() >= 1.0) {
            m_photCue21Clear->set(1.0);
            m_photCue21Set->set(1.0);
        }

        m_pCOCrossfader->set(m_pCOCrossfader->get() - diminuendo_CROSS_X);
        this->CROSSFADER_X_V--;
    }

    if (this->CROSSFADER_X_R_B == true && this->CROSSFADER_X_V > 0) {
        if (m_pCOCrossfader->get() <= -1.0) {
            m_photCue11Clear->set(1.0);
            m_photCue11Set->set(1.0);
        }

        m_pCOCrossfader->set(m_pCOCrossfader->get() + crescendo_CROSS_X);
        this->CROSSFADER_X_V--;
    }


    if (this->DECK_1_Q_L_C_B == true && (this->DECK_1_Q_L_C_V <= 0 || m_EQ_1_LOW->get() <= 0)) {
        this->DECK_1_Q_L_C_B = false;
    }

    if (this->DECK_1_Q_L_O_B == true && (this->DECK_1_Q_L_O_V <= 0 || m_EQ_1_LOW->get() >= 1)) {
        this->DECK_1_Q_L_O_B = false;
    }

    if (this->DECK_1_Q_M_C_B == true && (this->DECK_1_Q_M_C_V <= 0 || m_EQ_1_MID->get() <= 0)) {
        this->DECK_1_Q_M_C_B = false;
    }

    if (this->DECK_1_Q_M_O_B == true && (this->DECK_1_Q_M_O_V <= 0 || m_EQ_1_MID->get() >= 1)) {
        this->DECK_1_Q_M_O_B = false;
    }

    if (this->DECK_1_Q_H_C_B == true && (this->DECK_1_Q_H_C_V <= 0 || m_EQ_1_HIGH->get() <= 0)) {
        this->DECK_1_Q_H_C_B = false;
    }

    if (this->DECK_1_Q_H_O_B == true && (this->DECK_1_Q_H_O_V <= 0 || m_EQ_1_HIGH->get() >= 1)) {
        this->DECK_1_Q_H_O_B = false;
    }

    if (this->DECK_2_Q_L_C_B == true && (this->DECK_2_Q_L_C_V <= 0 || m_EQ_2_LOW->get() <= 0)) {
        this->DECK_2_Q_L_C_B = false;
    }

    if (this->DECK_2_Q_L_O_B == true && (this->DECK_2_Q_L_O_V <= 0 || m_EQ_2_LOW->get() >= 1)) {
        this->DECK_2_Q_L_O_B = false;
    }

    if (this->DECK_2_Q_M_C_B == true && (this->DECK_2_Q_M_C_V <= 0 || m_EQ_2_MID->get() <= 0)) {
        this->DECK_2_Q_M_C_B = false;
    }

    if (this->DECK_2_Q_M_O_B == true && (this->DECK_2_Q_M_O_V <= 0 || m_EQ_2_MID->get() >= 1)) {
        this->DECK_2_Q_M_O_B = false;
    }

    if (this->DECK_2_Q_H_C_B == true && (this->DECK_2_Q_H_C_V <= 0 || m_EQ_2_HIGH->get() <= 0)) {
        this->DECK_2_Q_H_C_B = false;
    }

    if (this->DECK_2_Q_H_O_B == true && (this->DECK_2_Q_H_O_V <= 0 || m_EQ_2_HIGH->get() >= 1)) {
        this->DECK_2_Q_H_O_B = false;
    }

    if (this->CROSSFADER_X_L_B == true &&
            (this->CROSSFADER_X_V <= 0 || m_pCOCrossfader->get() <= -1.0)) {
        this->CROSSFADER_X_L_B = false;
        std::cout << "CROSSFADE DONE. STOPPING DECK 2\n";

        m_Playing2->set(0.0);
    }

    if (this->CROSSFADER_X_R_B == true &&
            (this->CROSSFADER_X_V <= 0 || m_pCOCrossfader->get() >= 1.0)) {
        this->CROSSFADER_X_R_B = false;

        std::cout << "CROSSFADE DONE. STOPPING DECK 1\n";
        m_Playing1->set(0.0);
    }

    if (m_Playing1->get() == 1.0) {
        double playPosition1 = m_PlayPosition1->get();

        confirmado.open("/home/dumbo/mixxxposition1.txt");
        confirmado << std::to_string(playPosition1) + "\n";
        confirmado.close();
    }

    if (m_Playing2->get() == 1.0) {
        double playPosition2 = m_PlayPosition2->get();

        confirmado.open("/home/dumbo/mixxxposition2.txt");
        confirmado << std::to_string(playPosition2) + "\n";
        confirmado.close();
    }

clean_exit:
    delete m_pCue1;
    delete m_pCue2;
    delete m_photCue11Set;
    delete m_photCue21Set;
    delete m_photCue11Clear;
    delete m_photCue21Clear;
    delete m_trackSamples1;
    delete m_trackSamples2;

    delete m_LoopIn1;
    delete m_LoopOut1;
    delete m_LoopToggle1;

    delete m_LoopIn2;
    delete m_LoopOut2;
    delete m_LoopToggle2;

    delete m_PlayPosition1;
    delete m_Playing1;

    delete m_PlayPosition2;
    delete m_Playing2;

    delete m_EQ_1_LOW;
    delete m_EQ_1_MID;
    delete m_EQ_1_HIGH;

    delete m_EQ_2_LOW;
    delete m_EQ_2_MID;
    delete m_EQ_2_HIGH;

    return;
}

TrackPointer AutoDJProcessor::getNextTrackFromQueue() {
    // Get the track at the top of the playlist.
    bool randomQueueEnabled = m_pConfig->getValue<bool>(
            ConfigKey("[Auto DJ]", "EnableRandomQueue"));
    int minAutoDJCrateTracks = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "RandomQueueMinimumAllowed")).toInt();
    int tracksToAdd = minAutoDJCrateTracks - m_pAutoDJTableModel->rowCount();
    // In case we start off with < minimum tracks
    if (randomQueueEnabled && (tracksToAdd > 0)) {
        emit randomTrackRequested(tracksToAdd);
    }

    while (true) {
        TrackPointer nextTrack = m_pAutoDJTableModel->getTrack(
            m_pAutoDJTableModel->index(0, 0));

        if (nextTrack) {
            if (nextTrack->getFileInfo().checkFileExists()) {
                return nextTrack;
            } else {
                // Remove missing song from auto DJ playlist.
                m_pAutoDJTableModel->removeTrack(
                        m_pAutoDJTableModel->index(0, 0));
            }
        } else {
            // We're out of tracks. Return the null TrackPointer.
            return nextTrack;
        }
    }
}

bool AutoDJProcessor::loadNextTrackFromQueue(const DeckAttributes& deck, bool play) {
    TrackPointer nextTrack = getNextTrackFromQueue();

    // We ran out of tracks in the queue.
    if (!nextTrack) {
        // Disable AutoDJ.
        toggleAutoDJ(false);

        // And eject track (nextTrack is null) as "End of auto DJ warning"
        emitLoadTrackToPlayer(nextTrack, deck.group, false);
        return false;
    }

    emitLoadTrackToPlayer(nextTrack, deck.group, play);
    return true;
}

bool AutoDJProcessor::removeLoadedTrackFromTopOfQueue(const DeckAttributes& deck) {
    return removeTrackFromTopOfQueue(deck.getLoadedTrack());
}

bool AutoDJProcessor::removeTrackFromTopOfQueue(TrackPointer pTrack) {
    // No track to test for.
    if (!pTrack) {
        return false;
    }

    TrackId trackId(pTrack->getId());

    // Loaded track is not a library track.
    if (!trackId.isValid()) {
        return false;
    }

    // Get the track id at the top of the playlist.
    TrackId nextId(m_pAutoDJTableModel->getTrackId(
            m_pAutoDJTableModel->index(0, 0)));

    // No track at the top of the queue.
    if (!nextId.isValid()) {
        return false;
    }

    // If the loaded track is not the next track in the queue then do nothing.
    if (trackId != nextId) {
        return false;
    }

    // Remove the top track.
    m_pAutoDJTableModel->removeTrack(m_pAutoDJTableModel->index(0, 0));

    // Re-queue if configured.
    if (m_pConfig->getValueString(ConfigKey(kConfigKey, "Requeue")).toInt()) {
        m_pAutoDJTableModel->appendTrack(nextId);
    }

    maybeFillRandomTracks();
    return true;
}

void AutoDJProcessor::maybeFillRandomTracks() {
    int minAutoDJCrateTracks = m_pConfig->getValueString(
            ConfigKey(kConfigKey, "RandomQueueMinimumAllowed")).toInt();
    bool randomQueueEnabled = (((m_pConfig->getValueString(
            ConfigKey("[Auto DJ]", "EnableRandomQueue")).toInt())) == 1);

    int tracksToAdd = minAutoDJCrateTracks - m_pAutoDJTableModel->rowCount();
    if (randomQueueEnabled && (tracksToAdd > 0)) {
        qDebug() << "Randomly adding tracks";
        emit randomTrackRequested(tracksToAdd);
    }
}

void AutoDJProcessor::playerPlayChanged(DeckAttributes* thisDeck, bool playing) {
    if constexpr (sDebug) {
        qDebug() << this << "playerPlayChanged" << thisDeck->group << playing;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    if (thisDeck->loading) {
        // Note: When loading a new deck this signal arrives before the
        // playerTrackLoaded();
        return;
    }

    DeckAttributes* otherDeck = getOtherDeck(thisDeck);
    if (!otherDeck) {
        // This happens if all decks have center orientation
        return;
    }

    if (playing) {
        if (!otherDeck->isPlaying()) {
            // In case both decks were stopped and now this one just started, make
            // this deck the "from deck".
            calculateTransition(thisDeck, getOtherDeck(thisDeck), false);
        }
    } else {
        // Deck paused
        // This may happen if the user has previously pressed play on the "to deck"
        // before fading, for example to adjust the intro/outro cues, and lets the
        // deck play until the end, seek back to the start point instead of keeping
        if (thisDeck->playPosition() >= 1.0 && !thisDeck->isFromDeck) {
            // toDeck has stopped at the end. Recalculate the transition, because
            // it has been done from a now irrelevant previous position.
            // This forces the other deck to be the fromDeck.
            thisDeck->startPos = kKeepPosition;
            calculateTransition(otherDeck, thisDeck, true);
            if (thisDeck->startPos != kKeepPosition) {
                // Note: this seek will trigger the playerPositionChanged slot
                // which may calls the calculateTransition() again without seek = true;
                thisDeck->setPlayPosition(thisDeck->startPos);
            }
        }
    }
}

void AutoDJProcessor::playerIntroStartChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerIntroStartChanged" << pAttributes->group << position;
    }
    // nothing to do, because we want not to re-cue the toDeck and the from
    // Deck has already passed the intro
}

void AutoDJProcessor::playerIntroEndChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerIntroEndChanged" << pAttributes->group << position;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    if (pAttributes->isFromDeck) {
        // We have already passed the intro
        return;
    }
    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

void AutoDJProcessor::playerOutroStartChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerOutroStartChanged" << pAttributes->group << position;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

void AutoDJProcessor::playerOutroEndChanged(DeckAttributes* pAttributes, double position) {
    if constexpr (sDebug) {
        qDebug() << this << "playerOutroEndChanged" << pAttributes->group << position;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

double AutoDJProcessor::getIntroStartSecond(DeckAttributes* pDeck) {
    const mixxx::audio::FramePos introStartPosition = pDeck->introStartPosition();
    if (!introStartPosition.isValid()) {
        return getFirstSoundSecond(pDeck);
    }
    return framePositionToSeconds(introStartPosition, pDeck);
}

double AutoDJProcessor::getIntroEndSecond(DeckAttributes* pDeck) {
    const mixxx::audio::FramePos introEndPosition = pDeck->introEndPosition();
    if (!introEndPosition.isValid()) {
        // Assume a zero length intro if introEnd is not set.
        // The introStart is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed outroStart. If it has
        // not been placed, getIntroStartPosition will return 0:00.
        return getIntroStartSecond(pDeck);
    }
    return framePositionToSeconds(introEndPosition, pDeck);
}

double AutoDJProcessor::getOutroStartSecond(DeckAttributes* pDeck) {
    const mixxx::audio::FramePos outroStartPosition = pDeck->outroStartPosition();
    if (!outroStartPosition.isValid()) {
        // Assume a zero length outro if outroStart is not set.
        // The outroEnd is automatically placed by AnalyzerSilence, so use
        // that as a fallback if the user has not placed outroStart. If it has
        // not been placed, getOutroEndPosition will return the end of the track.
        return getOutroEndSecond(pDeck);
    }
    return framePositionToSeconds(outroStartPosition, pDeck);
}

double AutoDJProcessor::getOutroEndSecond(DeckAttributes* pDeck) {
    const mixxx::audio::FramePos outroEndPosition = pDeck->outroEndPosition();
    if (!outroEndPosition.isValid()) {
        return getLastSoundSecond(pDeck);
    }
    return framePositionToSeconds(outroEndPosition, pDeck);
    ;
}

double AutoDJProcessor::getFirstSoundSecond(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    CuePointer pFromTrackN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);
    if (pFromTrackN60dBSound) {
        const mixxx::audio::FramePos firstSound = pFromTrackN60dBSound->getPosition();
        if (firstSound.isValid()) {
            return framePositionToSeconds(firstSound, pDeck);
        }
    }
    return 0.0;
}

double AutoDJProcessor::getLastSoundSecond(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    CuePointer pFromTrackN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);
    if (pFromTrackN60dBSound) {
        Cue::StartAndEndPositions pos = pFromTrackN60dBSound->getStartAndEndPosition();
        if (pos.endPosition > mixxx::audio::kStartFramePos &&
                (pos.endPosition - pos.startPosition) > 0) {
            return framePositionToSeconds(pos.endPosition, pDeck);
        }
    }
    return getEndSecond(pDeck);
}

double AutoDJProcessor::getEndSecond(DeckAttributes* pDeck) {
    TrackPointer pTrack = pDeck->getLoadedTrack();
    if (!pTrack) {
        return 0.0;
    }

    mixxx::audio::FramePos trackEndPosition = pDeck->trackEndPosition();
    return framePositionToSeconds(trackEndPosition, pDeck);
}

double AutoDJProcessor::framePositionToSeconds(
        mixxx::audio::FramePos position, DeckAttributes* pDeck) {
    mixxx::audio::SampleRate sampleRate = pDeck->sampleRate();
    if (!sampleRate.isValid() || !position.isValid()) {
        return 0.0;
    }

    return position.value() / sampleRate / pDeck->rateRatio();
}

void AutoDJProcessor::calculateTransition(DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck,
        bool seekToStartPoint) {
    VERIFY_OR_DEBUG_ASSERT(pFromDeck && pToDeck) {
        return;
    }
    if (pFromDeck->loading || pToDeck->loading) {
        // don't use halve new halve old data during
        // changing of tracks
        return;
    }

    // We require ADJ_IDLE to prevent changing the thresholds in the middle of a
    // fade.
    VERIFY_OR_DEBUG_ASSERT(m_eState == ADJ_IDLE) {
        return;
    }

    const double fromDeckEndPosition = getEndSecond(pFromDeck);
    const double toDeckEndPosition = getEndSecond(pToDeck);
    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration. Use this alias for better readability.
    const double fromDeckDuration = fromDeckEndPosition;
    const double toDeckDuration = toDeckEndPosition;

    VERIFY_OR_DEBUG_ASSERT(fromDeckDuration >= kMinimumTrackDurationSec) {
        // Track has no duration or too short. This should not happen, because short
        // tracks are skipped after load. Play ToDeck immediately.
        pFromDeck->fadeBeginPos = 0;
        pFromDeck->fadeEndPos = 0;
        pToDeck->startPos = kKeepPosition;
        return;
    }
    if (toDeckDuration == 0) {
        // This is a seek call to zero after ejecting the track
        // this signal is received before the track pointer becomes null
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(toDeckDuration >= kMinimumTrackDurationSec) {
        // Track has no duration or too short. This should not happen, because short
        // tracks are skipped after load.
        loadNextTrackFromQueue(*pToDeck, false);
        return;
    }

    // Within this function, the outro refers to the outro of the currently
    // playing track and the intro refers to the intro of the next track.

    double outroEnd = getOutroEndSecond(pFromDeck);
    double outroStart = getOutroStartSecond(pFromDeck);
    const double fromDeckPosition = fromDeckDuration * pFromDeck->playPosition();

    VERIFY_OR_DEBUG_ASSERT(outroEnd <= fromDeckEndPosition) {
        outroEnd = fromDeckEndPosition;
    }

    if (fromDeckPosition > outroStart) {
        // We have already passed outroStart
        // This can happen if we have just enabled auto DJ
        outroStart = fromDeckPosition;
        if (fromDeckPosition > outroEnd) {
            outroEnd = math_min(outroStart + fabs(m_transitionTime), fromDeckEndPosition);
        }
    }
    double outroLength = outroEnd - outroStart;

    double toDeckPositionSeconds = toDeckDuration * pToDeck->playPosition();
    // Store here a possible fadeBeginPos for the transition after next
    // This is used to check if it will be possible or a re-cue is required.
    // here it is done for FullIntroOutro and FadeAtOutroStart.
    // It is adjusted below for the other modes.
    pToDeck->fadeEndPos = getOutroEndSecond(pToDeck);
    double toDeckOutroStartSecond = getOutroStartSecond(pToDeck);
    if (pToDeck->fadeEndPos == toDeckOutroStartSecond) {
        // outro not defined, use transition time.
        toDeckOutroStartSecond -= m_transitionTime;
    }
    pToDeck->fadeBeginPos = toDeckOutroStartSecond;

    double introStart;
    if (seekToStartPoint || toDeckPositionSeconds >= pToDeck->fadeBeginPos) {
        // toDeckPosition >= pToDeck->fadeBeginPos happens when the
        // user has seeked or played the to track behind fadeBeginPos of
        // the fade after the next.
        // In this case we recue the track just before the transition.
        introStart = getIntroStartSecond(pToDeck);
    } else {
        introStart = toDeckPositionSeconds;
    }

    double introLength = 0;
    const mixxx::audio::FramePos introEndPosition = pToDeck->introEndPosition();
    if (introEndPosition.isValid()) {
        const double introEnd = framePositionToSeconds(introEndPosition, pToDeck);
        if (introStart < introEnd) {
            introLength = introEnd - introStart;
        }
    }

    if constexpr (sDebug) {
        qDebug() << this << "calculateTransition"
                 << "introLength" << introLength
                 << "outroLength" << outroLength;
    }

    switch (m_transitionMode) {
    case TransitionMode::FullIntroOutro: {
        // Use the outro or intro length for the transition time, whichever is
        // shorter. Let the full outro and intro play; do not cut off any part
        // of either.
        //
        // In the diagrams below,
        // - is part of a track outside the outro/intro,
        // o is part of the outro
        // i is part of the intro
        // | marks the boundaries of the transition
        //
        // When outro > intro:
        // ------ooo|ooo|
        //          |iii|------
        //
        // When outro < intro:
        // ------|ooo|
        //       |iii|iii-----
        //
        // If only the outro or intro length is marked but not both, use the one
        // that is marked for the transition time. If neither is marked, fall
        // back to the transition time from the spinbox.
        double transitionLength = introLength;
        if (outroLength > 0) {
            if (transitionLength <= 0 || transitionLength > outroLength) {
                // Use outro length when the intro is not defined or longer
                // than the outro.
                transitionLength = outroLength;
            }
        }
        if (transitionLength > 0) {
            const double transitionEnd = introStart + transitionLength;
            if (transitionEnd > pToDeck->fadeBeginPos) {
                // End intro before next outro starts
                transitionLength = pToDeck->fadeBeginPos - introStart;
                VERIFY_OR_DEBUG_ASSERT(transitionLength > 0) {
                    // We seek to intro start above in this case so this never happens
                    transitionLength = 1;
                }
            }
            pFromDeck->fadeBeginPos = outroEnd - transitionLength;
            pFromDeck->fadeEndPos = outroEnd;
            pToDeck->startPos = introStart;
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, outroEnd, introStart);
        }
    } break;
    case TransitionMode::FadeAtOutroStart: {
        // Use the outro or intro length for the transition time, whichever is
        // shorter. If the outro is longer than the intro, cut off the end
        // of the outro.
        //
        // In the diagrams below,
        // - is part of a track outside the outro/intro,
        // o is part of the outro
        // i is part of the intro
        // | marks the boundaries of the transition
        //
        // When outro > intro:
        // ------|ooo|ooo
        //       |iii|------
        //
        // When outro < intro:
        // ------|ooo|
        //       |iii|iii-----
        //
        // If only the outro or intro length is marked but not both, use the one
        // that is marked for the transition time. If neither is marked, fall
        // back to the transition time from the spinbox.
        double transitionLength = outroLength;
        if (transitionLength > 0) {
            if (introLength > 0) {
                if (outroLength > introLength) {
                    // Cut off end of outro
                    transitionLength = introLength;
                }
            }
            const double transitionEnd = introStart + transitionLength;
            if (transitionEnd > pToDeck->fadeBeginPos) {
                // End intro before next outro starts
                transitionLength = pToDeck->fadeBeginPos - introStart;
                VERIFY_OR_DEBUG_ASSERT(transitionLength > 0) {
                    // We seek to intro start above in this case so this never happens
                    transitionLength = 1;
                }
            }
            pFromDeck->fadeBeginPos = outroStart;
            pFromDeck->fadeEndPos = outroStart + transitionLength;
            pToDeck->startPos = introStart;
        } else if (introLength > 0) {
            transitionLength = introLength;
            pFromDeck->fadeBeginPos = outroEnd - transitionLength;
            pFromDeck->fadeEndPos = outroEnd;
            pToDeck->startPos = introStart;
        } else {
            useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, outroEnd, introStart);
        }
    } break;
    case TransitionMode::FixedSkipSilence: {
        double toDeckStartSecond;
        pToDeck->fadeBeginPos = getLastSoundSecond(pToDeck);
        if (seekToStartPoint || toDeckPositionSeconds >= pToDeck->fadeBeginPos) {
            // toDeckPosition >= pToDeck->fadeBeginPos happens when the
            // user has seeked or played the to track behind fadeBeginPos of
            // the fade after the next.
            // In this case we recue the track just before the transition.
            toDeckStartSecond = getFirstSoundSecond(pToDeck);
        } else {
            toDeckStartSecond = toDeckPositionSeconds;
        }
        useFixedFadeTime(
                pFromDeck,
                pToDeck,
                fromDeckPosition,
                getLastSoundSecond(pFromDeck),
                toDeckStartSecond);
    } break;
    case TransitionMode::FixedFullTrack:
    default: {
        double startPoint;
        pToDeck->fadeBeginPos = toDeckEndPosition;
        if (seekToStartPoint || toDeckPositionSeconds >= pToDeck->fadeBeginPos) {
            // toDeckPosition >= pToDeck->fadeBeginPos happens when the
            // user has seeked or played the to track behind fadeBeginPos of
            // the fade after the next.
            // In this case we recue the track just before the transition.
            startPoint = 0.0;
        } else {
            startPoint = toDeckPositionSeconds;
        }
        useFixedFadeTime(pFromDeck, pToDeck, fromDeckPosition, fromDeckEndPosition, startPoint);
        }
    }

    // These are expected to be a fraction of the track length.
    pFromDeck->fadeBeginPos /= fromDeckDuration;
    pFromDeck->fadeEndPos /= fromDeckDuration;
    pToDeck->startPos /= toDeckDuration;
    pToDeck->fadeBeginPos /= toDeckDuration;
    pToDeck->fadeEndPos /= toDeckDuration;

    pFromDeck->isFromDeck = true;
    pToDeck->isFromDeck = false;

    VERIFY_OR_DEBUG_ASSERT(pFromDeck->fadeBeginPos <= 1) {
        pFromDeck->fadeBeginPos = 1;
    }

    if constexpr (sDebug) {
        qDebug() << this << "calculateTransition" << pFromDeck->group
                 << pFromDeck->fadeBeginPos << pFromDeck->fadeEndPos
                 << pToDeck->startPos;
    }
}

void AutoDJProcessor::useFixedFadeTime(
        DeckAttributes* pFromDeck,
        DeckAttributes* pToDeck,
        double fromDeckSecond,
        double fadeEndSecond,
        double toDeckStartSecond) {
    if (m_transitionTime > 0.0) {
        // Guard against the next track being too short. This transition must finish
        // before the next transition starts.
        double toDeckOutroStart = pToDeck->fadeBeginPos;
        if (pToDeck->fadeBeginPos >= pToDeck->fadeEndPos) {
            // no outro defined, the toDeck will also use the transition time
            toDeckOutroStart -= m_transitionTime;
        }
        if (toDeckOutroStart <= toDeckStartSecond + kMinimumTrackDurationSec) {
            // we have already passed the outro start
            // Check OutroEnd as alternative, which is for all transition mode
            // better than directly default to duration()
            double end = getOutroEndSecond(pToDeck);
            if (end <= toDeckStartSecond + kMinimumTrackDurationSec) {
                // we have also passed the outro end
                end = getEndSecond(pToDeck);
                VERIFY_OR_DEBUG_ASSERT(end > toDeckStartSecond + kMinimumTrackDurationSec) {
                    // as last resort move start point
                    // The caller makes sure that this never happens
                    toDeckStartSecond = end - kMinimumTrackDurationSec;
                }
            }
            // use the remaining time for fading
            toDeckOutroStart = (end - toDeckStartSecond) / 2 + toDeckStartSecond;
        }
        double transitionTime = math_min(toDeckOutroStart - toDeckStartSecond,
                m_transitionTime);
        VERIFY_OR_DEBUG_ASSERT(transitionTime >= kMinimumTrackDurationSec / 2) {
            transitionTime = kMinimumTrackDurationSec / 2;
        }
        // Note: pFromDeck->fadeBeginPos >= pFromDeck->fadeEndPos is handled in
        // playerPositionChanged() causing a jump cut.
        pFromDeck->fadeBeginPos = math_max(fadeEndSecond - transitionTime, fromDeckSecond);
        pFromDeck->fadeEndPos = fadeEndSecond;
        pToDeck->startPos = toDeckStartSecond;
    } else {
        pFromDeck->fadeBeginPos = fadeEndSecond;
        pFromDeck->fadeEndPos = fadeEndSecond;
        pToDeck->startPos = toDeckStartSecond + m_transitionTime;
    }
}

void AutoDJProcessor::playerTrackLoaded(DeckAttributes* pDeck, TrackPointer pTrack) {
    if constexpr (sDebug) {
        qDebug() << this << "playerTrackLoaded" << pDeck->group
                 << (pTrack ? pTrack->getLocation() : "(null)");
    }

    pDeck->loading = false;

    // Since the end position is measured in seconds from 0:00 it is also
    // the track duration.
    double duration = getEndSecond(pDeck);
    if (duration < kMinimumTrackDurationSec) {
        qWarning() << "Skip track with" << duration << "Duration"
                   << pTrack->getLocation();
        // Remove Tack with duration smaller than two callbacks
        removeTrackFromTopOfQueue(pTrack);

        // Load the next track. If we are the first AutoDJ track
        // (ADJ_ENABLE_P1LOADED state) then play the track.
        loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
    } else if (m_eState == ADJ_IDLE) {
        // this deck has just changed the track so it becomes the toDeck
        DeckAttributes* fromDeck = getOtherDeck(pDeck);
        // check if this deck has suitable alignment
        if (fromDeck && getOtherDeck(fromDeck) != pDeck) {
            if constexpr (sDebug) {
                qDebug() << this << "playerTrackLoaded()" << pDeck->group << "but not a toDeck";
            }
            // User has changed the orientation, disable Auto DJ
            toggleAutoDJ(false);
            return;
        }
        pDeck->startPos = kKeepPosition;
        calculateTransition(fromDeck, pDeck, true);
        if (pDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            pDeck->setPlayPosition(pDeck->startPos);
        }
        // we are her in the relative domain 0..1
        if (!fromDeck->isPlaying() && fromDeck->playPosition() >= 1.0) {
            // repeat a probably missed update
            playerPositionChanged(fromDeck, 1.0);
        }
    } else if (m_eState == ADJ_LEFT_FADING) {
        if (pDeck == getRightDeck()) {
            // restore the play state lost during loading
            pDeck->play();
        }
    } else if (m_eState == ADJ_RIGHT_FADING) {
        if (pDeck == getLeftDeck()) {
            // restore the play state lost during loading
            pDeck->play();
        }
    }
}

void AutoDJProcessor::playerLoadingTrack(DeckAttributes* pDeck,
        TrackPointer pNewTrack, TrackPointer pOldTrack) {
    if constexpr (sDebug) {
        qDebug() << this << "playerLoadingTrack" << pDeck->group
                 << "new:" << (pNewTrack ? pNewTrack->getLocation() : "(null)")
                 << "old:" << (pOldTrack ? pOldTrack->getLocation() : "(null)");
    }

    pDeck->loading = true;

    // The Deck is loading an new track

    // There are four conditions under which we load a track.
    // 1) We are enabling AutoDJ and no decks are playing. Mode is
    //    ADJ_ENABLE_P1LOADED.
    // 2) After #1, we load a track into the other deck. Mode is ADJ_IDLE.
    // 3) We are enabling AutoDJ and a single deck is playing. Mode is ADJ_IDLE.
    // 4) We have just completed fading from one deck to another. Mode is
    //    ADJ_IDLE.

    if (!pNewTrack) {
        // If a track is ejected because of a manual eject command or a load failure
        // this track seams to be undesired. Remove the bad track from the queue.
        removeTrackFromTopOfQueue(pOldTrack);

        // wait until the track is fully unloaded and the playerEmpty()
        // slot is called before load an alternative track.
    }
}

void AutoDJProcessor::playerEmpty(DeckAttributes* pDeck) {
    if constexpr (sDebug) {
        qDebug() << this << "playerEmpty()" << pDeck->group;
    }

    // The Deck has ejected a track and no new one is loaded
    // This happens if loading fails or the user manually ejected the track
    // and would normally stop the AutoDJ flow, which is not desired.
    // It should be safe to load a new track from the queue. The only case where
    // we request a load-and-play is case #1 currently so we can easily test for
    // this based on the mode.

    // Load the next track. If we are the first AutoDJ track
    // (ADJ_ENABLE_P1LOADED state) then play the track.
    loadNextTrackFromQueue(*pDeck, m_eState == ADJ_ENABLE_P1LOADED);
}

void AutoDJProcessor::playerRateChanged(DeckAttributes* pAttributes) {
    if constexpr (sDebug) {
        qDebug() << this << "playerRateChanged" << pAttributes->group;
    }

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    DeckAttributes* fromDeck = getFromDeck();
    if (!fromDeck) {
        return;
    }
    calculateTransition(fromDeck, getOtherDeck(fromDeck), false);
}

void AutoDJProcessor::setTransitionTime(int time) {
    if constexpr (sDebug) {
        qDebug() << this << "setTransitionTime" << time;
    }

    // Update the transition time first.
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionPreferenceName),
                   ConfigValue(time));
    m_transitionTime = time;

    // Then re-calculate fade thresholds for the decks.
    if (m_eState == ADJ_IDLE) {
        DeckAttributes* pLeftDeck = getLeftDeck();
        DeckAttributes* pRightDeck = getRightDeck();
        if (!pLeftDeck || !pRightDeck) {
            // User has changed the orientation, disable Auto DJ
            toggleAutoDJ(false);
            return;
        }
        if (pLeftDeck->isPlaying()) {
            calculateTransition(pLeftDeck, pRightDeck, false);
        }
        if (pRightDeck->isPlaying()) {
            calculateTransition(pRightDeck, pLeftDeck, false);
        }
    }
}

void AutoDJProcessor::setTransitionMode(TransitionMode newMode) {
    m_pConfig->set(ConfigKey(kConfigKey, kTransitionModePreferenceName),
            ConfigValue(static_cast<int>(newMode)));
    m_transitionMode = newMode;

    if (m_eState != ADJ_IDLE) {
        // We don't want to recalculate a running transition
        return;
    }

    // Then re-calculate fade thresholds for the decks.
    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();

    if (!pLeftDeck || !pRightDeck) {
        // User has changed the orientation, disable Auto DJ
        toggleAutoDJ(false);
        return;
    }

    if (pLeftDeck->isPlaying() && !pRightDeck->isPlaying()) {
        calculateTransition(pLeftDeck, pRightDeck, true);
        if (pRightDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            pRightDeck->setPlayPosition(pRightDeck->startPos);
        }
    } else if (pRightDeck->isPlaying() && pLeftDeck->isPlaying()) {
        calculateTransition(pRightDeck, pLeftDeck, true);
        if (pLeftDeck->startPos != kKeepPosition) {
            // Note: this seek will trigger the playerPositionChanged slot
            // which may calls the calculateTransition() again without seek = true;
            pLeftDeck->setPlayPosition(pLeftDeck->startPos);
        }
    } else {
        // user has manually started the other deck or stopped both.
        // don't know what to do.
    }
}

DeckAttributes* AutoDJProcessor::getLeftDeck() {
    // find first left deck
    for (const auto& pDeck : std::as_const(m_decks)) {
        if (pDeck->isLeft()) {
            return pDeck;
        }
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getRightDeck() {
    // find first right deck
    for (const auto& pDeck : std::as_const(m_decks)) {
        if (pDeck->isRight()) {
            return pDeck;
        }
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getOtherDeck(
        const DeckAttributes* pThisDeck) {
    if (pThisDeck->isLeft()) {
        return getRightDeck();
    }
    if (pThisDeck->isRight()) {
        return getLeftDeck();
    }
    return nullptr;
}

DeckAttributes* AutoDJProcessor::getFromDeck() {
    for (const auto& pDeck : std::as_const(m_decks)) {
        if (pDeck->isFromDeck) {
            return pDeck;
        }
    }
    return nullptr;
}

bool AutoDJProcessor::nextTrackLoaded() {
    if (m_eState == ADJ_DISABLED) {
        // AutoDJ always loads the top track (again) if enabled
        return false;
    }

    DeckAttributes* pLeftDeck = getLeftDeck();
    DeckAttributes* pRightDeck = getRightDeck();
    if (!pLeftDeck || !pRightDeck) {
        return false;
    }

    bool leftDeckPlaying = pLeftDeck->isPlaying();
    bool rightDeckPlaying = pRightDeck->isPlaying();

    // Calculate idle deck
    TrackPointer loadedTrack;
    if (leftDeckPlaying && !rightDeckPlaying) {
        loadedTrack = pRightDeck->getLoadedTrack();
    } else if (!leftDeckPlaying && rightDeckPlaying) {
        loadedTrack = pLeftDeck->getLoadedTrack();
    } else if (getCrossfader() < 0.0) {
        loadedTrack = pRightDeck->getLoadedTrack();
    } else {
        loadedTrack = pLeftDeck->getLoadedTrack();
    }

    return loadedTrack == getNextTrackFromQueue();
}
