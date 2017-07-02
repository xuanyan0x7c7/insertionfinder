SUBDIRS = commands data-structure
bin_PROGRAMS = insertionfinder
insertionfinder_SOURCES = algorithm.h cube.h formula.h \
    algorithm.c cube.c formula.c main.c
insertionfinder_CPPFLAGS = -Icommands -Idata-structure
insertionfinder_LDADD = commands/libcommands.a \
    data-structure/libdatastructure.a finder/libfinder.a
