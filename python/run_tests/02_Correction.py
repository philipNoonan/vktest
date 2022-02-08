from pathlib import Path
import json
import numpy as np
import cv2
import unittest

import sys

def main():
    print("Starting correction test...")

    if sys.platform == "win64":
        libname = Path(__file__).absolute().parents[2] / 'build/bin/Debug/'
    elif sys.platform == "darwin":
        libname = Path(__file__).absolute().parents[2] / 'build/lib/'
    sys.path.insert(1, str(libname))

    import correction as corr
    print("imported correction lib")

    image_width = int(2048 / 4)
    image_height = int(1088 / 4)

    
    testInput = np.load(str(Path(__file__).absolute().parents[2] / 'data/tests/correction_calibrated_u16.npy'))
    with open(str(Path(__file__).absolute().parents[2] / 'data/cameras/calibration/config.json')) as json_file:
        testCameraJson = json.load(json_file)

    cv2.imshow("corr", np.reshape(testInput, (16, image_height, image_width))[0,:,:])
    
    hvs = corr.HvsTest()
    hvs.setCalibratedHypercube(testInput)
    hvs.setCorrectionMatrix(testCameraJson["cameras"][1]["correction_matrix"])


    hvs.initGPU()


    hvs.run()
    outputCorrection = hvs.getCalibratedCorrectedHypercube()
    print(outputCorrection[0:100])

    outf32 = np.reshape(outputCorrection, (16, image_height, image_width))
    cv2.imshow("output band 0", outf32[0, :, :])
    cv2.waitKey(0)

    np.save(str(Path(__file__).absolute().parents[2] / 'data/tests/colorize_calibrated_corrected_u16.npy'), outputCorrection)



if __name__ == "__main__":
    main()