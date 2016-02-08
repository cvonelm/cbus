/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */

#include "libcbus.h"
#include <stdarg.h>
#include <sys/un.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int cbus_request_name(struct CBUS_conn *conn, char *name)
{
    int err;
    struct CBUS_msg *msg = cbus_response(conn,&err, "/_daemon", "/request/name", "s", name);

    if(err)
    {
        return err;
    }
    if(msg->type == CBUS_TYPE_FN_ERR)
    {
        cbus_free_msg(msg);
        return msg->args->int_value;
    }
    conn->address = name;
    cbus_free_msg(msg);
    return 0;
}
struct CBUS_conn *cbus_connect(char *address, int *err)
{
    struct sockaddr_un remote;
    signal(SIGPIPE, SIG_IGN);
    struct CBUS_conn *conn = calloc(1, sizeof(struct CBUS_conn));
    conn->path = address;
    if((conn->fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        free(conn);
        *err = CBUS_ERR_CONNECTION;
        return NULL;
    }
    remote.sun_family = AF_UNIX;
    memcpy(remote.sun_path, address, strlen(address));
    memcpy(remote.sun_path + strlen(address), "/sock", strlen("/sock") + 1);
    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if(connect(conn->fd, (struct sockaddr *)&remote, len) == -1)
    {
        free(conn);
        *err = CBUS_ERR_CONNECTION;
        return NULL;
    }
    struct CBUS_msg *msg = cbus_read(conn, err ,0);
    if(fn_return_matches(msg, "/_daemon", "/name", "s"))
    {
        conn->address = msg->args->str_value;
    }
    else
    {
        *err = CBUS_ERR_CONNECTION;
        return NULL;
    }
    return conn;
}
int cbus_answer(struct CBUS_conn *conn, struct CBUS_msg *msg, char *args, ...)
{
    va_list a;
    va_start(a, args);
    char *answer_msg = cbus_construct_return(msg, args, a);
    if(answer_msg == NULL)
    {
        return CBUS_ERR_PARSE;
    }
    va_end(a);
    cbus_send_msg(conn, answer_msg);
    free(answer_msg);
    return 0;
}
struct CBUS_msg *cbus_read(struct CBUS_conn *conn, int *err, int flags)
{
    if(!(flags && CBUS_FLAG_NO_BACKLOG) && conn->backlog != NULL)
    {
        struct CBUS_msg *result = conn->backlog;
        conn->backlog = result->next;
        return result;
    }
    fd_set master;
    
