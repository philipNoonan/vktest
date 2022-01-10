from pathlib import Path
import unittest

import sys

def main():
    print("Starting correction test...")
   # print(str(libname))
   # demosaic_c_lib = ctypes.CDLL("C:\\code\\vkHSI\\build\\bin\\Debug\\demosaic")
    libname = Path(__file__).absolute().parents[2] / 'build/bin/Debug/'

    sys.path.insert(1, str(libname))

    import correction as corr

    #testImage = (Path(__file__).absolute().parents[2] / 'data/tests/demosaic_input_f32.bin')
    testImage = (Path(__file__).absolute().parents[2] / 'data/tests/correction_input_u16.bin')

    hvs = corr.HvsTest()
    hvs.loadFileU16(str(testImage))



if __name__ == "__main__":
    main()