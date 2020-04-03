AC_INIT([insertionfinder], [0.4.23], [xuanyan@xuanyan.ws])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
LT_INIT

AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([17])
AX_PTHREAD

AC_CHECK_HEADERS([univalue.h], [], [AC_MSG_ERROR([unable to find header univalue.h])])
AC_CHECK_HEADERS([range/v3/all.hpp], [], [AC_MSG_ERROR([unable to find library range-v3])])
AC_CHECK_HEADERS([filesystem], [HAVE_FILESYSTEM=yes])

BOOST_REQUIRE([1.66.0])
AS_IF([test "$HAVE_FILESYSTEM"], [], [
    BOOST_SYSTEM
    BOOST_FILESYSTEM
])
BOOST_ASIO
BOOST_PROGRAM_OPTIONS
BOOST_TEST

AC_CONFIG_HEADERS([config.h])
AX_PREFIX_CONFIG_H(include/insertionfinder/config.h)
AC_CONFIG_FILES([
    Makefile
    include/Makefile
    src/Makefile
    src/algorithm/Makefile
    src/case/Makefile
    src/cli/Makefile
    src/cube/Makefile
    src/finder/Makefile
    src/improver/Makefile
    src/twist/Makefile
    src/utils/Makefile
    test/Makefile
    data/algorithms/Makefile
    data/algorithms/extras/Makefile
    data/algorithms/slices/Makefile
])

AC_OUTPUT
