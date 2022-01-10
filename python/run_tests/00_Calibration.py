from pathlib import Path
import numpy as np
import unittest

import sys

def main():
    print("Starting calibration test...")
   # print(str(libname))
   # demosaic_c_lib = ctypes.CDLL("C:\\code\\vkHSI\\build\\bin\\Debug\\demosaic")
    libname = Path(__file__).absolute().parents[2] / 'build/bin/Debug/'

    sys.path.insert(1, str(libname))

    import calibration as cali

    #testImage = (Path(__file__).absolute().parents[2] / 'data/tests/demosaic_input_f32.bin')
    #testImage = (Path(__file__).absolute().parents[2] / 'data/tests/calibration_input_u16.bin')

    hvs = cali.HvsTest()
    #hvs.loadFileU16(str(testImage))

    #hvs.setWhiteValue(1023)
    #hvs.setDarkValue(0)
    #hvs.setMaskValue(255)

    print(hvs.getCalibratedImage())


if __name__ == "__main__":
    main()