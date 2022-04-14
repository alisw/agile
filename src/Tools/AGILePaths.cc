#include "AGILe/AGILe.hh"
#include "AGILe/Utils.hh"
#include "binreloc.h"

namespace AGILe {


  string getLibPath() {
    BrInitError error;
    br_init_lib(&error);
    char* tmp = br_find_lib_dir(DEFAULTLIBDIR);
    const string libdir(tmp);
    free(tmp);
    return libdir;
  }


  string getDataPath() {
    BrInitError error;
    br_init_lib(&error);
    char* tmp = br_find_data_dir(DEFAULTDATADIR);
    const string sharedir(tmp);
    free(tmp);
    return sharedir;
  }


  string getAGILeDataPath() {
    return getDataPath() + "/AGILe";
  }


  vector<string> getGenPaths() {
    vector<string> dirs;
    char* env = 0;
    env = getenv("AGILE_GEN_PATH");
    if (env) {
      // Use the AGILe generator path variable if set...
      dirs += split(env);
    } else {
      // Fall back to the Genser area (v84) on CERN CVMFS
      dirs += "/cvmfs/sft.cern.ch/lcg/releases/LCG_84/MCGenerators";
    }
    return dirs;
  }


  /// @todo Add API method: setLCGPlatformTag which overrides environment. Requires global variable... hmm.
  string getLCGPlatformTag() {
    // This non-standard env variable can be specified by the user.
    const char* envlcg = getenv("LCG_PLATFORM");
    if (envlcg) {
      return string(envlcg);
    } else {
      // Also try a fallback of the LCG tag, computed at build time.
      return LCGTAG;
    }
  }


}
