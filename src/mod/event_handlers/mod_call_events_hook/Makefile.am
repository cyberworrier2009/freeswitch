include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_call_events_hook

mod_LTLIBRARIES = mod_call_events_hook.la
mod_call_events_hook_la_SOURCES  = mod_call_events_hook.c data_structs/structs.c 
mod_call_events_hook_la_CFLAGS   = $(AM_CFLAGS) -g -I$(srcdir)/data_structs 
mod_call_events_hook_la_LIBADD   = $(switch_builddir)/libfreeswitch.la -lcurl
mod_call_events_hook_la_LDFLAGS  = -avoid-version -module -no-undefined -shared
