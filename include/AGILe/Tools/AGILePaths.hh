// -*- C++ -*-
#ifndef AGILE_AgilePaths_HH
#define AGILE_AgilePaths_HH

namespace AGILe {


  /// Get library install path
  std::string getLibPath();

  /// Get data install path
  std::string getDataPath();

  /// Get Agile data install path
  std::string getAGILeDataPath();


  /// Get AGILe generator library search paths (cf. Genser root directory)
  std::vector<std::string> getGenPaths();

  /// Get LCG platform tag
  std::string getLCGPlatformTag();

  /// Utility to append a vector of strings to another (Introduced since the += operator from
  /// boost assign failed to work for boost > 1.68)
  void vectorAppend(vector<string>& to, vector<string> const& from);

}

#endif
