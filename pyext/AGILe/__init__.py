"Python interface to the Rivet MC validation system"

## Change dlopen status to GLOBAL for AGILe lib loading
import sys
try:
    import ctypes
    sys.setdlopenflags(sys.getdlopenflags() | ctypes.RTLD_GLOBAL)
    del ctypes
except:
    import dl
    sys.setdlopenflags(sys.getdlopenflags() | dl.RTLD_GLOBAL)
    del dl
del sys

## Import SWIG-generated wrapper core
from agilewrap import *

## Provide an extra Python-only function used to enforce the AGILe scripts' minimal Python version
def check_python_version():
    import sys
    req_version = (2,7,0)
    if sys.version_info[:3] < req_version:
        print("AGILe scripts require Python version >= %s... exiting" % ".".join(req_version))
        sys.exit(1)
