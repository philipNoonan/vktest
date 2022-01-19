from pathlib import Path
import numpy as np
import cv2
import unittest

import sys

def main():
    print("Starting demosaic test...")
    if sys.platform == "win64":
        libname = Path(__file__).absolute().parents[2] / 'build/bin/Debug/'
    elif sys.platform == "darwin":
        libname = Path(__file__).absolute().parents[2] / 'build/lib/'
    sys.path.insert(1, str(libname))

    import demosaic as demo
    print("imported demosaic lib")

    image_width = 2048
    image_height = 1088

    #testCalibrated = (cv2.imread(str(Path(__file__).absolute().parents[2] / 'data/tests/demosaic_calibrated_u16.png'), cv2.IMREAD_ANYDEPTH)).astype(np.float32) / 65535.0
    
    testCalibrated = np.load(str(Path(__file__).absolute().parents[2] / 'data/tests/demosaic_calibrated_u16.npy'))

    cv2.imshow("corr", np.reshape(testCalibrated, (image_height, image_width)))

    hvs = demo.HvsTest()
    #print(hvs.getA())
    #print("loaded obj")

    hvs.setCalibratedImage(testCalibrated)
    hvs.initGPU()
    #hvs.loadFileU16(str(testImage))

    #hvs.setWhiteValue(1023)
    #hvs.setDarkValue(0)
    #hvs.setMaskValue(255)

    hvs.run()
    outputDemosaiced = hvs.getCalibratedHypercube()
    print(outputDemosaiced[0:100])

    outf32 = np.reshape(outputDemosaiced, (16, int(image_height / 4), int(image_width / 4)))
    cv2.imshow("output band 0", outf32[0, :, :])

    cv2.waitKey(0)

    # np.save(str(Path(__file__).absolute().parents[2] / 'data/tests/correction_calibrated_u16.npy'), outputDemosaiced)

if __name__ == "__main__":
    main()