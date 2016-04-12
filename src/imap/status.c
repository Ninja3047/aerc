#define _POSIX_C_SOURCE 201112LL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "imap/imap.h"
#include "util/stringop.h"
#include "util/hashtable.h"
#include "log.h"

void handle_resp_CAPABILITY(struct imap_connection *imap, list_t *args) {
	char *j = join_args((char **)args->items, args->length);
	worker_log(L_DEBUG, "Server capabilities: %s", j);
	free(j);
	// TODO: Do something with them?
}

hashtable_t *response_handlers;

void init_status_handlers() {
	if (!response_handlers) {
		response_handlers = create_hashtable(128, hash_string);
		hashtable_set(response_handlers, "CAPABILITY", handle_resp_CAPABILITY);
	}
}

void handle_imap_OK(struct imap_connection *imap, const char *token,
		const char *cmd, const char *args) {
	worker_log(L_DEBUG, "IMAP OK: %s", args);
	imap->events.ready(imap->events.data);
}

void handle_imap_status(struct imap_connection *imap, const char *token,
		const char *cmd, const char *args) {
	if (args[0] == '[' && strchr(args, ']')) {
		// Includes optional status response
		int i = strchr(args + 1, ']') - args - 1;
		char *status = malloc(i + 1);
		strncpy(status, args + 1, i);
		status[i] = '\0';
		args = args + 3 + i;
		list_t *split = split_string(status, " ");
		void (*handler)(struct imap_connection *imap, list_t *args) =
			hashtable_get(response_handlers, split->items[0]);
		if (handler) {
			handler(imap, split);
		}
		free_flat_list(split);
	}
	if (strcmp(token, "*") == 0) {
		if (strcmp(cmd, "OK") == 0) {
			handle_imap_OK(imap, token, cmd, args);
		} else {
			worker_log(L_DEBUG, "Got unhandled status command %s %s",
					cmd,args);
		}
	} else {
		imap_handler_t handler = hashtable_get(imap->pending, token);
		if (handler) {
			handler(imap, token, cmd, args);
		} else {
			worker_log(L_DEBUG, "Got unsolicited status command for %s", token);
		}
	}
}
