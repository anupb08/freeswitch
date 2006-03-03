#include <switch.h>

void fs_console_log(char *msg)
{
	switch_console_printf(SWITCH_CHANNEL_CONSOLE, msg);
}

void fs_console_clean(char *msg)
{
	switch_console_printf(SWITCH_CHANNEL_CONSOLE_CLEAN, msg);
}

struct switch_core_session *fs_core_session_locate(char *uuid)
{
	return switch_core_session_locate(uuid);
}

void fs_channel_answer(struct switch_core_session *session)
{
	switch_channel *channel = switch_core_session_get_channel(session);
	switch_channel_answer(channel);
}

void fs_channel_pre_answer(struct switch_core_session *session)
{
	switch_channel *channel = switch_core_session_get_channel(session);
	switch_channel_pre_answer(channel);
}

void fs_channel_hangup(struct switch_core_session *session)
{
	switch_channel *channel = switch_core_session_get_channel(session);
	switch_channel_hangup(channel);
}

void fs_channel_set_variable(struct switch_core_session *session, char *var, char *val)
{
	switch_channel *channel = switch_core_session_get_channel(session);
	switch_channel_set_variable(channel, var, val);
}

void fs_channel_get_variable(struct switch_core_session *session, char *var)
{
	switch_channel *channel = switch_core_session_get_channel(session);
	switch_channel_get_variable(channel, var);
}

void fs_channel_set_state(struct switch_core_session *session, char *state)
{
	switch_channel *channel = switch_core_session_get_channel(session);
	switch_channel_state fs_state = switch_channel_get_state(channel);

	if (!strcmp(state, "EXECUTE")) {
		fs_state = CS_EXECUTE;
	} else 	if (!strcmp(state, "TRANSMIT")) {
		fs_state = CS_TRANSMIT;
	}
	
	switch_channel_set_state(channel, fs_state);
}

int fs_ivr_play_file(struct switch_core_session *session, char *file, char *timer_name_in) 
{
	char *timer_name = switch_strlen_zero(timer_name_in) ? NULL : timer_name;
	switch_status status = switch_ivr_play_file(session, NULL, file, timer_name, NULL, NULL, 0);
	return status == SWITCH_STATUS_SUCCESS ? 1 : 0;
}

