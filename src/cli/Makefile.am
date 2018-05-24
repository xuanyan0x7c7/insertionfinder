AM_CPPFLAGS = -I$(top_srcdir)/include \
    -DALGORITHMSDIR=\"$(datadir)/$(PACKAGE)/$(VERSION)/algorithms\"
SUBDIRS = univalue
noinst_LTLIBRARIES = libcli.la
libcli_la_SOURCES = commands.hpp parser.hpp \
    generate.cpp parser.cpp search.cpp verify.cpp
