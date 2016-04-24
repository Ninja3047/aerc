#define _POSIX_C_SOURCE 201112LL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "internal/imap.h"
#include "util/stringop.h"
#include "util/list.h"
#include "log.h"

void imap_capability(struct imap_connection *imap, imap_callback_t callback,
		void *data) {
	imap_send(imap, callback, data, "CAPABILITY");
}

void handle_imap_capability(struct imap_connection *imap, const char *token,
		const char *cmd, imap_arg_t *args) {
	struct imap_capabilities *cap = calloc(1,
			sizeof(struct imap_capabilities));

	struct { const char *name; bool *ptr; } ptrs[] = {
		{ "IMAP4rev1", &cap->imap4rev1 },
		{ "STARTTLS", &cap->starttls },
		{ "LOGINDISABLED", &cap->logindisabled },
		{ "AUTH=PLAIN", &cap->auth_plain },
		{ "AUTH=LOGIN", &cap->auth_login },
		{ "IDLE", &cap->idle }
	};

	while (args) {
		if (args->type == IMAP_STRING || args->type == IMAP_ATOM) {
			for (size_t j = 0; j < sizeof(ptrs) / (sizeof(void*) * 2); ++j) {
				if (strcmp(ptrs[j].name, args->str) == 0) {
					*ptrs[j].ptr = true;
				}
			}
		}
		args = args->next;
	}

	free(imap->cap);
	imap->cap = cap;
}
