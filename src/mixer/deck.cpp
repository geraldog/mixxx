#include "mixer/deck.h"

#include "control/controlobject.h"
#include "moc_deck.cpp"
#include <QRegularExpression>

namespace {

const QRegularExpression kDeckRegex(QStringLiteral("^\\[Channel(\\d+)\\]$"));

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

Deck::Deck(PlayerManager* pParent,
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
    deckName = handleGroup.name();

    m_pStemControl = new ControlObject(ConfigKey(handleGroup.name(), "LoadStems"));
    m_pStemControl->connectValueChangeRequest(this,
            &Deck::slotStemEnabled, Qt::DirectConnection);
}

Deck::~Deck() {
    delete m_pStemControl;
}

void Deck::Deallocator(void* data, size_t length, void* arg)
{
	//free(data);
	//data = nullptr;
}

void Deck::slotStemEnabled(double v) {
    bool enable = v > 0.0;

    if (enable) {
        QThread *threadTF = QThread::create(threadedTensorflow, this);
	connect(threadTF, &QThread::finished, threadTF, &QThread::deleteLater);
        threadTF->start();
    }
}

void Deck::threadedTensorflow(Deck* deck) {
        const int deckNumber = extractIntFromRegex(kDeckRegex, deck->deckName);
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

        TrackPointer pTrack = deck->getLoadedTrack();

        QString firstScratchFile = QDir::homePath() + QString("/spleeterScratch_") + QString::number(firstStemNumber.toInt()) + QString(".wav");
        QString secondScratchFile = QDir::homePath() + QString("/spleeterScratch_") + QString::number(firstStemNumber.toInt() + 1) + QString(".wav");
        QString thirdScratchFile = QDir::homePath() + QString("/spleeterScratch_") + QString::number(firstStemNumber.toInt() + 2) + QString(".wav");
        QString fourthScratchFile = QDir::homePath() + QString("/spleeterScratch_") + QString::number(firstStemNumber.toInt() + 3) + QString(".wav");

        deck->m_pPlayerManager->slotLoadToStem(firstScratchFile, firstStemNumber.toInt());
        deck->m_pPlayerManager->slotLoadToStem(secondScratchFile, firstStemNumber.toInt() + 1);
        deck->m_pPlayerManager->slotLoadToStem(thirdScratchFile, firstStemNumber.toInt() + 2);
        deck->m_pPlayerManager->slotLoadToStem(fourthScratchFile, firstStemNumber.toInt() + 3);
}
