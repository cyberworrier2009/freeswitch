#include <switch.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "data_structs/structs.h"

SWITCH_MODULE_LOAD_FUNCTION(mod_call_events_hook_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_call_events_hook_shutdown);
SWITCH_MODULE_DEFINITION(mod_call_events_hook, mod_call_events_hook_load, mod_call_events_hook_shutdown, NULL);

static switch_event_node_t *event_node = NULL;
static switch_hash_t *call_recordings_hash = NULL;

array_list_struct_t *call_recordings = NULL;

typedef struct call_event_info {
    char* event;
    char* domain;
    char* destination_number;
    char* caller_number;
    char* call_direction;
	char* call_uuid;
	char* caller_name;
    char* recording_file_path;
} call_event_info_t;

static void free_call_event_info(call_event_info_t *info) {
    if (info) {
        switch_safe_free(info->event);
        switch_safe_free(info->domain);
        switch_safe_free(info->destination_number);
        switch_safe_free(info->call_direction);
        switch_safe_free(info->caller_number);
		switch_safe_free(info->call_uuid);
        if(info->recording_file_path != NULL){
            switch_safe_free(info->recording_file_path);
        } 
        switch_safe_free(info->caller_name);      
		switch_safe_free(info);
    }
}
char * parse_call_recordings(array_list_struct_t *call_recordings){
    cJSON *root_object = cJSON_CreateObject();
    cJSON *call_recordings_json = cJSON_CreateArray();
    char *json_string = NULL;
    
    for (int i=0; i<call_recordings->array_size;i++){
        call_event_info_t *recording_info = (call_event_info_t *) call_recordings->data[i];
        if(recording_info){
            cJSON *call_record = cJSON_CreateObject();
            cJSON_AddItemToObject(call_record,"event", cJSON_CreateString(recording_info->event ? recording_info->event : ""));
            cJSON_AddItemToObject(call_record,"domain", cJSON_CreateString(recording_info->domain ? recording_info->domain : ""));
            cJSON_AddItemToObject(call_record,"destination_number", cJSON_CreateString(recording_info->destination_number ? recording_info->destination_number : ""));
            cJSON_AddItemToObject(call_record,"caller_number", cJSON_CreateString(recording_info->caller_number ? recording_info->caller_number : ""));
            cJSON_AddItemToObject(call_record,"call_direction", cJSON_CreateString(recording_info->call_direction ? recording_info->call_direction : ""));
            cJSON_AddItemToObject(call_record,"call_uuid", cJSON_CreateString(recording_info->call_uuid ? recording_info->call_uuid : ""));
            cJSON_AddItemToObject(call_record,"caller_name", cJSON_CreateString(recording_info->caller_name ? recording_info->caller_name : ""));
            cJSON_AddItemToObject(call_record,"recording_file_path", cJSON_CreateString(recording_info->recording_file_path ? recording_info->recording_file_path : ""));
            cJSON_AddItemToArray(call_recordings_json, call_record);
        }          
    }
    cJSON_AddItemToObject(root_object, "event_name", cJSON_CreateString("call_recordings"));
    cJSON_AddItemToObject(root_object, "call_recordings", call_recordings_json);
    switch_log_printf(SWITCH_CHANNEL_LOG,SWITCH_LOG_INFO, "Parsed call recordings: %s\n", cJSON_Print(root_object) ? cJSON_Print(root_object) : "null");
    json_string = cJSON_Print(root_object);
    cJSON_Delete(root_object);
   
    return json_string; 
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
        cJSON_AddStringToObject(data, "recording_file_path", obj->recording_file_path ? obj->recording_file_path : "");
        cJSON_AddItemToObject(root,"data",data);
		json_string = cJSON_Print(root);
		cJSON_Delete(root);
    }
    return json_string;
}

