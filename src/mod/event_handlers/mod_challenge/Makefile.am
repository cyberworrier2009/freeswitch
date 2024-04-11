include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_challenge

mod_LTLIBRARIES = mod_challenge.la
mod_event_test_la_SOURCES  = mod_challenge.c
mod_event_test_la_CFLAGS   = $(AM_CFLAGS)
mod_event_test_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_event_test_la_LDFLAGS  = -avoid-version -module -no-undefined -shared
