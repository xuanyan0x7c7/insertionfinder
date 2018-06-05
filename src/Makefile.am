AM_CPPFLAGS = -I$(top_srcdir)/include
SUBDIRS = algorithm case cli cube finder
bin_PROGRAMS = insertionfinder
insertionfinder_SOURCES = main.cpp
insertionfinder_LDADD = \
    cli/libcli.la \
    cli/univalue/libunivalue.la \
    finder/libfinder.la \
    case/libcase.la \
    cube/libcube.la \
    algorithm/libalgorithm.la \
    -lboost_program_options
