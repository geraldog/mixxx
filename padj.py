#!/usr/bin/python3
import time
import os

def DSLSequentialCommand(indexConfirm: int, indexWrite: int, DSLCommand: str)->tuple:
    while (True):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            indexRead = int(canIPlayWithMadness)
    
        except:
            time.sleep(0.004)

        if (indexRead == indexConfirm):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            print(DSLCommand)
            controlXY.write(DSLCommand + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")
            break

        else:
            time.sleep(0.004)
            continue

    return (indexConfirm, indexWrite)

def DSLPositionalCommand(indexConfirm: int, indexWrite: int, DSLCommand: str, playPosition: float, deckToWatch: int)->tuple:
    while (True):
        checkerboard = open("/home/dumbo/mixxxposition" + str(deckToWatch) + ".txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()
    
        try:
            positionNow = float(canIPlayWithMadness)
    
        except:
            time.sleep(0.004)
            continue
    
        if (positionNow >= playPosition):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            try:
                indexRead = int(canIPlayWithMadness)
    
            except:
                time.sleep(0.004)

            if (indexRead == indexConfirm):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                print(DSLCommand)
                controlXY.write(DSLCommand + "\n" + str(indexWrite) + "\n")
                controlXY.close()
    
                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")

                break
 
            else:
                time.sleep(0.004)
                continue

        else:
            time.sleep(0.004)
            continue
    return (indexConfirm, indexWrite)

def deckLoad(indexConfirm: int, indexWrite: int, deckNumber: int, pathToSong: str, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "L" + str(deckNumber) + pathToSong)
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "L" + str(deckNumber) + pathToSong, playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def playDeck(indexConfirm: int, indexWrite: int, deckNumber: int, playStart: float, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "P" + str(deckNumber) + str(playStart))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "P" + str(deckNumber) + str(playStart), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def beatLoop(indexConfirm: int, indexWrite: int, deckNumber: int, beatsNumber: int, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "O" + str(deckNumber) + "B" + str(beatsNumber))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "O" + str(deckNumber) + "B" + str(beatsNumber), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def cueMark(indexConfirm: int, indexWrite: int, deckNumber: int, cueNumber: int, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "H" + str(deckNumber) + "M" + "{cueMarker:02d}".format(cueMarker = cueNumber))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "H" + str(deckNumber) + "M" + "{cueMarker:02d}".format(cueMarker = cueNumber), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def cueGotoAndSet(indexConfirm: int, indexWrite: int, deckNumber: int, cueNumber: int, cuePosition: float, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "H" + str(deckNumber) + "X" + "{cueMarker:02d}{cuePosition:0.6f}".format(cueMarker = cueNumber, cuePosition = cuePosition))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "H" + str(deckNumber) + "X" + "{cueMarker:02d}{cuePosition:0.6f}".format(cueMarker = cueNumber, cuePosition = cuePosition), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def activateStems(indexConfirm: int, indexWrite: int, deckNumber: int, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "Z" + str(deckNumber))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "Z" + str(deckNumber), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def setBPM(indexConfirm: int, indexWrite: int, deckNumber: int, bpm: float, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "T" + str(deckNumber) + str(bpm))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "T" + str(deckNumber) + str(bpm), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def openOrCloseStem(indexConfirm: int, indexWrite: int, stemNumber: int, openOrClose: bool, slope: float, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        if (openOrClose):
            indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "S" + str(stemNumber) + "VO" + str(slope))
        else:
            indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "S" + str(stemNumber) + "VC" + str(slope))

    else:
        if (openOrClose):
            indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "S" + str(stemNumber) + "VO" + str(slope), playPosition, deckToWatch)
        else:
            indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "S" + str(stemNumber) + "VC" + str(slope), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

def faderInvert(indexConfirm: int, indexWrite: int, slope: float, nowOrLater: bool, playPosition: float, deckToWatch: int)->tuple:
    if (nowOrLater):
        indexConfirm, indexWrite = DSLSequentialCommand(indexConfirm, indexWrite, "FX" + str(slope))
    else:
        indexConfirm, indexWrite = DSLPositionalCommand(indexConfirm, indexWrite, "FX" + str(slope), playPosition, deckToWatch)
    return (indexConfirm, indexWrite)

indexConfirm = int("-1") 
indexWrite = int("1")

