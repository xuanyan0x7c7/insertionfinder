AM_CPPFLAGS = -I$(top_srcdir)/include $(BOOST_CPPFLAGS) -DBOOST_TEST_DYN_LINK
TESTS = insertionfinder-test
check_PROGRAMS = insertionfinder-test
AM_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS)
insertionfinder_test_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIBS)
insertionfinder_test_SOURCES = algorithm.cpp
