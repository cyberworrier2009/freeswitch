#include <switch.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

SWITCH_MODULE_LOAD_FUNCTION(mod_call_events_hook_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_call_events_hook_shutdown);
SWITCH_MODULE_DEFINITION(mod_call_events_hook, mod_call_events_hook_load, mod_call_events_hook_shutdown, NULL);

static switch_event_node_t *event_node = NULL;
typedef struct call_event_info {
    char* event;
    char* domain;
    char* destination_number;
    char* caller_number;
    char* call_direction;
	char* call_uuid;
	char* caller_name;
} call_event_info_t;

static void free_call_event_info(call_event_info_t *info) {
    if (info) {
        switch_safe_free(info->event);
        switch_safe_free(info->domain);
        switch_safe_free(info->destination_number);
        switch_safe_free(info->call_direction);
        switch_safe_free(info->caller_number);
		switch_safe_free(info->call_uuid);
  switch_safe_free(info->caller_name);      
		switch_safe_free(info);
    }
}

char *convert_to_json(call_event_info_t *obj) {
    char *json_string = NULL;
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    if (root) {
  
      
		cJSON_AddStringToObject(root, "name", obj->event);
                cJSON_AddStringToObject(data, "domain", obj->domain ? obj->domain : "");
        cJSON_AddStringToObject(data, "event", obj->event ? obj->event : "");
        cJSON_AddStringToObject(data, "destination_number", obj->destination_number ? obj->destination_number : "");
        cJSON_AddStringToObject(data, "direction", obj->call_direction ? obj->call_direction : "");
        cJSON_AddStringToObject(data, "caller_number", obj->caller_number);
		cJSON_AddStringToObject(data, "call_uuid", obj->call_uuid );
  cJSON_AddStringToObject(data,"caller_name", obj->caller_name); 
		cJSON_AddItemToObject(root,"data",data);
		json_string = cJSON_Print(root);
		cJSON_Delete(root);
    }
    return json_string;
}

static void send_event_data(call_event_info_t *data) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char *post_body = convert_to_json(data);
		if (post_body) {
            CURLcode res;
            const char *url = "http://localhost:8080/event";  // Replace with your actual endpoint
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body);
            res = curl_easy_perform(curl);
                        if (res != CURLE_OK) {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                                  "curl_easy_perform() failed: %s\n",
                                  curl_easy_strerror(res));
            }

            free(post_body);
        }
        curl_easy_cleanup(curl);
    } else {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                          "Failed to initialize CURL\n");
    }
}

static void event_handler(switch_event_t *event) {
    const char *event_name = switch_event_get_header(event, "Event-Name");

    if (event_name) {
        const char *domain_name = switch_event_get_header(event, "variable_domain_name");
        const char *call_direction = switch_event_get_header(event, "Caller-Direction");
        const char *callee_number = switch_event_get_header(event, "Caller-Destination-Number");
        const char *caller_number = switch_event_get_header(event, "Caller-Caller-ID-Number");
		const char *caller_name = switch_event_get_header(event, "Caller-Caller-ID-Name");
		const char *call_uuid = switch_event_get_header(event, "Caller-Unique-ID");
        if (!domain_name) {
            domain_name = "unknown";
        }

        if (strcmp(event_name, "CHANNEL_ANSWER") == 0 ||
            strcmp(event_name, "CHANNEL_HANGUP") == 0 ||
            strcmp(event_name, "CHANNEL_CREATE") == 0 ||
            strcmp(event_name, "CHANNEL_PROGRESS") == 0||
            strcmp(event_name, "CHANNEL_PROGRESS_MEDIA") == 0){ 
            
            call_event_info_t *call_info = (call_event_info_t*) malloc(sizeof(call_event_info_t));
            if (call_info) {
               
                call_info->domain = switch_safe_strdup(domain_name);
                call_info->destination_number = switch_safe_strdup(callee_number);
                call_info->event = switch_safe_strdup(event_name);
                call_info->caller_number = switch_safe_strdup(caller_number);
                call_info->call_direction = switch_safe_strdup(call_direction);
				call_info->call_uuid = switch_safe_strdup(call_uuid);
				call_info->caller_name = switch_safe_strdup(caller_name);
                if(strcmp(event_name, "CHANNEL_PROGRESS_MEDIA")==0){
                    call_info->event = switch_safe_strdup("CS_RINGING");
                }
                else{
                    call_info->event =switch_safe_strdup(event_name);
                }
				send_event_data(call_info);
                free_call_event_info(call_info);
            } else {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to allocate memory for call_info\n");
            }
        }
    }
}

SWITCH_STANDARD_API(mod_call_events_hook_start_function) {
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

SWITCH_MODULE_LOAD_FUNCTION(mod_call_events_hook_load) {
    switch_api_interface_t *api_interface;
    *module_interface = switch_loadable_module_create_module_interface(pool, modname);

    SWITCH_ADD_API(api_interface, "call_event_hooks", "Domain specific event hooks", mod_call_events_hook_start_function, "<start>|<stop>");

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_call_events_hook_shutdown) {
    if (event_node != NULL) {
        switch_event_unbind(&event_node);
        event_node = NULL;
    }
    return SWITCH_STATUS_SUCCESS;
}