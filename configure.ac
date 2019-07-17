AC_INIT([insertionfinder], [0.4.16], [xuanyan@xuanyan.ws])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
LT_INIT

AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([17])
AX_PTHREAD

AC_CHECK_HEADERS([optional], [HAVE_OPTIONAL=yes])
AC_CHECK_HEADERS([filesystem], [HAVE_FILESYSTEM=yes])

BOOST_REQUIRE
AS_IF([test "$HAVE_OPTIONAL"], [], [BOOST_OPTIONAL])
AS_IF([test "$HAVE_FILESYSTEM"], [], [
    BOOST_SYSTEM
    BOOST_FILESYSTEM
])
BOOST_PROGRAM_OPTIONS

AC_CONFIG_HEADERS([config.h])
AX_PREFIX_CONFIG_H(include/insertionfinder/config.h)
AC_CONFIG_FILES([
    Makefile
    include/Makefile
    src/Makefile
    src/algorithm/Makefile
    src/case/Makefile
    src/cli/Makefile
    src/cli/univalue/Makefile
    src/cube/Makefile
    src/finder/Makefile
    data/algorithms/Makefile
    data/algorithms/extras/Makefile
])

AC_OUTPUT
