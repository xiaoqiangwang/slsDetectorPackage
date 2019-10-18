import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))
from sls_detector import Eiger, Jungfrau, Detector, defs
from sls_detector import ExperimentalDetector

from _sls_detector.io import read_my302_file
from sls_detector.dacs import Dac

d = Detector()
e = ExperimentalDetector()

from sls_detector import dacIndex
c = Dac(dacIndex(3), 0, 4000, 2000, e)
j = Jungfrau()

