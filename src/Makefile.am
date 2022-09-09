AM_CPPFLAGS = -I$(top_srcdir)/include
SUBDIRS = algorithm case cli cube finder improver insertion twist utils
bin_PROGRAMS = insertionfinder
insertionfinder_SOURCES = main.cpp

if USE_SYS_UNIVALUE
insertionfinder_LDADD = \
    cli/libcli.la \
    improver/libimprover.la \
    finder/libfinder.la \
    case/libcase.la \
    cube/libcube.la \
    insertion/libinsertion.la \
    algorithm/libalgorithm.la \
    twist/libtwist.la \
    utils/libutils.la \
    ${PTHREAD_CFLAGS} \
    ${BOOST_SYSTEM_LIBS} \
    ${BOOST_FILESYSTEM_LIBS} \
    ${BOOST_PROGRAM_OPTIONS_LIBS} \
    -lunivalue
libinsertionfinder_la_LIBADD = \
    improver/libimprover.la \
    finder/libfinder.la \
    case/libcase.la \
    cube/libcube.la \
    insertion/libinsertion.la \
    algorithm/libalgorithm.la \
    twist/libtwist.la \
    utils/libutils.la
else
insertionfinder_LDADD = \
    cli/libcli.la \
    improver/libimprover.la \
    finder/libfinder.la \
    case/libcase.la \
    cube/libcube.la \
    insertion/libinsertion.la \
    algorithm/libalgorithm.la \
    twist/libtwist.la \
    utils/libutils.la \
    ${PTHREAD_CFLAGS} \
    ${BOOST_SYSTEM_LIBS} \
    ${BOOST_FILESYSTEM_LIBS} \
    ${BOOST_PROGRAM_OPTIONS_LIBS} \
    $(top_srcdir)/vendors/univalue/libunivalue.la
libinsertionfinder_la_LIBADD = \
    improver/libimprover.la \
    finder/libfinder.la \
    case/libcase.la \
    cube/libcube.la \
    insertion/libinsertion.la \
    algorithm/libalgorithm.la \
    twist/libtwist.la \
    utils/libutils.la \
    $(top_srcdir)/vendors/univalue/libunivalue.la
endif

lib_LTLIBRARIES = libinsertionfinder.la
libinsertionfinder_la_SOURCES =

install-exec-hook:
	cd $(DESTDIR)$(bindir) \
		&& mv -f insertionfinder${EXEEXT} insertionfinder-${VERSION}${EXEEXT} \
		&& $(LN_S) insertionfinder-${VERSION}${EXEEXT} insertionfinder${EXEEXT}

uninstall-local:
	cd $(DESTDIR)$(bindir) && rm -f insertionfinder-${VERSION}${EXEEXT}
