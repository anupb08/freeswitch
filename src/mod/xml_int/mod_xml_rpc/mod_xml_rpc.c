/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005/2006, Anthony Minessale II <anthmct@yahoo.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthmct@yahoo.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Anthony Minessale II <anthmct@yahoo.com>
 *
 *
 * mod_xml_rpc.c -- XML RPC
 *
 */
#include <switch.h>
#include <xmlrpc-c/base.h>
#ifdef ABYSS_WIN32
#undef strcasecmp
#endif
#include <xmlrpc-c/abyss.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#include <curl/curl.h>



static const char modname[] = "mod_xml_rpc";



static struct {
	uint16_t port;
	uint8_t running;
	char *url;
	char *bindings;
	char *realm;
	char *user;
	char *pass;
} globals;

SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_url, globals.url);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_bindings, globals.bindings);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_realm, globals.realm);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_user, globals.user);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_pass, globals.pass);

struct config_data {
	char *name;
	int fd;
};

static size_t file_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
	register unsigned int realsize = (unsigned int)(size * nmemb);
	struct config_data *config_data = data;

	write(config_data->fd, ptr, realsize);
	return realsize;
}


static switch_xml_t xml_url_fetch(char *section,
								  char *tag_name,
								  char *key_name,
								  char *key_value,
								  char *params)
{
	char filename[1024] = "";
	CURL *curl_handle = NULL;
	struct config_data config_data;
	switch_xml_t xml = NULL;
    char *data = NULL;

    if (!(data = switch_mprintf("section=%s&tag_name=%s&key_name=%s&key_value=%s%s%s\n", 
                                section,
                                tag_name ? tag_name : "",
                                key_name ? key_name : "",
                                key_value ? key_value : "",
                                params ? "&" : "", params ? params : ""))) {

        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Memory Error!\n");
        return NULL;
    }

	srand((unsigned int)(time(NULL) + strlen(globals.url)));
	snprintf(filename, sizeof(filename), "%s%04x.tmp", SWITCH_GLOBAL_dirs.temp_dir, (rand() & 0xffff));
	curl_handle = curl_easy_init();
	if (!strncasecmp(globals.url, "https", 5)) {
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
	}
		
	config_data.name = filename;
	if ((config_data.fd = open(filename, O_CREAT | O_RDWR | O_TRUNC)) > -1) {
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl_handle, CURLOPT_URL, globals.url);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, file_callback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&config_data);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "freeswitch-xml/1.0");
		curl_easy_perform(curl_handle);
		curl_easy_cleanup(curl_handle);
		close(config_data.fd);
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error!\n");
	}

    switch_safe_free(data);

	if (!(xml = switch_xml_parse_file(filename))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Parsing Result!\n");
	}

	unlink(filename);
	
	return xml;
}



static switch_loadable_module_interface_t xml_rpc_module_interface = {
	/*.module_name */ modname,
	/*.endpoint_interface */ NULL,
	/*.timer_interface */ NULL,
	/*.dialplan_interface */ NULL,
	/*.codec_interface */ NULL,
	/*.application_interface */ NULL,
	/*.api_interface */ NULL,
	/*.file_interface */ NULL,
	/*.speech_interface */ NULL,
	/*.directory_interface */ NULL
};

static switch_status_t do_config(void) 
{
	char *cf = "xml_rpc.conf";
	switch_xml_t cfg, xml, settings, param;
	char *realm, *user, *pass;

	realm = user = pass = NULL;
	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	if ((settings = switch_xml_child(cfg, "settings"))) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");

			if (!strcasecmp(var, "auth-realm")) {
				realm = val;
			} else if (!strcasecmp(var, "auth-user")) {
				user = val;
			} else if (!strcasecmp(var, "auth-pass")) {
				pass = val;
			} else if (!strcasecmp(var, "gateway-url")) {
				char *bindings = (char *) switch_xml_attr_soft(param, "bindings");
				set_global_bindings(bindings);
				set_global_url(val);
			} else if (!strcasecmp(var, "http-port")) {
				globals.port = (uint16_t)atoi(val);
			}
		}
	}
	
	if (!globals.port) {
		globals.port = 8080;
	}
	if (user && pass && realm) {
		set_global_realm(realm);
		set_global_user(user);
		set_global_pass(pass);
	}
	switch_xml_free(xml);

	return globals.url ? SWITCH_STATUS_SUCCESS : SWITCH_STATUS_FALSE;
}


SWITCH_MOD_DECLARE(switch_status_t) switch_module_load(const switch_loadable_module_interface_t **module_interface, char *filename)
{
	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = &xml_rpc_module_interface;

	memset(&globals, 0, sizeof(globals));

	if (do_config() == SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding XML Fetch Function [%s] [%s]\n", globals.url, globals.bindings ? globals.bindings : "all");
		switch_xml_bind_search_function(xml_url_fetch, switch_xml_parse_section_string(globals.bindings));
	}

	curl_global_init(CURL_GLOBAL_ALL);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}


