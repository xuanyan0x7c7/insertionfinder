AM_CPPFLAGS = -I$(top_srcdir)/include
SUBDIRS = algorithm case cli cube
bin_PROGRAMS = insertionfinder
insertionfinder_SOURCES = main.cpp
insertionfinder_LDADD = \
    algorithm/libalgorithm.la \
    case/libcase.la \
    cli/libcli.la \
    cube/libcube.la \
    -lboost_program_options
