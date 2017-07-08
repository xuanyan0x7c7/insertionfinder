AC_INIT([insertionfinder], [0.1], [xuanyan@xuanyan.ws])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR

AC_PROG_CC_C99
AC_PROG_RANLIB

AH_TEMPLATE([PREFIX], ["Program file prefix"])
AM_CONDITIONAL([PREFIX_DEFINED], [test $prefix = NONE])
AM_COND_IF(
    [PREFIX_DEFINED],
    AC_DEFINE([PREFIX], ["/usr/local"]),
    AC_DEFINE_UNQUOTED([PREFIX], ["$prefix"])
)

AC_CHECK_HEADERS([getopt.h pthread.h regex.h unistd.h])
AC_CHECK_FUNCS([strdup open_memstream])

PKG_CHECK_MODULES([json], [json-glib-1.0])

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
    data/algorithms/Makefile
])

AC_OUTPUT
