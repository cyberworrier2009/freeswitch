#include <switch.h>

static __uint32_t call_counter = 0;
static __uint32_t success_counter = 0;
static __uint32_t failure_counter = 0;
static double total_r = 0;
static double total_mos = 0;

void calcuate_r_mos_stats(switch_core_session_t *session);
switch_status_t report_counter(switch_core_session_t *session);

switch_status_t report_counter(switch_core_session_t *session)
{
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_channel_state_t state = switch_channel_get_state(channel);
	const char *destination_number = switch_channel_get_variable(channel, "destination_number");
	if (state == CS_EXECUTE) {
		if (destination_number && strncmp(destination_number, "sip.telnyx.com", strlen("sip.telnyx.com")) == 0) {
			call_counter++;
		}
	} else if (state == CS_EXCHANGE_MEDIA) {
		if (destination_number && strncmp(destination_number, "sip.telnyx.com", strlen("sip.telnyx.com")) == 0) {
			calcuate_r_mos_stats(session);
			success_counter++;
		}

	} else if (state == CS_HANGUP) {
		if (destination_number && strncmp(destination_number, "sip.telnyx.com", strlen("sip.telnyx.com")) == 0) {
			failure_counter++;
		}
	}
	return SWITCH_STATUS_SUCCESS;
}

void calcuate_r_mos_stats(switch_core_session_t *session)
{

	switch_rtp_stats_t *stats = switch_core_media_get_stats(session, SWITCH_MEDIA_TYPE_AUDIO, NULL);
	total_r = stats->inbound.R;
	total_mos = stats->inbound.mos;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_challenge_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_challenge_shutdown);
SWITCH_MODULE_DEFINITION(mod_challenge, mod_challenge_load, mod_challenge_shutdown, NULL);

static switch_state_handler_table_t state_handlers = {

	.on_routing = report_counter, .on_execute = report_counter, .on_hangup = report_counter};

SWITCH_STANDARD_API(telnyx_stats)
{
	char *output = NULL;
	output = switch_mprintf("Total Calls: %d\nSuccessful Calls: %d\nFailed Calls: %d\n", call_counter, success_counter,
							failure_counter);
	stream->write_function(stream, output, strlen(output));
	switch_safe_free(output);

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_challenge_load)
{
	switch_api_interface_t *api_interface;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	SWITCH_ADD_API(api_interface, "telnyx_stats", "Print Telnyx Call Statistics", telnyx_stats, "syntax");
	switch_core_add_state_handler(&state_handlers);

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_challenge_shutdown)
{
	switch_core_remove_state_handler(&state_handlers);

	return SWITCH_STATUS_SUCCESS;
}
