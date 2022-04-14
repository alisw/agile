## Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([AGILe],[1.5.1],[rivet@projects.hepforge.org],[AGILe])

## Check and block installation into the src/build dir
if test "$prefix" = "$PWD"; then
  AC_MSG_ERROR([Installation into the build directory is not supported: use a different --prefix argument])
fi
## Force default prefix to have a path value rather than NONE
if test "$prefix" = "NONE"; then
   prefix=/usr/local
fi

AC_CONFIG_SRCDIR([src/Core/Loader.cc])
AC_CONFIG_HEADER([include/AGILe/DummyConfig.hh include/AGILe/AGILeConfig.hh include/AGILe/FortranWrappers.hh])
AM_INIT_AUTOMAKE(dist-bzip2)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])

## Package-specific #defines
AC_DEFINE_UNQUOTED(AGILE_VERSION, "$PACKAGE_VERSION", "AGILe version string")
AC_DEFINE_UNQUOTED(AGILE_NAME, "$PACKAGE_NAME", "AGILe name string")
AC_DEFINE_UNQUOTED(AGILE_STRING, "$PACKAGE_STRING", "AGILe name and version string")
AC_DEFINE_UNQUOTED(AGILE_TARNAME, "$PACKAGE_TARNAME", "AGILe short name string")
AC_DEFINE_UNQUOTED(AGILE_BUGREPORT, "$PACKAGE_BUGREPORT", "AGILe contact email address")

## Library version flags, current:revision:age (increment these for each new version)
VERSIONFLAGS="-version-info 10:0:0" ## set for 1.5.0
AC_SUBST(VERSIONFLAGS)

## Cache PATH variable
AC_ARG_VAR(PATH, [we grab the PATH variable, since various generator config scripts may need to be called repeatably])

## Set default compiler flags
if test "x$CXXFLAGS" == "x"; then CXXFLAGS="-O3"; fi

## More comprehensible error messages if possible. See:
## http://www.bdsoft.com/tools/stlfilt.html
if test "x$CXXFLAGS" == "x"; then
  AC_PATH_PROG(GFILT, gfilt, $CXX, $PATH:$HOME/bin:$HOME/local/bin)
  CXX=$GFILT
fi

## Declare language
AC_LANG(C++)

## OS X
AC_CEDAR_OSX

## LCG platform tag
AC_LCG_TAG

## Checks for programs.
AC_PROG_CXX
AC_PROG_FC(gfortran g95 g77)
AC_FC_LIBRARY_LDFLAGS
AC_FC_WRAPPERS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_DISABLE_STATIC

## Hack to make libtool use the FC Fortran compiler via F77, since
## libtool < 2.x doesn't understand --tag=FC
F77=$FC
AM_FFLAGS=$AM_FCFLAGS
AC_SUBST(F77)
AC_SUBST(AM_FFLAGS)

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

## Work out library suffix for the build
LIB_SUFFIX=\\\"$shrext_cmds\\\"
AC_SUBST([LIB_SUFFIX])

## System libraries
AC_CEDAR_HEADERS([Boost], , , [AC_MSG_ERROR([Boost is required])])
oldCPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS -I$BOOSTINCPATH"
BOOSTERRMSG="You need at least version 1.34.0 of Boost for this installation."
BOOSTERRMSG="$BOOSTERRMSG If you want to use Boost from a non-standard location,"
BOOSTERRMSG="$BOOSTERRMSG please specify '--with-boost-incpath=/path/to/boost_1_35/include'"
BOOSTERRMSG="$BOOSTERRMSG as option to the configure script or run 'export"
BOOSTERRMSG="$BOOSTERRMSG BOOSTFLAGS=--with-boost-incpath=/path/to/boost_1_35/include'"
BOOSTERRMSG="$BOOSTERRMSG before you run the rivet-bootstrap script."
AC_CHECK_HEADER([boost/foreach.hpp], [], [AC_MSG_ERROR([Boost foreach not found. $BOOSTERRMSG])])
AC_CHECK_HEADER([boost/smart_ptr.hpp], [], [AC_MSG_ERROR([Boost smart_ptr not found. $BOOSTERRMSG])])
AC_CHECK_HEADER([boost/lexical_cast.hpp], [], [AC_MSG_ERROR([Boost lexical_cast not found. $BOOSTERRMSG])])
AC_CHECK_HEADER([boost/assign.hpp], [], [AC_MSG_ERROR([Boost assign not found. $BOOSTERRMSG])])
CPPFLAGS=$oldCPPFLAGS

