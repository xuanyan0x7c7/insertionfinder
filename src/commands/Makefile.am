noinst_LIBRARIES = libcommands.a
libcommands_a_SOURCES = commands.h parser.h utils.h \
	generate-algfile.c parser.c solve.c utils.c verify.c
libcommands_a_CFLAGS = $(json_CFLAGS)
AM_CPPFLAGS = -DALGORITHMSDIR=\"$(datadir)/$(PACKAGE)/$(VERSION)/algorithms\"
