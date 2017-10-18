SUBDIRS = algorithm commands cube data-structure finder formula utils
bin_PROGRAMS = insertionfinder
insertionfinder_SOURCES = init.h init.c main.c
insertionfinder_LDADD = commands/libcommands.la \
	finder/libfinder.la \
	algorithm/libalgorithm.la \
	cube/libcube.la \
	formula/libformula.la \
	data-structure/libdatastructure.la \
	utils/libutils.la \
	$(json_LIBS) \
	-lm
AM_LDFLAGS = -pthread

install-exec-hook:
	cd $(DESTDIR)$(bindir) \
		&& mv -f insertionfinder${EXEEXT} insertionfinder-${VERSION}${EXEEXT} \
		&& $(LN_S) insertionfinder-${VERSION}${EXEEXT} insertionfinder${EXEEXT}

uninstall-local:
	cd $(DESTDIR)$(bindir) && rm -f insertionfinder-${VERSION}${EXEEXT}
