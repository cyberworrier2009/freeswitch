include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_ban

mod_LTLIBRARIES = mod_ban.la
mod_ban_la_SOURCES  = mod_ban.c ./data_struct/src/arraylist.c
mod_ban_la_CFLAGS   = $(AM_CFLAGS) -g
mod_ban_la_CPPFLAGS = $(AM_CPPFLAGS) -I/home/osama/src/freeswitch/src/mod/event_handlers/mod_ban/data_struct/src
mod_ban_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_ban_la_LDFLAGS  = -avoid-version -module -no-undefined -shared

