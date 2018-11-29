AC_INIT([insertionfinder], [0.4.8], [xuanyan@xuanyan.ws])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
LT_INIT

AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX([17])
AX_PTHREAD

AC_CHECK_HEADERS([optional])
AC_CHECK_HEADERS([filesystem])

AC_CONFIG_HEADERS([config.h])
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