indexConfirm, indexWrite = deckLoad(indexConfirm, indexWrite, 1, "/home/dumbo/DICK_RIPS/Z.Z.Top - First Album/02 - Z.Z.Top - Brown Sugar.flac", True, 0, 0)
indexConfirm, indexWrite = playDeck(indexConfirm, indexWrite, 1, 0.0, True, 0, 0)
indexConfirm, indexWrite = setBPM(indexConfirm, indexWrite, 1, 105.0, True, 0, 0)
indexConfirm, indexWrite = activateStems(indexConfirm, indexWrite, 1, True, 0, 0)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 1, False, 1000.0, True, 0, 0)

#indexConfirm, indexWrite = cueGotoAndSet(indexConfirm, indexWrite, 1, 2, 0.71, True, 0, 0) # 2 for First Hotcue
#indexConfirm, indexWrite = beatLoop(indexConfirm, indexWrite, 1, 23, False, 0.71522, 1)
#indexConfirm, indexWrite = cueMark(indexConfirm, indexWrite, 1, 3, True, 0, 0)
#indexConfirm, indexWrite = cueGotoAndSet(indexConfirm, indexWrite, 1, 4, 0.859212, True, 0, 0)
#indexConfirm, indexWrite = beatLoop(indexConfirm, indexWrite, 1, 15, True, 0, 0)
#indexConfirm, indexWrite = cueMark(indexConfirm, indexWrite, 1, 5, True, 0, 0)

indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 2, False, 0.9, False, 0.8, 1)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 3, False, 0.9, True, 0, 0)

indexConfirm, indexWrite = deckLoad(indexConfirm, indexWrite, 2, "/home/dumbo/DICK_RIPS/Os Mutantes - A Arte de Os Mutantes/18 - Os Mutantes - É Proibido Proibir (ft. Caetano Veloso).flac", True, 0, 0)

indexConfirm, indexWrite = playDeck(indexConfirm, indexWrite, 2, 0.1, True, 0, 0)
indexConfirm, indexWrite = setBPM(indexConfirm, indexWrite, 2, 105.0, True, 0, 0)
indexConfirm, indexWrite = activateStems(indexConfirm, indexWrite, 2, True, 0, 0)

indexConfirm, indexWrite = faderInvert(indexConfirm, indexWrite, 0.36, True, 0, 0)

indexConfirm, indexWrite = cueMark(indexConfirm, indexWrite, 2, 2, False, 0.612206, 2)
indexConfirm, indexWrite = cueGotoAndSet(indexConfirm, indexWrite, 2, 3, 0.737791, True, 0, 0)

indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 6, False, 10.0, False, 0.85, 2)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 7, False, 10.0, True, 0, 0)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 8, False, 0.4, True, 0, 0)

indexConfirm, indexWrite = deckLoad(indexConfirm, indexWrite, 1, "/home/dumbo/DICK_RIPS/Clapton, Eric & Steve Winwood - Live From Madison Square Garden [Limited Deluxe 2DVD + CD Version] [CD2]/09 - Clapton, Eric & Steve Winwood - Cocaine (Live).flac", True, 0, 0)
indexConfirm, indexWrite = playDeck(indexConfirm, indexWrite, 1, 0.0, True, 0, 0)
indexConfirm, indexWrite = setBPM(indexConfirm, indexWrite, 1, 105.0, True, 0, 0)
indexConfirm, indexWrite = activateStems(indexConfirm, indexWrite, 1, True, 0, 0)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 1, False, 1000.0, True, 0, 0)
indexConfirm, indexWrite = faderInvert(indexConfirm, indexWrite, 0.45, True, 0, 0)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 1, True, 10.0, False, 0.10, 1)

indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 2, False, 0.9, False, 0.80, 1)

indexConfirm, indexWrite = deckLoad(indexConfirm, indexWrite, 2, "/home/dumbo/DICK_RIPS/Ronaldo & Os Impedidos - Ronaldo & Os Impedidos/11 - Ronaldo & Os Impedidos - Volta'N Blues.flac", True, 0, 0)

indexConfirm, indexWrite = playDeck(indexConfirm, indexWrite, 2, 0.1, True, 0, 0)
indexConfirm, indexWrite = setBPM(indexConfirm, indexWrite, 2, 105.0, True, 0, 0)
indexConfirm, indexWrite = activateStems(indexConfirm, indexWrite, 2, True, 0, 0)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 5, False, 1000.0, True, 0, 0)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 7, False, 1000.0, True, 0, 0)

indexConfirm, indexWrite = faderInvert(indexConfirm, indexWrite, 0.36, True, 0, 0)

indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 5, True, 0.8, False, 0.30, 2)
indexConfirm, indexWrite = openOrCloseStem(indexConfirm, indexWrite, 7, True, 0.8, True, 0, 0)
