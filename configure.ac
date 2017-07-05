AC_INIT([insertionfinder], [0.0], [xuanyan@xuanyan.ws])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR

AC_PROG_CC
AC_PROG_RANLIB

AC_CHECK_HEADERS([getopt.h regex.h])
AC_CHECK_FUNCS([strdup])

AH_TEMPLATE([PREFIX], ["Program file prefix"])
AM_CONDITIONAL([PREFIX_DEFINED], [test $prefix = NONE])
AM_COND_IF(
    [PREFIX_DEFINED],
    AC_DEFINE([PREFIX], ["/usr/local"]),
    AC_DEFINE_UNQUOTED([PREFIX], ["$prefix"])
)

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
])

AC_OUTPUT
