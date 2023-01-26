#include "mixer/stem.h"
#include "track/track.h"
#include <QRegularExpression>
#include "moc_stem.cpp"

namespace {

const QRegularExpression kStemRegex(QStringLiteral("^\\[Stem(\\d+)\\]$"));

int extractIntFromRegex(const QRegularExpression& regex, const QString& group) {
    const QRegularExpressionMatch match = regex.match(group);
    DEBUG_ASSERT(match.isValid());
    if (!match.hasMatch()) {
        return false;
    }
    // The regex is expected to contain a single capture group with the number
    constexpr int capturedNumberIndex = 1;
    DEBUG_ASSERT(match.lastCapturedIndex() <= capturedNumberIndex);
    if (match.lastCapturedIndex() < capturedNumberIndex) {
        qWarning() << "No number found in group" << group;
        return false;
    }
    const QString capturedNumber = match.captured(capturedNumberIndex);
    DEBUG_ASSERT(!capturedNumber.isNull());
    bool okay = false;
    const int numberFromMatch = capturedNumber.toInt(&okay);
    VERIFY_OR_DEBUG_ASSERT(okay) {
       return false;
    }
    return numberFromMatch;
}

} //anonymous namespace

Stem::Stem(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMaster*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ true) {
    stemName = handleGroup.name();
}

void Stem::slotStemPlay(TrackPointer pTrack) {
        int stemNumber = extractIntFromRegex(kStemRegex, stemName);
        QString deckNumber;

        if (stemNumber <= 4) {
            deckNumber = "1";
        }

        else if (stemNumber > 4 && stemNumber <= 9) {
            deckNumber = "2";
        }

	else if (stemNumber > 9 && stemNumber <= 12) {
            deckNumber = "3";
        }

	else if (stemNumber > 12 && stemNumber <= 16) {
            deckNumber = "4";
        }

        ControlProxy* m_DeckPlayPosition = new ControlProxy(QString("[Channel") + deckNumber + QString("]"), "playposition");
        ControlProxy* m_DeckVolume = new ControlProxy(QString("[Channel") + deckNumber + QString("]"), "volume");
        ControlProxy* m_DeckFileBpm = new ControlProxy(QString("[Channel") + deckNumber + QString("]"), "file_bpm");
        ControlProxy* m_DeckBpm = new ControlProxy(QString("[Channel") + deckNumber + QString("]"), "bpm");
        ControlProxy* m_DeckReplayGain = new ControlProxy(QString("[Channel") + deckNumber + QString("]"), "replaygain");
        ControlProxy* m_DeckKeyLock = new ControlProxy(QString("[Channel") + deckNumber + QString("]"), "keylock");

        ControlProxy* m_StemPlayPosition = new ControlProxy(stemName, "playposition");
        ControlProxy* m_StemVolume = new ControlProxy(stemName, "volume");
        ControlProxy* m_StemPlay = new ControlProxy(stemName, "play");
        ControlProxy* m_StemBpm = new ControlProxy(stemName, "bpm");
        ControlProxy* m_StemReplayGain = new ControlProxy(stemName, "replaygain");
        ControlProxy* m_StemKeyLock = new ControlProxy(stemName, "keylock");

        pTrack->trySetBpm(m_DeckFileBpm->get());
        m_StemBpm->set(m_DeckBpm->get());

        m_StemKeyLock->set(m_DeckKeyLock->get());
	m_StemReplayGain->set(m_DeckReplayGain->get());
        m_StemPlayPosition->set(m_DeckPlayPosition->get());
        m_StemVolume->set(0.8);
        m_StemPlay->set(1.0);

	if (stemNumber % 4 == 0) {
	    if (deckNumber == "1") {
	        QTimer::singleShot(100, this, &Stem::slotMuteDeck1);
	    }
	    else if (deckNumber == "2") {
	        QTimer::singleShot(100, this, &Stem::slotMuteDeck2);
	    }
	    else if (deckNumber == "3") {
	        QTimer::singleShot(100, this, &Stem::slotMuteDeck3);
	    }
	    else if (deckNumber == "4") {
	        QTimer::singleShot(100, this, &Stem::slotMuteDeck4);
	    }
	}

        delete m_DeckPlayPosition;
        delete m_DeckVolume;
        delete m_DeckFileBpm;
        delete m_DeckBpm;
	delete m_DeckReplayGain;
	delete m_DeckKeyLock;
        delete m_StemPlayPosition;
        delete m_StemVolume;
        delete m_StemPlay;
        delete m_StemBpm;
	delete m_StemReplayGain;
	delete m_StemKeyLock;
}

void Stem::slotMuteDeck1() {
    ControlProxy* m_DeckVolume = new ControlProxy(QString("[Channel1]"), "volume");
    m_DeckVolume->set(0.0);
    delete m_DeckVolume;
}

void Stem::slotMuteDeck2() {
    ControlProxy* m_DeckVolume = new ControlProxy(QString("[Channel2]"), "volume");
    m_DeckVolume->set(0.0);
    delete m_DeckVolume;
}

void Stem::slotMuteDeck3() {
    ControlProxy* m_DeckVolume = new ControlProxy(QString("[Channel3]"), "volume");
    m_DeckVolume->set(0.0);
    delete m_DeckVolume;
}

void Stem::slotMuteDeck4() {
    ControlProxy* m_DeckVolume = new ControlProxy(QString("[Channel4]"), "volume");
    m_DeckVolume->set(0.0);
    delete m_DeckVolume;
}

