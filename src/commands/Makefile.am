noinst_LTLIBRARIES = libcommands.la
libcommands_la_SOURCES = commands.h parser.h utils.h \
	generate-algfile.c parser.c solve.c utils.c verify.c
libcommands_la_CFLAGS = $(json_CFLAGS)
AM_CPPFLAGS = -DALGORITHMSDIR=\"$(datadir)/$(PACKAGE)/$(VERSION)/algorithms\"