static switch_status_t http_stream_write(switch_stream_handle_t *handle, char *fmt, ...)
{
	va_list ap;
	TSession *r = handle->data;
	int ret = 0;
	char *data;

	va_start(ap, fmt);
#ifdef HAVE_VASPRINTF
	ret = vasprintf(&data, fmt, ap);
#else
	if ((data = (char *) malloc(2048))) {
		vsnprintf(data, 2048, fmt, ap);
	}
#endif
	va_end(ap);
	
	if (data) {
		ret = 0;
		HTTPWrite(r, data, (uint32_t)strlen(data));
		free(data);
	}
	
	return ret ? SWITCH_STATUS_FALSE : SWITCH_STATUS_SUCCESS;
}


abyss_bool HandleHook(TSession *r)
{
    char *m = "text/html";
	switch_stream_handle_t stream = {0};
	char *command;

	stream.data = r;
	stream.write_function = http_stream_write;

	if (globals.realm) {
		if (!RequestAuth(r,globals.realm, globals.user, globals.pass)) {
			return TRUE;
		}
	}



	if(strncmp(r->uri, "/api/", 5)) {
		return FALSE;
	}

	if (switch_event_create(&stream.event, SWITCH_EVENT_API) == SWITCH_STATUS_SUCCESS) {
		if (r->uri) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-URI", r->uri);
		if (r->query) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-QUERY", r->query);
		if (r->host) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-HOST", r->host);
		if (r->from) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-FROM", r->from);
		if (r->useragent) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-USER-AGENT", r->useragent);
		if (r->referer) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-REFERER", r->referer);
		if (r->requestline) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-REQUESTLINE", r->requestline);
		if (r->user) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-USER", r->user);
		if (r->port) switch_event_add_header(stream.event, SWITCH_STACK_BOTTOM, "HTTP-PORT", "%u", r->port);
	}

	command = r->uri + 5;
	
	ResponseChunked(r);
	ResponseStatus(r,200);
	ResponseContentType(r, m);
    ResponseWrite(r);
	switch_api_execute(command, r->query, NULL, &stream);
	HTTPWriteEnd(r);
    return TRUE;
}

#define CMDLEN 1024 * 256
static xmlrpc_value *freeswitch_api(xmlrpc_env *const envP, xmlrpc_value *const paramArrayP, void *const userData) 
{
	char *command, *arg;
	char *retbuf = malloc(CMDLEN);
	switch_stream_handle_t stream = {0};
	xmlrpc_value *val;

    /* Parse our argument array. */
    xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &command, &arg);
    if (envP->fault_occurred) {
        return NULL;
	}

	memset(retbuf, 0, CMDLEN);
	stream.data = retbuf;
	stream.end = stream.data;
	stream.data_size = CMDLEN;
	stream.write_function = switch_console_stream_write;
	switch_api_execute(command, arg, NULL, &stream);

    /* Return our result. */
    val = xmlrpc_build_value(envP, "s", retbuf);
	free(retbuf);

	return val;
}

SWITCH_MOD_DECLARE(switch_status_t) switch_module_runtime(void) 
{
    TServer abyssServer;
    xmlrpc_registry * registryP;
    xmlrpc_env env;
	char logfile[512];

	globals.running = 1;
    
    xmlrpc_env_init(&env);

    registryP = xmlrpc_registry_new(&env);
	
    xmlrpc_registry_add_method(&env, registryP, NULL, "freeswitch.api", &freeswitch_api, NULL);

    MIMETypeInit();
	MIMETypeAdd("text/html", "html");

	snprintf(logfile, sizeof(logfile), "%s%s%s", SWITCH_GLOBAL_dirs.log_dir, SWITCH_PATH_SEPARATOR, "freeswitch_http.log");
    ServerCreate(&abyssServer, "XmlRpcServer", globals.port, SWITCH_GLOBAL_dirs.htdocs_dir, logfile);
    
    xmlrpc_server_abyss_set_handler(&env, &abyssServer, "/RPC2", registryP);

    ServerInit(&abyssServer);
	ServerAddHandler(&abyssServer, HandleHook);
	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Starting HTTP Port %d, DocRoot [%s]\n", globals.port, SWITCH_GLOBAL_dirs.htdocs_dir);
    while (globals.running) {
        ServerRunOnce2(&abyssServer, ABYSS_FOREGROUND);
    }


	return SWITCH_STATUS_SUCCESS;
}



SWITCH_MOD_DECLARE(switch_status_t) switch_module_shutdown(void)
{
	globals.running = 0;
	curl_global_cleanup();
	return SWITCH_STATUS_SUCCESS;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:nil
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 expandtab:
 */
