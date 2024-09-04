#include "mixer/deck.h"
#include "mixer/playermanager.h"

#include "track/track.h"
#include "control/controlobject.h"
#include "moc_deck.cpp"
#include <QRegularExpression>

namespace {

const QRegularExpression kDeckRegex(QStringLiteral("^\\[Channel(\\d+)\\]$"));
const QRegularExpression kFilenameRegex(QStringLiteral("([^\\/]+)\\.[^.\\/:*?\"<>|]+$"));

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

QString extractFilenameFromRegex(const QRegularExpression& regex, const QString& group) {
    const QRegularExpressionMatch match = regex.match(group);
    DEBUG_ASSERT(match.isValid());
    if (!match.hasMatch()) {
        return "ERR_NO_FILE";
    }
    // The regex is expected to contain a single capture group with the number
    constexpr int capturedNumberIndex = 1;
    DEBUG_ASSERT(match.lastCapturedIndex() <= capturedNumberIndex);
    if (match.lastCapturedIndex() < capturedNumberIndex) {
        qWarning() << "No filename found in group" << group;
        return "ERR_NO_FILE";
    }
    const QString capturedFilename = match.captured(capturedNumberIndex);
    DEBUG_ASSERT(!capturedFilename.isNull());
    return capturedFilename;
}

} //anonymous namespace


Deck::Deck(PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMixer* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup)
        : BaseTrackPlayerImpl(pParent,
                  pConfig,
                  pMixingEngine,
                  pEffectsManager,
                  defaultOrientation,
                  handleGroup,
                  /*defaultMainMix*/ true,
                  /*defaultHeadphones*/ false,
                  /*primaryDeck*/ true) {
    deckName = handleGroup.name();

    m_pStemControl = new ControlObject(ConfigKey(handleGroup.name(), "LoadStems"));
    m_pStemControl->connectValueChangeRequest(this,
            &Deck::slotStemEnabled, Qt::DirectConnection);
}

Deck::~Deck() {
    delete m_pStemControl;
}

void Deck::slotStemEnabled(double v) {
    bool enable = v > 0.0;

    if (enable) {
        const int deckNumber = extractIntFromRegex(kDeckRegex, this->deckName);
	QString firstStemNumber;

	if (deckNumber == 1) {
	    firstStemNumber = "1";
	}

	else if (deckNumber == 2) {
	    firstStemNumber = "5";
        }

	else if (deckNumber == 3) {
	    firstStemNumber = "9";
	}

	else if (deckNumber == 4) {
	    firstStemNumber = "13";
	}

        TrackPointer pTrack = this->getLoadedTrack();
        const QString fileName = extractFilenameFromRegex(kFilenameRegex, pTrack->getLocation());

        QString firstScratchFile = QDir::homePath() + QString("/separated/mdx_extra_q/") + fileName + QString("/vocals.wav");
        QString secondScratchFile = QDir::homePath() + QString("/separated/mdx_extra_q/") + fileName + QString("/drums.wav");
        QString thirdScratchFile = QDir::homePath() + QString("/separated/mdx_extra_q/") + fileName + QString("/bass.wav");
        QString fourthScratchFile = QDir::homePath() + QString("/separated/mdx_extra_q/") + fileName + QString("/other.wav");

        this->m_pPlayerManager->slotLoadToStem(firstScratchFile, firstStemNumber.toInt());
        this->m_pPlayerManager->slotLoadToStem(secondScratchFile, firstStemNumber.toInt() + 1);
        this->m_pPlayerManager->slotLoadToStem(thirdScratchFile, firstStemNumber.toInt() + 2);
        this->m_pPlayerManager->slotLoadToStem(fourthScratchFile, firstStemNumber.toInt() + 3);
    }
}
