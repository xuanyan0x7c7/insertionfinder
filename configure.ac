AC_INIT([insertionfinder], [0.3.4], [xuanyan@xuanyan.ws])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR
LT_INIT

AC_PROG_CC_C99
AC_PROG_LN_S

AC_CHECK_HEADERS([getopt.h pthread.h regex.h unistd.h])
AC_CHECK_FUNCS([open_memstream clock_gettime])

PKG_CHECK_MODULES([json], [json-glib-1.0 >= 1.1.2])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/algorithm/Makefile
    src/commands/Makefile
    src/cube/Makefile
    src/data-structure/Makefile
    src/finder/Makefile
    src/formula/Makefile
    src/utils/Makefile
    data/algorithms/Makefile
])

AC_OUTPUT
