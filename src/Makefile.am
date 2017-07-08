SUBDIRS = algorithm commands cube data-structure finder formula
bin_PROGRAMS = insertionfinder
insertionfinder_SOURCES = init.h init.c main.c
insertionfinder_LDADD = commands/libcommands.a \
	finder/libfinder.a \
	algorithm/libalgorithm.a \
	cube/libcube.a \
	formula/libformula.a \
	data-structure/libdatastructure.a \
	$(json_LIBS) \
	-lm
AM_LDFLAGS = -pthread

install-exec-hook:
	cd $(DESTDIR)$(bindir) \
		&& mv -f insertionfinder${EXEEXT} insertionfinder-${VERSION}${EXEEXT} \
		&& $(LN_S) insertionfinder-${VERSION}${EXEEXT} insertionfinder${EXEEXT}

uninstall-local:
	cd $(DESTDIR)$(bindir) && rm -f insertionfinder-${VERSION}${EXEEXT}
