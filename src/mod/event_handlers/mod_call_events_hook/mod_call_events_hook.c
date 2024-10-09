#include <switch.h>
SWITCH_MODULE_LOAD_FUNCTION(mod_call_events_hook_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_call_events_hook_shutdown);
SWITCH_MODULE_DEFINITION(mod_call_events_hook, mod_call_events_hook_load, mod_call_events_hook_shutdown, NULL);

static switch_event_node_t *event_node = NULL;

static void event_handler(switch_event_t *event){
    //const *domain = switch_event_get_header(event,"variable_domain_name");
     if (switch_event_get_header(event,"Event-Name")){
        if(!strcmp(switch_event_get_header(event,"Event-Name"), "CHANNEL_ANSWER")){
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Call answer for domain");
        }
        else if(!strcmp(switch_event_get_header(event,"Event-Name"), "CHANNEL_HANGUP")){
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Call hangup for domain");

        }
    }
}

SWITCH_STANDARD_API(mod_call_events_hook_start_function){

if (zstr(cmd)) {
        stream->write_function(stream, "-ERR Invalid Input\n");
        return SWITCH_STATUS_SUCCESS;
    }

    if (!strcmp(cmd, "start")) {
        if (event_node == NULL) {
            if (switch_event_bind_removable("call_event_hooks", SWITCH_EVENT_ALL, SWITCH_EVENT_SUBCLASS_ANY, event_handler, NULL, &event_node) == SWITCH_STATUS_SUCCESS) {
                stream->write_function(stream, "+OK Event hooks started");
            } else {
                stream->write_function(stream, "-ERR Failed to start event hooks\n");
            }
        } else {
            stream->write_function(stream, "-ERR Event hooks already started\n");
        }
    } else if (!strcmp(cmd, "stop")) {
        if (event_node != NULL) {
            switch_event_unbind(&event_node);
            event_node = NULL;
            stream->write_function(stream, "+OK Event hooks stopped\n");
        } else {
            stream->write_function(stream, "-ERR Event hooks not running\n");
        }
    } else {
        stream->write_function(stream, "-ERR Unknown command. Use 'start' or 'stop'\n");
    }

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_call_events_hook_load){
  
switch_api_interface_t *api_interface;
    *module_interface = switch_loadable_module_create_module_interface(pool, modname);

    SWITCH_ADD_API(api_interface, "call_event_hooks", "Domain specific event hooks", mod_call_events_hook_start_function, "<start>|<stop>");

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_call_events_hook_shutdown){
   if(event_node != NULL){
        switch_event_unbind(&event_node);
        event_node = NULL;
    }
    return SWITCH_STATUS_SUCCESS;
}
