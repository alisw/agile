#include "AGILe/AGILe.hh"
#include "AGILe/Generator.hh"
#include "AGILe/Tools/AGILePaths.hh"
#include "AGILe/Tools/osdir.hh"
#include <dlfcn.h>
#include <unistd.h>

namespace AGILe {


  namespace Loader {


    Log& getLog() {
      return AGILe::Log::getLog("AGILe.Loader");
    }


    // Struct for holding library names and versions
    struct GenLibInfo {
      GenLibInfo(string libname, string pkgname="", string pkgversion="") {
        this->libname = libname;
        this->pkgname = pkgname;
        this->pkgversion = pkgversion;
      }
      string libname, pkgname, pkgversion;
    };


    // Static library handles
    vector<void*> handles;
    void* agilehandle = 0;

    // Libraries required for each known generator.
    typedef map<string, vector<GenLibInfo> > GenLibs;

    // Generator version is kept globally for use by the
    // Generator::setVersion(...) function.
    string version;


    // Initialise the dynamic loader
    void initialize() {
      // Nothing to do with raw dlopen
    }



    // Make the proper system library name
    string sysLibName(string libname) {
      // Put lib suffix on the end if needed
      if (libname.find(SYSDSO) == string::npos) libname += SYSDSO;
      return libname;
    }



    vector<string> getPlatforms(const GenLibInfo& info) {
      vector<string> platforms;
      if (!info.pkgname.empty()) {
        // If we're not trying to load an AGILe interface...
        platforms += getLCGPlatformTag();
      }
      // Also try with no platform tag
      platforms += "";

      return platforms;
    }



    vector<string> getLibSearchDirs(const GenLibInfo& info) {
      vector<string> dirs;
      if (info.pkgname.empty()) {
        // If we're trying to load an AGILe interface...

        /// @todo Is this needed? Can't dlopen do this automatically for non-gen libs?

        // Use the AGILe library install path
        dirs.push_back(getLibPath());

        // Use the current dir, too.
        dirs.push_back(".");

        // and some default system locations
        dirs += "/usr/lib", "/usr/lib64", "/usr/lib32";
        dirs += "/usr/local/lib", "/usr/local/lib64", "/usr/local/lib32";

        // And then the user's (non-system) library path (inc. for dyld on OS X)
        const char* envld = getenv("LD_LIBRARY_PATH");
        if (envld) vectorAppend(dirs, split(envld));
        const char* envdyld = getenv("DYLD_LIBRARY_PATH");
        if (envdyld) vectorAppend(dirs, split(envdyld));

      } else {
        // If we're loading a real generator library...
        vectorAppend(dirs, getGenPaths());
      }

      return dirs;
    }



    string findGenLib(const GenLibInfo& info,
                      const vector<string> searchdirs,
                      const vector<string> platforms)
    {
      // Get platform-specific filename
      string libname = sysLibName(info.libname);

      // Try to find lib file here
      foreach (const string& d, searchdirs) {
        foreach (const string& p, platforms) {
          vector<string> versionsuffixes;
          versionsuffixes += ".3", ".2", "";
          foreach (string& vsuffix, versionsuffixes) {
            string libpath = d + "/" + info.pkgname;
            if (!info.pkgversion.empty()) {
              libpath += "/" + info.pkgversion + vsuffix;
            }
            if (!p.empty()) {
              libpath += "/" + p;
            }
            if (!info.pkgname.empty()) {
              libpath += "/lib";
            }
            libpath += "/" + libname;
            // Remove duplicate slashes from path
            while (libpath.find("//") != string::npos) {
              libpath.replace(libpath.find("//"), 2, "/");
            }
            // Return path if present
            getLog() << Log::TRACE << "Testing for " << libpath << endl;
            if (access(libpath.c_str(), R_OK) == 0) {
              getLog() << Log::TRACE << "Found " << libpath << endl;
              return libpath;
            }
          }
        }
      }
      // Fall back to non-Genser structure
      foreach (const string& d, searchdirs) {
        const string libdir = d + "/lib";
        vector<string> libnames;
        libnames += libname,(info.libname+info.pkgversion+SYSDSO);
        foreach (string& ln, libnames) {
          string libpath = libdir + "/" + ln;
          // Remove duplicate slashes from path
          while (libpath.find("//") != string::npos) {
            libpath.replace(libpath.find("//"), 2, "/");
          }
          // Return path if present
          getLog() << Log::TRACE << "Testing for " << libpath << endl;
          if (access(libpath.c_str(), R_OK) == 0) {
            getLog() << Log::TRACE << "Found " << libpath << endl;
            return libpath;
          }
        }
      }
      return "";
    }



