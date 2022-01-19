from pathlib import Path
import numpy as np
import cv2
import unittest

import sys

def main():
    print("Starting calibration test...")

    if sys.platform == "win64":
        libname = Path(__file__).absolute().parents[2] / 'build/bin/Debug/'
    elif sys.platform == "darwin":
        libname = Path(__file__).absolute().parents[2] / 'build/lib/'
    sys.path.insert(1, str(libname))

    import calibration as cali
    print("imported calibration lib")

    testRaw = cv2.imread(str(Path(__file__).absolute().parents[2] / 'data/tests/calibration_input_u16.png'), cv2.IMREAD_ANYDEPTH)
    testWhite = cv2.imread(str(Path(__file__).absolute().parents[2] / 'data/tests/calibration_white_u16.png'), cv2.IMREAD_ANYDEPTH)
    testDark = cv2.imread(str(Path(__file__).absolute().parents[2] / 'data/tests/calibration_dark_u16.png'), cv2.IMREAD_ANYDEPTH)

    testMask = np.ones_like(testRaw, dtype="ubyte") * 255
    cv2.imshow("raw", cv2.normalize(testRaw, dst=None, alpha=0, beta=65535, norm_type=cv2.NORM_MINMAX))
    #cv2.imshow("white", testWhite)
    #cv2.imshow("dark", testDark)
    #print(testMask[0:100])


    hvs = cali.HvsTest()
    #print(hvs.getA())
    #print("loaded obj")

    hvs.setRawImage(testRaw)
    hvs.setWhiteImage(testWhite)
    hvs.setDarkImage(testDark)
    hvs.setMaskImage(testMask)

    hvs.initGPU()
    #hvs.loadFileU16(str(testImage))



    #hvs.setWhiteValue(1023)
    #hvs.setDarkValue(0)
    #hvs.setMaskValue(255)

    hvs.run()
    outputCalibrated = hvs.getCalibratedImage()
    print(outputCalibrated[0:100])

    outf32 = np.reshape(outputCalibrated, (testRaw.shape[0], testRaw.shape[1]))
    cv2.imshow("output", outf32)

    cv2.waitKey(0)

    #np.save(str(Path(__file__).absolute().parents[2] / 'data/tests/demosaic_calibrated_u16.npy'), outputCalibrated)

    #outu16 = (outf32 * 65535.0).astype(np.uint16)

    #cv2.imwrite(str(Path(__file__).absolute().parents[2] / 'data/tests/demosaic_calibrated_u16.png'), outu16)




if __name__ == "__main__":
    main()