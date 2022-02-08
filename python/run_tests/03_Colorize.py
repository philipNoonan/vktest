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

    import colorize as colo
    print("imported colorize lib")

    image_width = int(2048 / 4)
    image_height = int(1088 / 4)

    testInput = np.load(str(Path(__file__).absolute().parents[2] / 'data/tests/colorize_calibrated_corrected_u16.npy'))

    with open(str(Path(__file__).absolute().parents[2] / 'data/cameras/calibration/config.json')) as json_file:
        testCameraJson = json.load(json_file)

    padded_color_matrix = (np.c_[testCameraJson["cameras"][1]["sRGB_correction_v2"], np.ones(16)])

    print(padded_color_matrix)

    hvs = colo.HvsTest()


    hvs.setCalibratedCorrectedHypercube(testInput)
    hvs.setColorMatrix(padded_color_matrix)
    #print(hvs.getA())
    #print("loaded obj")
    hvs.initGPU()
    #hvs.loadFileU16(str(testImage))

    #hvs.setWhiteValue(1023)
    #hvs.setDarkValue(0)
    #hvs.setMaskValue(255)

    hvs.run()

    output_srgb = hvs.getSRGB()
    print(output_srgb[0:100])

    outu8 = np.reshape(output_srgb, (image_height, image_width, 4))
    # cv2.imshow("output srgb", np.swapaxes(outu8, 0, 2))
    cv2.imshow("output srgb", outu8)

    cv2.waitKey(0)

    # np.save(str(Path(__file__).absolute().parents[2] / 'data/tests/srgb_output_u16.npy'), output_srgb)



if __name__ == "__main__":
    main()