    string findGenLib(const GenLibInfo& info) {
      // Build up lists of search directories and platforms
      vector<string> platforms = getPlatforms(info);
      vector<string> dirs = getLibSearchDirs(info);

      return findGenLib(info, dirs, platforms);
    }



    void* loadGenLib(const string& libpath, int dlmode=(RTLD_LAZY|RTLD_GLOBAL)) {
      getLog() << Log::TRACE << "Trying to load " << libpath << endl;
      void* lib = dlopen(libpath.c_str(), dlmode);
      if (lib) {
        getLog() << Log::TRACE << "Successfully loaded " << libpath << " (" << lib << ")" << endl;
        // If this is an AGILe interface, set the global handle
        if (libpath.find("libAGILe") != string::npos) {
          getLog() << Log::TRACE << "Setting AGILe module handle for " << libpath
                   << " (" << lib << ")" << endl;
          agilehandle = lib;
        }
      } else {
        getLog() << Log::TRACE << "Failed to load " << libpath << endl;
      }
      return lib;
    }


    void* loadGenLib(const GenLibInfo& info, int dlmode=(RTLD_LAZY|RTLD_GLOBAL)) {
      const string libpath = findGenLib(info);
      if (libpath != "") {
        return loadGenLib(libpath, dlmode);
      }
      return 0;
    }


    vector<string> _findGenVersions(const string& name) {
      vector<string> rtn;
      vector<string> dirs = getGenPaths();
      foreach (const string& d, dirs) {
        oslink::directory dir(d + "/" + name);
        while (dir) {
          string filename = dir.next();
          if (filename == "." || filename == "..") continue;
          if (filename.find(".2") == filename.length()-2 || filename.find(".3") == filename.length()-2) {
            filename = filename.substr(0, filename.length()-2);
          }
          //if (rtn.find(filename) == rtn.end()) rtn += filename;
          if (find(rtn.begin(), rtn.end(), filename) == rtn.end()) rtn += filename;
        }
      }
      // Get the versions sorted most-recent first, so that it will be the first to be tested
      sort(rtn.begin(), rtn.end());
      rtn += "HEAD";
      reverse(rtn.begin(), rtn.end());
      return rtn;
    }