    FD_SET(conn->fd, &master);
    for(;;)
    {
        if(select(conn->fd + 1, &master, NULL, NULL, NULL) == -1)
        {
            *err = CBUS_ERR_CONNECTION;
            return NULL;
        }
        struct CBUS_msg *msg =  cbus_get_msg(conn, err);
        if(msg != NULL)
        {
            return msg;
        }
    }

}
struct CBUS_msg *cbus_get_msg(struct CBUS_conn *conn, int *err)
{
    *err = 0;
    int read_size;
        ioctl(conn->fd, FIONREAD, &read_size);
        if(read_size == 0)
        {
            *err = CBUS_ERR_DISCONNECT;
            return NULL;
        }
        if(conn->msg == NULL)
        {
            conn->msg = malloc(sizeof(uint32_t));
            if(recv(conn->fd, conn->msg, sizeof(uint32_t), 0) == -1)
            {
                free(conn->msg);
                conn->msg = NULL;
                *err = CBUS_ERR_CONNECTION;
                return NULL;
            }
            conn->wanted_len = *(uint32_t*)conn->msg;
            conn->cur_len = sizeof(uint32_t);
            read_size -= sizeof(uint32_t);
            conn->msg = realloc(conn->msg, conn->wanted_len);
        }
        if(read_size + conn->cur_len >= conn->wanted_len)
        {
            recv(conn->fd, conn->msg + conn->cur_len, conn->wanted_len - conn->cur_len, 0);
            struct CBUS_msg *msg = cbus_parse_msg(conn->msg);
            cbus_reset_conn(conn);
            return msg;
        }
        else if(read_size + conn->cur_len < conn->wanted_len)
        {
            recv(conn->fd, conn->msg + conn->cur_len, read_size, 0);
            conn->cur_len += read_size;
        }
        return NULL;

}
void cbus_reset_conn(struct CBUS_conn *conn)
{
    conn->msg = NULL;
    conn->wanted_len = 0;
    conn->cur_len = 0;
}
int cbus_call(struct CBUS_conn *conn, char *address, char *fn_name, char *args, ...)
{
    va_list a;
    va_start(a, args);
    int ret = v_cbus_call(conn, address, fn_name, args,a );
    va_end(a);
    return ret;
}
int v_cbus_call(struct CBUS_conn *conn, char *address, char *fn_name, char *args, va_list a)
{
    uint32_t serial = rand();
    char *token = cbus_get_auth(conn, address, fn_name);
    if(token == NULL)
    {
        return CBUS_ERR_NO_AUTH;
    }
    char *msg = v_cbus_construct_msg(CBUS_TYPE_FN_CALL, serial, token, conn->address, address,
            fn_name, args, a);
    if(msg == NULL)
    {
        return CBUS_ERR_PARSE;
    }
    cbus_send_msg(conn, msg);
    free(msg);
    return serial;
}

int cbus_subscribe(struct CBUS_conn *conn, char *sender, char *sig_name)
{
    int err;
    struct CBUS_msg *msg = cbus_response(conn,
            &err,
            "/_daemon",
            "/subscribe",
            "ss", sender, sig_name);
    if(err)
    {
        return err;
    }
    if(msg->type == CBUS_TYPE_FN_ERR)
    {
        return msg->args->int_value;
    }
    return 0;
}
int cbus_emit(struct CBUS_conn *conn, char *sig_name, char *args, ...)
{
    va_list a;
    va_start(a, args);
    uint32_t serial = rand();
    char *token = cbus_get_auth(conn, conn->address, sig_name);
    if(token == NULL)
    {
        return CBUS_ERR_NO_AUTH;
    }
    char *msg = cbus_construct_signal(serial,
            "",
            conn->address,
            sig_name,
            args,
            a);
    if(msg == NULL)
    {
        return CBUS_ERR_PARSE;
    }
    va_end(a);
    int err = cbus_send_msg(conn, msg);
    if(err)
    {
        return err;
    }
    return 0;
}
struct CBUS_msg *cbus_response(struct CBUS_conn *conn, int *err,
        char *address, char *fn_name, char *args, ...)
{
    uint32_t serial;
    va_list a;
    va_start(a, args);
    serial = v_cbus_call(conn, address, fn_name, args, a);
    va_end(a);
    while(1)
    {
        struct CBUS_msg *msg = cbus_read(conn, err, CBUS_FLAG_NO_BACKLOG);
        if(*err != 0)
        {
            return NULL;
        }
        if(msg->serial == serial)
        {
            return msg;
        }
        else
        {
            if(conn->backlog == NULL)
            {
                conn->backlog = msg;
            }
            else
            {
                struct CBUS_msg *msg_end = conn->backlog;
                for(;msg_end->next != NULL; msg_end = msg_end->next);
                msg_end->next = msg;
            }
        }
    }


}
int cbus_send_msg(struct CBUS_conn *conn, char *msg)
{
    int err = 0;
        if((err = send(conn->fd, msg, *(uint32_t*)msg, 0)) < 0)
        {
            if(err == EPIPE)
            {
                return CBUS_ERR_DISCONNECT;
            }
            else
            {
                return CBUS_ERR_CONNECTION;
            }
        }
    return 0;
}
int cbus_send_err(struct CBUS_conn *conn, struct CBUS_msg *msg, int err, char *err_message)
{
    char *err_msg = cbus_construct_err(msg->serial, conn->address, msg->from, msg->fn_name, err, err_message);
    cbus_send_msg(conn, err_msg);
    free(err_msg);
    return 0;
}
int cbus_send_return(struct CBUS_conn *conn, struct CBUS_msg *msg, char *args, ...)
{
    va_list a;
    va_start(a, args);
    char *ret_msg = cbus_construct_return(msg, args, a);
    va_end(a);
    cbus_send_msg(conn, ret_msg);
    free(ret_msg);
    return 0;
}
void cbus_disconnect(struct CBUS_conn *conn)
{
    struct CBUS_msg *msg = conn->backlog;
    while(msg != NULL)
    {
        struct CBUS_msg *temp = msg->next;
        cbus_free_msg(msg);
        msg = temp;
    }
    close(conn->fd);
    free(conn);
}

void cbus_free_msg(struct CBUS_msg *msg)
{
    free(msg->msg);
    free(msg);
}