## Testing HepMC
AC_CEDAR_LIBRARYANDHEADERS([HepMC], , , [AC_MSG_ERROR([HepMC is required])])
#AC_CEDAR_LIBRARY([HepMCfio], , , [AC_MSG_ERROR([HepMCfio is required])])

## Standard build flags
AM_CPPFLAGS="-I\$(top_srcdir)/include -I\$(top_builddir)/include "
AM_CPPFLAGS="$AM_CPPFLAGS -I\$(BOOSTINCPATH) \$(BOOST_CPPFLAGS)"
AM_CPPFLAGS="$AM_CPPFLAGS -I\$(HEPMCINCPATH) "
AM_LDFLAGS="-L\$(top_builddir)/src/Tools -L\$(top_builddir)/src"
AM_CXXFLAGS="$AM_CXXFLAGS -DLIB_SUFFIX=$LIB_SUFFIX "
AC_CEDAR_CHECKCXXFLAG([-pedantic], [AM_CXXFLAGS="$AM_CXXFLAGS -pedantic "])
AC_CEDAR_CHECKCXXFLAG([-ansi], [AM_CXXFLAGS="$AM_CXXFLAGS -ansi "])
AC_CEDAR_CHECKCXXFLAG([-Wall], [AM_CXXFLAGS="$AM_CXXFLAGS -Wall "])
AC_CEDAR_CHECKCXXFLAG([-Wno-long-long], [AM_CXXFLAGS="$AM_CXXFLAGS -Wno-long-long "])


## Debug flag (default=none)
AC_ARG_ENABLE([debug], [AC_HELP_STRING(--enable-debug,
  [build with debugging symbols @<:@default=no@:>@])], [], [enable_debug=no])
if test x$enable_debug == xyes; then
  AC_CEDAR_CHECKCXXFLAG([-g], [AM_CXXFLAGS="$AM_CXXFLAGS -g "])
fi

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(AM_CXXFLAGS)

## Fortran build flags
FORTRANLDFLAGS="-L$HEPMCLIBPATH" # -L$HEPMCFIOLIBPATH"
FORTRANLIBADD="$FCLIBS" # -l$HEPMCFIOLIBNAME
AC_SUBST(FORTRANLDFLAGS)
AC_SUBST(FORTRANLIBADD)

## Dlopen export flags
DLOPENFLAGS="-export-dynamic"
AC_SUBST(DLOPENFLAGS)


