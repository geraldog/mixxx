#!/usr/bin/python3
import time
import os

lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
controlXY = open("/home/dumbo/controlmixxx.txt", "w")
controlXY.write("L1/home/dumbo/AUTOMUSIK/12 Hush Hush.flac.mp3\n1\n")
controlXY.close()

os.close(lockFile)
os.unlink("/home/dumbo/controlmixxx.txt.lock")


controlCheck = False

indexConfirm = 0
indexWrite = 2


while (controlCheck == False):
    checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
    canIPlayWithMadness = checkerboard.readline()
    checkerboard.close()

    if (canIPlayWithMadness == str(indexConfirm) + "\n"):
        lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
        controlXY = open("/home/dumbo/controlmixxx.txt", "w")
        controlXY.write("P10" + "\n" + str(indexWrite) + "\n")
        controlXY.close()

        indexConfirm += 1
        indexWrite += 1
        os.close(lockFile)
        os.unlink("/home/dumbo/controlmixxx.txt.lock")


        controlCheck = True

    else:
        time.sleep(0.004)

controlCheck = False

while (controlCheck == False):
    checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
    canIPlayWithMadness = checkerboard.readline()
    checkerboard.close()

    if (canIPlayWithMadness == str(indexConfirm) + "\n"):
        lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
        controlXY = open("/home/dumbo/controlmixxx.txt", "w")
        controlXY.write("T1175" + "\n" + str(indexWrite) + "\n")
        controlXY.close()

        indexConfirm += 1
        indexWrite += 1
        os.close(lockFile)
        os.unlink("/home/dumbo/controlmixxx.txt.lock")


        controlCheck = True

    else:
        time.sleep(0.004)

controlCheck = False

while (controlCheck == False):
    checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
    canIPlayWithMadness = checkerboard.readline()
    checkerboard.close()

    if (canIPlayWithMadness == str(indexConfirm) + "\n"):
        lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
        controlXY = open("/home/dumbo/controlmixxx.txt", "w")
        controlXY.write("K113" + "\n" + str(indexWrite) + "\n")
        controlXY.close()

        indexConfirm += 1
        indexWrite += 1
        os.close(lockFile)
        os.unlink("/home/dumbo/controlmixxx.txt.lock")


        controlCheck = True

    else:
        time.sleep(0.004)


while (True):
    checkerboard = open("/home/dumbo/mixxxposition1.txt", "r")
    canIPlayWithMadness = checkerboard.readline()
    checkerboard.close()

    try:
        positionNow = float(canIPlayWithMadness)

    except:
        time.sleep(0.004)
        continue

    if (positionNow >= 0.5806451612903226):
        lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
        controlXY = open("/home/dumbo/controlmixxx.txt", "w")
        controlXY.write("L2/home/dumbo/AUTOMUSIK/11 Believe.flac.mp3\n" + str(indexWrite) + "\n")
        controlXY.close()

        indexConfirm += 1
        indexWrite += 1
        os.close(lockFile)
        os.unlink("/home/dumbo/controlmixxx.txt.lock")


    else:
        time.sleep(0.004)
        continue

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("P20" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True

        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("T2175" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True

        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("Q2HC1000" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("Q2LC1000" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("Q1LC1" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("Q1HC1" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("Q2LO1" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("Q2HO1" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)

    controlCheck = False

    while (controlCheck == False):
        checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        if (canIPlayWithMadness == str(indexConfirm) + "\n"):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("FX2" + "\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


            controlCheck = True
        else:
            time.sleep(0.004)



    while (True):
        checkerboard = open("/home/dumbo/mixxxposition2.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.3346613545816733):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L1/home/dumbo/AUTOMUSIK/02 Supergrass.flac.mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P10.548313" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True

            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T1175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True

            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("K14" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LC10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HC10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1LO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1HO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX20" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    while (True):
        checkerboard = open("/home/dumbo/mixxxposition1.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.8253687315634219):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L2/home/dumbo/AUTOMUSIK/05 Midnight.flac.mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P20.2579" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T2175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)



        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1LC10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1HC10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX20" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    while (True):
        checkerboard = open("/home/dumbo/mixxxposition2.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.66507936507936506):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L1/home/dumbo/AUTOMUSIK/09 Sambassim (DJ Marky VIP mix).flac.mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P10.013392857142857142" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T1175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)



        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LC1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HC1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2MC1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1LO1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1HO1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX4" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    while (True):
        checkerboard = open("/home/dumbo/mixxxposition1.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.70982142857142858):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L2/home/dumbo/AUTOMUSIK/18 When I'm Close 2 U.flac.mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P20.3458904109589041" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T2175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2MO10" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)



        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX2" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    while (True):
        checkerboard = open("/home/dumbo/mixxxposition2.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.6198630136986302):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L1/home/dumbo/AUTOMUSIK/10. Drumagick - Cambraia.mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P10.4861407249466951" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T1175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HC2" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    while (True):
        checkerboard = open("/home/dumbo/mixxxposition1.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.80):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L2/home/dumbo/AUTOMUSIK/David W.S. - Talarico (Cd Edit).mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P20.459383" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T2175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LC1000" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HO1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LO1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1LC3" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1HC3" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX5" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    while (True):
        checkerboard = open("/home/dumbo/mixxxposition2.txt", "r")
        canIPlayWithMadness = checkerboard.readline()
        checkerboard.close()

        try:
            positionNow = float(canIPlayWithMadness)

        except:
            time.sleep(0.004)
            continue

        if (positionNow >= 0.745111):
            lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
            controlXY = open("/home/dumbo/controlmixxx.txt", "w")
            controlXY.write("L1/home/dumbo/AUTOMUSIK/07. Landslide - Drumandbossa.mp3\n" + str(indexWrite) + "\n")
            controlXY.close()

            indexConfirm += 1
            indexWrite += 1
            os.close(lockFile)
            os.unlink("/home/dumbo/controlmixxx.txt.lock")


        else:
            time.sleep(0.004)
            continue


        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm)+ "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("P10.141376" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("T1175" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("K114" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2HC1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q2LC1" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1HO2" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("Q1LO2" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        controlCheck = False

        while (controlCheck == False):
            checkerboard = open("/home/dumbo/confirmixxx.txt", "r")
            canIPlayWithMadness = checkerboard.readline()
            checkerboard.close()

            if (canIPlayWithMadness == str(indexConfirm) + "\n"):
                lockFile = os.open("/home/dumbo/controlmixxx.txt.lock", os.O_CREAT|os.O_EXCL|os.O_RDWR)
                controlXY = open("/home/dumbo/controlmixxx.txt", "w")
                controlXY.write("FX3" + "\n" + str(indexWrite) + "\n")
                controlXY.close()

                indexConfirm += 1
                indexWrite += 1
                os.close(lockFile)
                os.unlink("/home/dumbo/controlmixxx.txt.lock")


                controlCheck = True
            else:
                time.sleep(0.004)

        break

    break
