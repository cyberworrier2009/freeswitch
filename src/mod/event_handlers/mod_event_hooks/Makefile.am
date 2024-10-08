include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_event_hook

mod_LTLIBRARIES = mod_event_hook.la
mod_event_hook_la_SOURCES  = mod_event_hook.c 
mod_event_hook_CFLAGS   = $(AM_CFLAGS) -g
mod_event_hook_CPPFLAGS = $(AM_CPPFLAGS) 
mod_event_hook_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_event_hook_LDFLAGS  = -avoid-version -module -no-undefined -shared