## Python extension
AC_ARG_ENABLE(pyext, [AC_HELP_STRING(--disable-pyext,
  [don't build Python module (default=build)])],
  [], [enable_pyext=yes])
AZ_PYTHON_DEFAULT
## Basic Python checks
if test x$enable_pyext == xyes; then
  AZ_PYTHON_PATH
  AZ_PYTHON_VERSION_ENSURE([2.4])
  PYTHON_VERSION=`$PYTHON -c "import sys; print '.'.join(map(str, sys.version_info@<:@:2@:>@));"`
  AC_SUBST(PYTHON_VERSION)
  AGILE_PYTHONPATH=`$PYTHON -c "import distutils.sysconfig; print distutils.sysconfig.get_python_lib(prefix='$prefix', plat_specific=True);"`
  AC_SUBST(AGILE_PYTHONPATH)
  ## Test for Python header
  if test -x "$PYTHON"; then
    AC_MSG_CHECKING([for Python include path])
    python_incpath=`$PYTHON -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc();"`
    AC_MSG_RESULT([$python_incpath])
    python_header="$python_incpath/Python.h"
    if test -z "$python_incpath"; then
      AC_MSG_ERROR([Can't build Python extension since include directory cannot be determined from distutils])
      enable_pyext=no
    elif test ! -e "$python_header"; then
      AC_MSG_ERROR([Can't build Python extension since header file $python_header cannot be found])
      enable_pyext=no
    fi
  else
    AC_MSG_ERROR([Can't build Python extension since python can't be found])
    enable_pyext=no
  fi
fi
## SWIG checks
if test x$enable_pyext == xyes; then
  AC_PROG_SWIG
  if test x$SWIG == x; then
    AC_MSG_ERROR([Can't build Python extension since swig could not be found])
    enable_pyext=no
  else
    ## Test that SWIG makes a compilable source file...
    ## cf. g++ 4.x requires that string literal is "_const_ char*"
    cat > conftest.i <<EOL
%module conftest
%{
  void foo();
%}
void foo();
EOL
    AC_MSG_CHECKING([if $SWIG can make a Python function wrapper])
    flag_ok=yes
    stat_string=`$SWIG -c++ -python conftest.i 2>&1 1>&5` ; test -z "$stat_string" || flag_ok=no
    AC_MSG_RESULT([$flag_ok])
    if test x$flag_ok != xyes; then
      AC_MSG_ERROR([Can't build Python extension since $SWIG is not able to make a Python wrapper])
      enable_pyext=no
    else
      swig_compiler=$CXX
      AC_CEDAR_CHECKCXXFLAG([-Wno-format], [AC_MSG_NOTICE([adding -Wno-format to swig compilation test]);
        swig_compiler="$swig_compiler -Wno-format"])
      AC_MSG_CHECKING([if $SWIG is compatible with the $CXX compiler])
      flag_ok=yes
      if test x$flag_ok == xyes; then
        stat_string=`$swig_compiler -c conftest_wrap.cxx $CPPFLAGS $CXXFLAGS -I$python_incpath 2>&1 1>&5` ; test -z "$stat_string" || flag_ok=no
      fi
      AC_MSG_RESULT([$flag_ok])
      if test x$flag_ok != xyes; then
        AC_MSG_ERROR([Can't build Python extension since $SWIG is not compatible with $CXX. Get a newer SWIG version!])
        enable_pyext=no
      fi
    fi
  fi
  rm -rf conftest*
fi
## Finish
if test x$enable_pyext == xyes; then
  AC_MSG_NOTICE([All Python build checks successful: 'AGILe' Python extension will be built])
fi
AM_CONDITIONAL(ENABLE_PYEXT, [test x$enable_pyext == xyes])


AC_EMPTY_SUBST
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(include/Makefile include/AGILe/Makefile)
AC_CONFIG_FILES(src/Makefile src/Core/Makefile src/Tools/Makefile src/IO/Makefile)
AC_CONFIG_FILES(src/FPythia/Makefile)
AC_CONFIG_FILES(src/FHerwig/Makefile)
AC_CONFIG_FILES(src/AlpGen/Makefile)
AC_CONFIG_FILES(src/Charybdis/Makefile)
AC_CONFIG_FILES(src/Rapgap/Makefile)
AC_CONFIG_FILES(src/Cascade/Makefile)
AC_CONFIG_FILES(src/Phojet/Makefile)
AC_CONFIG_FILES(src/Ariadne/Makefile)
AC_CONFIG_FILES(bin/Makefile bin/agile-config)
AC_CONFIG_FILES(pyext/Makefile pyext/AGILe/Makefile pyext/setup.py)
AC_CONFIG_FILES(data/Makefile)
AC_CONFIG_FILES(test/Makefile)
AC_CONFIG_FILES(agileenv.sh agileenv.csh)

AC_OUTPUT

if test x$enable_python == xyes; then
   cat <<EOF

************************************************************
AGILe CONFIGURED!

Now build and install (to the $prefix tree) with e.g.
make -j2 && make -j2 install

To use AGILe, either link against the AGILe library with
your own main program, or use the agile-runmc command line
program.

For a more pleasant command line experience, you can include
the data/agile-completion file into your .bashrc file
(or your bash_completion.d directory if you have one.)

The agileenv.*sh files will not be installed, but can help you
to set up an AGILe runtime environment in future.
************************************************************
EOF
fi
