AC_INIT([insertionfinder], [0.0], [xuanyan@xuanyan.ws])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR

AC_PROG_CC
AC_PROG_RANLIB

AC_CHECK_HEADERS([getopt.h regex.h])
AC_CHECK_FUNCS([strdup])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/commands/Makefile
    src/data-structure/Makefile
])

AC_OUTPUT