static void send_event_data(char *post_body) {
    CURL *curl = curl_easy_init();
    if (curl) {
		
        if (post_body) {
            CURLcode res;
            const char *url = "http://localhost:8081/event";  // Replace with your actual endpoint
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
    call_event_info_t *call_info = NULL;
    char *call_recordings_body = NULL; 
    char *call_info_body = NULL;
    if (event_name) {
        const char *domain_name = switch_event_get_header(event, "variable_domain_name");
        const char *call_direction = switch_event_get_header(event, "Caller-Direction");
        const char *callee_number = switch_event_get_header(event, "Caller-Destination-Number");
        const char *caller_number = switch_event_get_header(event, "Caller-Caller-ID-Number");
		const char *caller_name = switch_event_get_header(event, "Caller-Caller-ID-Name");
		const char *call_uuid = switch_event_get_header(event, "Caller-Unique-ID");
        const char *recording_file = switch_event_get_header(event, "Record-File-Path");
        if (!domain_name) {
            domain_name = "unknown";
        }
        if(strcmp(event_name, "CHANNEL_CREATE")==0){
            call_recordings = create_array_list();
            switch_core_hash_insert(call_recordings_hash, call_uuid, call_recordings);
        } 
        if (strcmp(event_name, "CHANNEL_ANSWER") == 0 ||
            strcmp(event_name, "CHANNEL_HANGUP") == 0 ||
            strcmp(event_name, "CHANNEL_CREATE") == 0 ||
            strcmp(event_name, "CHANNEL_PROGRESS") == 0||
            strcmp(event_name, "RECORD_STOP") == 0||
            strcmp(event_name, "CHANNEL_PROGRESS_MEDIA") == 0 ||
            strcmp(event_name, "CHANNEL_HANGUP_COMPLETE") == 0 ){ 
            
                if(strcmp(event_name, "RECORD_STOP")==0 && switch_file_exists(recording_file, NULL) == SWITCH_STATUS_SUCCESS){
                    
                    call_event_info_t *record_info = (call_event_info_t*) malloc(sizeof(call_event_info_t)); 
                    if(recording_file && record_info){
                        
                        record_info->call_uuid = switch_safe_strdup(call_uuid);
                        record_info->event = switch_safe_strdup(event_name);
                        record_info->domain = switch_safe_strdup(domain_name);
                        record_info->destination_number = switch_safe_strdup(callee_number);
                        record_info->caller_number = switch_safe_strdup(caller_number);
                        record_info->caller_name = switch_safe_strdup(caller_name);
                        record_info->call_direction = switch_safe_strdup(call_direction); 
                        record_info->recording_file_path = switch_safe_strdup(recording_file);
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Recording file path: %s\n", recording_file);
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO,
                            "call_event_info_t struct logging:\n"
                            "  event: %s\n"
                            "  domain: %s\n"
                            "  destination_number: %s\n"
                            "  caller_number: %s\n"
                            "  call_direction: %s\n"
                            "  caller_name: %s\n"
                            "  call_uuid: %s\n"
                            "  recording_file_path: %s\n",
                            record_info->event ? record_info->event : "(null)",
                            record_info->domain ? record_info->domain : "(null)",
                            record_info->destination_number ? record_info->destination_number : "(null)",
                            record_info->caller_number ? record_info->caller_number : "(null)",
                            record_info->call_direction ?  record_info->call_direction : "(null)",
                            record_info->caller_name ? record_info->caller_name : "(null)",
                            record_info->call_uuid ? record_info->call_uuid : "(null)",
                            record_info->recording_file_path ? record_info->recording_file_path : "(null)"
                        );
                     
                        call_recordings = switch_core_hash_find(call_recordings_hash, call_uuid);
                        add_array_element(call_recordings, record_info);
                    }
                    else{
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to allocate memory for record_info\n");
                                       } 
                }
                else{
                   call_info = (call_event_info_t*) malloc(sizeof(call_event_info_t)); 
                    if (call_info) {
                    call_info->domain = switch_safe_strdup(domain_name);
                    call_info->destination_number = switch_safe_strdup(callee_number);
                    call_info->event = switch_safe_strdup(event_name);
                    call_info->caller_number = switch_safe_strdup(caller_number);
                    call_info->call_direction = switch_safe_strdup(call_direction);
                    call_info->call_uuid = switch_safe_strdup(call_uuid);
                    call_info->caller_name = switch_safe_strdup(caller_name);
                    call_info->recording_file_path = switch_safe_strdup("");
                    if(strcmp(event_name, "CHANNEL_PROGRESS_MEDIA")==0){
                        call_info->event = switch_safe_strdup("CS_RINGING");
                    }
                    else{
                        call_info->event =switch_safe_strdup(event_name);
                    }

                    if(strcmp(event_name, "CHANNEL_HANGUP_COMPLETE") == 0){
                        call_recordings = switch_core_hash_find(call_recordings_hash, call_uuid);
                        if(call_recordings && call_recordings->array_size > 0){
                            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Call recordings found, processing them. Call recordings: %d\n", array_size(call_recordings));
                            call_recordings_body = parse_call_recordings(call_recordings); 
                            send_event_data(call_recordings_body);
                            free_array_list(call_recordings);
                        }
                       switch_core_hash_delete(call_recordings_hash, call_uuid); 
                    }
                    call_info_body = convert_to_json(call_info);
                    send_event_data(call_info_body);
                    free_call_event_info(call_info);
            } else {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to allocate memory for call_info\n");
            } 
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
    switch_core_hash_init(&call_recordings_hash);
    SWITCH_ADD_API(api_interface, "call_events_hook", "Domain specific event hooks", mod_call_events_hook_start_function, "<start>|<stop>");

    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_call_events_hook_shutdown) {
    if (event_node != NULL) {
        switch_event_unbind(&event_node);
        event_node = NULL;
    }
    return SWITCH_STATUS_SUCCESS;
}
