if USE_SYS_UNIVALUE
AM_CPPFLAGS = -I$(top_srcdir)/include \
    -DALGORITHMSDIR=\"$(datadir)/$(PACKAGE)/$(VERSION)/algorithms\"
noinst_LTLIBRARIES = libcli.la
libcli_la_SOURCES = commands.hpp parser.hpp utils.hpp \
    generate.cpp improve.cpp parser.cpp search.cpp utils.cpp verify.cpp
else
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/vendors/univalue/include \
    -DALGORITHMSDIR=\"$(datadir)/$(PACKAGE)/$(VERSION)/algorithms\"
noinst_LTLIBRARIES = libcli.la
libcli_la_SOURCES = commands.hpp parser.hpp utils.hpp \
    generate.cpp improve.cpp parser.cpp search.cpp utils.cpp verify.cpp
endif
