#include <arraylist.h>
#include <switch.h>

SWITCH_MODULE_LOAD_FUNCTION(mod_ban_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_ban_shutdown);
SWITCH_MODULE_DEFINITION(mod_ban, mod_ban_load, mod_ban_shutdown, NULL);

// static switch_event_node_t *node = NULL;
arraylist *bad_ip_list = NULL;

struct bad_ip_information {
	char *ip;
	int num_attempts;
} bad_ip_information;

void add_bad_ip(char *ip)
{

	struct bad_ip_information *info = malloc(sizeof(bad_ip_information));

	if (!bad_ip_list) { bad_ip_list = arraylist_create(); }

	info->ip = strdup(ip);
	info->num_attempts = info->num_attempts + 1;
	arraylist_add(bad_ip_list, info);
}

static void event_handler_ip(switch_event_t *event)
{
	// Early return if event is NULL
	if (event == NULL) { return; }

	

	// Check if event is custom and has a subclass name
	if (event->event_id == SWITCH_EVENT_CUSTOM && event->subclass_name) {
		char *name = switch_event_get_header(event, "to-user");
		char *ip = switch_event_get_header(event, "network-ip");
		
		if (strncmp(event->subclass_name, "sofia::register_failure", 23) == 0) {
			if (bad_ip_list == NULL) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "adding items to bad list\n");
				add_bad_ip(ip);
			} else {
				struct bad_ip_information *info = NULL;
				for (int i = 0; i < bad_ip_list->size; i++) {
					info = arraylist_get(bad_ip_list, i);
					if (strcmp(info->ip, ip) == 0) { break; }
					info = NULL;
				}

				if (info == NULL) {
					// IP not found in list, so add it
					info = (struct bad_ip_information *)malloc(sizeof(struct bad_ip_information));
					if (info == NULL) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Memory allocation failed\n");
						return;
					}
					info->ip = ip;
					info->num_attempts = 1;
					arraylist_add(bad_ip_list, info);
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
									  "Received event %d Network-ip %s added to black list attempt\n", event->event_id,
									  ip);
				} else {
					// IP found in list, so increment attempts
					info->num_attempts++;
					if (info->num_attempts > 3) {
						char command[256];
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
										  "Registration failure for user %s from network ip %s\n", name, ip);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Banning ip %s\n", ip);
						snprintf(command, sizeof(command), "iptables -A INPUT -s %s -j DROP", ip);
						system(command);
					}
				}
			}
		}
	}
}

SWITCH_STANDARD_API(ban)
{
	switch_event_bind("mod_ban", SWITCH_EVENT_CUSTOM, SWITCH_EVENT_SUBCLASS_ANY, event_handler_ip, NULL);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Ban API called\n");
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_ban_load)
{
	switch_api_interface_t *api_interface;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	SWITCH_ADD_API(api_interface, "ban", "Ban unauthorized users", ban, "syntax");
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_ban_shutdown)
{
	// switch_event_unbind(&node);
	switch_event_unbind_callback(event_handler_ip);
	if (bad_ip_list != NULL) arraylist_destroy(bad_ip_list);
	return SWITCH_STATUS_SUCCESS;
}
