from pathlib import Path
import unittest

import sys

def main():
    print("Starting demosaic test...")
   # print(str(libname))
   # demosaic_c_lib = ctypes.CDLL("C:\\code\\vkHSI\\build\\bin\\Debug\\demosaic")
    libname = Path(__file__).absolute().parents[2] / 'build/bin/Debug/'

    sys.path.insert(1, str(libname))

    import demosaic as demo

    #testImage = (Path(__file__).absolute().parents[2] / 'data/tests/demosaic_input_f32.bin')
    testImage = (Path(__file__).absolute().parents[2] / 'data/tests/demosaic_input_u16.png')

    hvs = demo.HvsTest()
    hvs.loadFileF32(str(testImage))

if __name__ == "__main__":
    main()