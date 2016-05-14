#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdbool.h>
#include "util/list.h"

struct account_config_extra {
    char *key, *value;
};

struct account_config {
    char *name;
    char *source;
    list_t *extras;
};

struct aerc_config {
    struct {
        char *index_format;
        char *timestamp_format;
        bool show_all_headers;
    } ui;
    list_t *accounts;
};

extern struct aerc_config *config;

bool load_main_config(const char *file);
bool load_accounts_config();
void free_config(struct aerc_config *config);

#endif