#ifndef _IMAP_H
#define _IMAP_H

// RFC 3501

#include <stdbool.h>
#include <poll.h>
#include <stdarg.h>
#include "util/hashtable.h"
#include "urlparse.h"
#include "absocket.h"

enum recv_mode {
    RECV_WAIT,
    RECV_LINE,
    RECV_BULK
};

struct imap_capabilities {
    bool imap4rev1;
    bool starttls;
    bool logindisabled;
    bool auth_plain;
    bool auth_login;
    bool idle;
};

enum imap_status {
    STATUS_OK, STATUS_NO, STATUS_BAD, STATUS_PREAUTH, STATUS_BYE
};

struct imap_connection;

typedef void (*imap_callback_t)(struct imap_connection *imap,
        void *data, enum imap_status status, const char *args);

struct imap_state {
    char *selected_mailbox;
};

struct imap_connection {
    bool logged_in;
    absocket_t *socket;
    enum recv_mode mode;
    char *line;
    int line_index, line_size;
    struct pollfd poll[1];
    int next_tag;
    hashtable_t *pending;
    struct imap_capabilities *cap;
    struct imap_state *state;
    struct uri *uri;
};

struct imap_pending_callback {
    imap_callback_t callback;
    void *data;
};

bool imap_connect(struct imap_connection *imap, const char *host,
		const char *port, bool use_ssl, imap_callback_t callback, void *data);
void imap_receive(struct imap_connection *imap);
void imap_send(struct imap_connection *imap, imap_callback_t callback,
		void *data, const char *fmt, ...);
void imap_close(struct imap_connection *imap);
void handle_line(struct imap_connection *imap, const char *line);
struct imap_pending_callback *make_callback(imap_callback_t callback, void *data);

// Handlers
void init_status_handlers();
void handle_imap_status(struct imap_connection *imap, const char *token,
		const char *cmd, const char *args);
void handle_imap_capability(struct imap_connection *imap, const char *token,
		const char *cmd, const char *_args);
void handle_imap_list(struct imap_connection *imap, const char *token,
		const char *cmd, const char *_args);

// Do...ers?
void imap_list(struct imap_connection *imap, imap_callback_t callback,
		void *data, const char *refname, const char *boxname);
void imap_capability(struct imap_connection *imap, imap_callback_t callback,
        void *data);

#endif
