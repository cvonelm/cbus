/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */

#ifndef _LIBCBUS_COMMON_H_
#define _LIBCBUS_COMMON_H_
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
struct CBUS_conn
{
    char *address;
    int fd;
    struct CBUS_sub *subs;
    char *msg;
    size_t wanted_len;
    size_t cur_len;
    struct CBUS_conn *before;
    struct CBUS_conn *next;
    struct CBUS_msg *backlog;
    char *path;
};
struct CBUS_sub
{
    char *sender_name;
    char *signal_name;
    int serial;
    struct CBUS_sub *next;
};
struct CBUS_msg 
{
    /*proto part*/
    
    uint32_t length;
    uint32_t type;
    uint32_t serial;
    char *token;
    char *from;
    char *to;
    char *fn_name;
    char *arg_str;
    struct CBUS_arg *args;
    /*END: proto part*/

    char *msg;
    struct CBUS_msg *next;
    int err;
    char *errstr;

};
struct CBUS_arg
{
    union
    {
        int int_value;
        double double_value;
        char *str_value;
    };
    int type;
    struct CBUS_arg *next;
};
enum CBUS_msg_type
{
    CBUS_TYPE_FN_CALL=1,
    CBUS_TYPE_FN_RETURN=2,
    CBUS_TYPE_FN_ERR=3,
    CBUS_TYPE_SIGNAL=4
};
enum CBUS_arg_type
{
    CBUS_TYPE_INT=1,
    CBUS_TYPE_STRING=2,
    CBUS_TYPE_DOUBLE=3
};
enum CBUS_err
{
    CBUS_ERR_OTHER=-1,
    CBUS_ERR_CONNECTION=-2,
    CBUS_ERR_DISCONNECT=-3,
    CBUS_ERR_PARSE=-4,
    CBUS_ERR_NO_AUTH=-5,
    CBUS_ERR_NOT_FOUND=-6,
    CBUS_ERR_CONFLICT=-7,
    CBUS_ERR_NOMEM=-8,
    CBUS_ERR_UNKNOWN_FN=-9
};
enum CBUS_flag
{
    CBUS_FLAG_NO_BACKLOG=1
};
/* we pass no length here as it is assumed, that the underlying transport takes
 * care of the right msg length */

/* tools for parsing messages */
struct CBUS_msg *cbus_parse_msg(char *msg);

/* tools for constructing messages */

char *cbus_construct_msg(uint32_t type, uint32_t serial, char *token, char *from, char *to,
        char *fn_name, char *args, ...);
char *v_cbus_construct_msg(uint32_t type, uint32_t serial, char *token, char *from, char *to,
        char *fn_name, char *args, va_list a);
char *cbus_construct_return(struct CBUS_msg *msg, char *args, va_list a);
char *cbus_construct_err(uint32_t serial, char *from, char *to, char *fn_name, int err, char *err_msg);
char *cbus_construct_signal(uint32_t serial, char *token, char *from, char *sig_name, char *args, va_list a);
void cbus_free_msg(struct CBUS_msg *msg);

/* Helpers */
int fn_call_matches(struct CBUS_msg *msg, char *to, char *fn_name, char *args);
int fn_return_matches(struct CBUS_msg *msg, char *from, char *fn_name, char *args);
int signal_matches(struct CBUS_msg *msg, char *from, char *sig_name, char *args);
void cbus_print_msg(struct CBUS_msg *msg);
char *cbus_errstr(int err);


struct CBUS_conn *cbus_connect(char *address, int *err); 
void cbus_disconnect(struct CBUS_conn *conn);
struct CBUS_msg *cbus_read(struct CBUS_conn *conn, int *err, int flags); 
int cbus_call(struct CBUS_conn *conn, char *address, char *fn_name, char *args, ...);
int v_cbus_call(struct CBUS_conn *conn, char *address, char *fn_name, char *args, va_list a); 
int cbus_answer(struct CBUS_conn *conn, struct CBUS_msg *msg, char *args, ...); 
int cbus_subscribe(struct CBUS_conn *conn, char *sender, char *sig_name);
int cbus_emit(struct CBUS_conn *conn, char *sig_name, char *args, ...);

char *cbus_get_auth(struct CBUS_conn *conn, char *address, char *fn_name);
int cbus_check_auth(struct CBUS_conn *conn, struct CBUS_msg *msg);

struct CBUS_msg *cbus_response(struct CBUS_conn *conn, int *err, char *address,
        char *fn_anme, char *args, ...);
struct CBUS_msg *cbus_get_msg(struct CBUS_conn *conn, int *err);
int cbus_send_msg(struct CBUS_conn *conn, char *msg);
int cbus_send_return(struct CBUS_conn *conn, struct CBUS_msg *msg,
        char *args, ...);
int cbus_send_err(struct CBUS_conn *conn, struct CBUS_msg *msg, int err,
        char *err_msg);
int cbus_request_name(struct CBUS_conn *conn, char *name);
void cbus_reset_conn(struct CBUS_conn *conn);
int cbus_check_name(char *name);
#endif