    GenLibs _getAllGenLibInfo() {

      // Is LHAPDF available?
      const vector<string> lhapdf_versions = _findGenVersions("lhapdf");
      bool haveLHAPDF = false;
      GenLibInfo lhapdfLibInfo("DUMMY");
      foreach (const string& v, lhapdf_versions) {
        lhapdfLibInfo = GenLibInfo("libLHAPDF", "lhapdf", v);
        haveLHAPDF = ! findGenLib(lhapdfLibInfo).empty();
        if (haveLHAPDF) break;
      }
      // Try to fall back to non-Genser packaging
      if (!haveLHAPDF) {
        lhapdfLibInfo = GenLibInfo("libLHAPDF");
        haveLHAPDF = ! findGenLib(lhapdfLibInfo).empty();
      }
      // Announce the result
      if (haveLHAPDF) {
        Log::getLog("AGILe.Loader") << Log::DEBUG << "LHAPDF is available" << endl;
      } else {
        Log::getLog("AGILe.Loader") << Log::WARN << "LHAPDF is not available" << endl;
      }


      // Find available Fortran generator versions
      vector<string> py6_versions = _findGenVersions("pythia6");
      vector<string> hw6_versions = _findGenVersions("herwig");
      vector<string> alpgen_versions = _findGenVersions("alpgen");
      vector<string> rapgap_versions = _findGenVersions("rapgap");
      vector<string> cascade_versions = _findGenVersions("cascade");
      vector<string> ariadne_versions = _findGenVersions("ariadne");
      vector<string> phojet_versions = _findGenVersions("phojet");


      // Return value, to be populated
      GenLibs gens;


      // Fortran Herwig + variants
      foreach (const string& v, hw6_versions) {
        vector<GenLibInfo> baselibs, reqlibs;
        baselibs += GenLibInfo("libherwig", "herwig", v);
        if (haveLHAPDF) {
          baselibs += lhapdfLibInfo;
        } else {
          baselibs += GenLibInfo("libherwig_pdfdummy", "herwig", v);
        }
        baselibs += GenLibInfo("libHepMCfio");
        baselibs += GenLibInfo("libAGILeFHerwig");
        const GenLibInfo libinfo_herwigdummy("libherwig_dummy", "herwig", v);
        const GenLibInfo libinfo_jimmy("libjimmy", "jimmy", "4.31");
        const GenLibInfo libinfo_charybher("libcharybdis-h", "charybdis", "1.003hp");
        ////
        // Standard Herwig
        reqlibs = baselibs;
        reqlibs += libinfo_herwigdummy;
        gens["Herwig:" + v] = reqlibs;
        //
        // Herwig+Jimmy
        reqlibs = baselibs;
        reqlibs += libinfo_herwigdummy;
        reqlibs += libinfo_jimmy;
        reqlibs += GenLibInfo("libAGILeFHerwigJimmy");
        gens["HerwigJimmy:" + v] = reqlibs;
        //
        // Herwig(+Jimmy)+AlpGen
        foreach (const string& alpgen_version, alpgen_versions) {
          // Herwig+AlpGen
          reqlibs = baselibs;
          reqlibs += GenLibInfo("libalpgen", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libalpsho", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libatoher", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libAGILeAlpGen");
          reqlibs += GenLibInfo("libAGILeAlpGenFHerwig");
          gens["AlpGenHerwig:" + v + ":" + alpgen_version] = reqlibs;
          // Herwig+Jimmy+AlpGen
          reqlibs = baselibs;
          reqlibs += GenLibInfo("libalpgen", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libalpsho", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libatoher", "alpgen", alpgen_version);
          reqlibs += libinfo_jimmy;
          reqlibs += GenLibInfo("libAGILeAlpGen");
          reqlibs += GenLibInfo("libAGILeFHerwigJimmy");
          reqlibs += GenLibInfo("libAGILeAlpGenFHerwigJimmy");
          gens["AlpGenHerwigJimmy:" + v + ":" + alpgen_version] = reqlibs;
        }
        //
        // Herwig+Charybdis
        reqlibs = baselibs;
        reqlibs += libinfo_charybher;
        reqlibs += GenLibInfo("libAGILeCharybdis");
        reqlibs += GenLibInfo("libAGILeCharybdisFHerwig");
        gens["CharybdisHerwig:" + v] = reqlibs;
        //
        // Herwig+Jimmy+Charybdis
        reqlibs = baselibs;
        reqlibs += libinfo_charybher;
        reqlibs += libinfo_jimmy;
        reqlibs += GenLibInfo("libAGILeCharybdis");
        reqlibs += GenLibInfo("libAGILeFHerwigJimmy");
        reqlibs += GenLibInfo("libAGILeCharybdisFHerwigJimmy");
        gens["CharybdisHerwigJimmy:" + v] = reqlibs;
      }


      // Fortran Pythia + variants
      foreach (const string& v, py6_versions) {
        vector<GenLibInfo> baselibs, reqlibs;
        baselibs += GenLibInfo("libpythia6", "pythia6", v);
        if (haveLHAPDF) {
          baselibs += lhapdfLibInfo;
        } else {
          baselibs += GenLibInfo("libpythia6_pdfdummy", "pythia6", v);
        }
        baselibs += GenLibInfo("libHepMCfio");
        //
        // Standard Pythia
        reqlibs = baselibs;
        reqlibs += GenLibInfo("libAGILeFPythia_pyevwt");
        reqlibs += GenLibInfo("libAGILeFPythia");
        gens["Pythia6:" + v] = reqlibs;
        //
        // Pythia+AlpGen
        foreach (const string& alpgen_version, alpgen_versions) {
          reqlibs = baselibs;
          reqlibs += GenLibInfo("libalpgen", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libalpsho", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libatopyt", "alpgen", alpgen_version);
          reqlibs += GenLibInfo("libpythia6_dummy", "pythia6", v);
          reqlibs += GenLibInfo("libAGILeAlpGen");
          reqlibs += GenLibInfo("libAGILeAlpGenFPythia");
          gens["AlpGenPythia6:" + v + ":" + alpgen_version] = reqlibs;
        }
        //
        // Pythia+Charybdis
        reqlibs = baselibs;
        reqlibs += GenLibInfo("libcharybdis", "charybdis", "1.003hp");
        reqlibs += GenLibInfo("libpythia6_dummy", "pythia6", v);
        reqlibs += GenLibInfo("libAGILeCharybdis");
        reqlibs += GenLibInfo("libAGILeCharybdisFPythia");
        gens["CharybdisPythia6:" + v] = reqlibs;
        //
        // Pythia+Rapgap
        foreach (const string& vr, rapgap_versions) {
          reqlibs.clear();
          reqlibs += GenLibInfo("libpythia6", "pythia6", v);
          reqlibs.push_back(GenLibInfo("libpythia6_dummy", "pythia6", v)); // This is for pytime
          //reqlibs += GenLibInfo("libpythia6_dummies", "pythia6", v);
          reqlibs += GenLibInfo("librapgap32", "Rapgap", vr);
          reqlibs += GenLibInfo("libHepMCfio"); // This order is important
          if (haveLHAPDF) {
            reqlibs += lhapdfLibInfo;
          } else {
            reqlibs += GenLibInfo("libpythia6_pdfdummy", "pythia6", v);
          }
          reqlibs += GenLibInfo("libAGILeRapgap");
          gens["RapgapPythia6:" + v + ":" + vr] = reqlibs;
        }
        //
        // Pythia+Cascade
        foreach (const string& vc, cascade_versions) {
          vector<GenLibInfo> baselibs, reqlibs;
          baselibs.push_back(GenLibInfo("libpythia6", "pythia6", v));
          baselibs.push_back(GenLibInfo("libpythia6_dummy", "pythia6", v)); // This is for pytime
          baselibs.push_back(GenLibInfo("libcascade2", "cascade", vc));
          baselibs.push_back(GenLibInfo("libHepMCfio")); // This order is important
          if (haveLHAPDF) {
            reqlibs += lhapdfLibInfo;
          } else {
            reqlibs += GenLibInfo("libpythia6_pdfdummy", "pythia6", v);
          }

          reqlibs = baselibs;
          reqlibs.push_back(GenLibInfo("libAGILeCascade"));
          gens["CascadePythia6:" + v + ":" + vc] = reqlibs;
        }
        //
        // Pythia+Ariadne
        foreach (const string& va, ariadne_versions) {
          vector<GenLibInfo> baselibs, reqlibs;
          baselibs.push_back(GenLibInfo("libpythia6", "pythia6", v));
          baselibs.push_back(GenLibInfo("libar4", "ariadne", va));
          baselibs.push_back(GenLibInfo("libHepMCfio")); // This order is important
          if (haveLHAPDF) {
            reqlibs += lhapdfLibInfo;
          } else {
            reqlibs += GenLibInfo("libpythia6_pdfdummy", "pythia6", v);
          }

          reqlibs = baselibs;
          reqlibs.push_back(GenLibInfo("libAGILeAriadne"));
          gens["Ariadne" + va + ":" + "Pythia6." + v] = reqlibs;
        }
        //
        // Pythia+Phojet
        foreach (const string& vp, phojet_versions) {
          vector<GenLibInfo> baselibs, reqlibs;
          baselibs.push_back(GenLibInfo("libpythia6", "pythia6", v));
          baselibs.push_back(GenLibInfo("libphojet", "phojet", vp));
          baselibs.push_back(GenLibInfo("libHepMCfio")); // This order is important
          if (haveLHAPDF) {
            reqlibs += lhapdfLibInfo;
          } else {
            reqlibs += GenLibInfo("libpythia6_pdfdummy", "pythia6", v);
          }

          reqlibs = baselibs;
          reqlibs.push_back(GenLibInfo("libAGILePhojet"));
          gens["Phojet" + vp + ":" + "Pythia6." + v] = reqlibs;
        }
      }

      return gens;
    }


    // Find which generators are present by testing the files.
    vector<string> getAvailableGens() {
      GenLibs genlibs = _getAllGenLibInfo();
      vector<string> availableGens;
      typedef pair<string, vector<GenLibInfo> > GenLibInfoKV;
      foreach (const GenLibInfoKV& g, genlibs) {
        getLog() << Log::TRACE << "Trying to find libs for " << g.first << endl;
        // All listed libraries must be available for each generator to be considered useable.
        bool ok = true;
        foreach (const GenLibInfo& l, g.second) {
          const string libpath = findGenLib(l);
          if (libpath != "") {
            getLog() << Log::TRACE << "Found " << l.libname << " at " << libpath << endl;
          } else {
            getLog() << Log::TRACE << "Didn't find "
                     << l.pkgname << ":" << l.pkgversion << ":" << l.libname << endl;
            ok = false;
            break;
          }
        }
        // If all libs were present, add this gen to the "available" list.
        if (ok) {
          getLog() << Log::DEBUG << "Found all libs for " << g.first << endl;
          availableGens.push_back(g.first);
        }
      }
      return availableGens;
    }


    void loadGenLibs(const string& genname) {
      const string fioName = "libHepMCfio";
      string agileName = "";
      string pdflibName = "";

      // Get version string from colon separated version string if given.
      string name = genname;
      version = "";
      const size_t colonpos = genname.rfind(":");
      if (colonpos != string::npos) {
        const size_t versionlength = genname.length() - colonpos - 1;
        name = genname.substr(0, colonpos);
        version = genname.substr(colonpos+1, versionlength);
      }

      GenLibs genlibs = _getAllGenLibInfo();
      if (genlibs.find(genname) == genlibs.end()) {
        throw runtime_error("Trying to load an unknown generator combination: " + genname);
      } else {
        const vector<GenLibInfo> libs = genlibs[genname];
        for (vector<GenLibInfo>::const_iterator l = libs.begin(); l != libs.end(); ++l) {
          handles.push_back(loadGenLib(*l));
        }
      }


      // Check and reset dlopen errors
      for (vector<void*>::const_iterator h = handles.begin(); h != handles.end(); ++h) {
        if (*h == 0) {
          throw runtime_error((string("Failed to load libraries: ") + dlerror()).c_str());
        }
      }
      dlerror();
    }


    template <typename Fn>
    inline Fn evil_cast(void* ptr) {
      return reinterpret_cast<Fn>(reinterpret_cast<size_t>(ptr));
    }


    Generator* createGen() {
      // Load the factory and create an instance of the class
      void* makeGen = dlsym(agilehandle, "create");
      if (char* err = dlerror()) {
        throw runtime_error((string("Can't load creation factory: ") + err).c_str());
      }

      // Make the generator object.
      Generator* gen = evil_cast<Creator>(makeGen)();

      // Set the generator version for e.g. HepML dumps of generator state.
      gen->setVersion(version);

      return gen;
    }


    void destroyGen(Generator* gen) {
      //Destroyer* destroy_gen = (Destroyer*) dlsym(agilemod, "destroy");
      //if (dlerror()) {
      //  throw runtime_error("Can't load destruction factory");
      //}
      //destroy_gen(gen);
      delete gen;
    }


    // Close the libraries
    void finalize() {
      for (vector<void*>::reverse_iterator h = handles.rbegin(); h != handles.rend(); ++h) {
        if (*h != 0) dlclose(*h);
      }
    }


  }
}
