/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */
#include "libcbus.h"
#include <string.h>
#include <stdlib.h>
char *cbus_construct_msg(uint32_t type, uint32_t serial, char *token, char *from, char *to,
        char *fn_name, char *args, ...)
{
    va_list a;
    va_start(a, args);
    char *result = v_cbus_construct_msg(type, serial, token, from, to, fn_name, args, a);
    va_end(a);
    return result;
}
char *v_cbus_construct_msg(uint32_t type, uint32_t serial, char *token, char *from, char *to,
        char *fn_name, char *args, va_list a)
{
    uint32_t size = sizeof(uint32_t) * 3; /* LENGTH|TYPE|SERIAL */
    
    size += strlen(token) + 1;
    size += strlen(from) + 1; 
    size += strlen(to) + 1;
    size += strlen(fn_name) + 1;
    size += strlen(args) + 1;
    
    va_list b;
    
    va_copy(b, a);
    char *args_it = args;
    for(;*args_it != 0;args_it++)
    {
        if(*args_it == 'i')
        {
            va_arg(a, uint32_t);
            size += sizeof(uint32_t);
        }
        else if(*args_it == 'd')
        {
            va_arg(a, double);
            size += sizeof(double);
        }
        else if(*args_it == 's')
        {
            char *s = va_arg(a, char *);
            size += strlen(s)  + 1;
        }
    }
    va_end(a);
    
    char *msg = malloc(size);
    char *msg_it = msg;

    /*SIZE */
    *(uint32_t *)msg_it = size;
    msg_it += sizeof(uint32_t);
    
    /*TYPE*/
    *(uint32_t *)msg_it = type;
    msg_it += sizeof(uint32_t);
    
    /*SERIAL*/
    *(uint32_t *)msg_it = serial;
    msg_it += sizeof(uint32_t);

    /*TOKEN */
    memcpy(msg_it, token, strlen(token) + 1);
    msg_it += strlen(token) + 1;

    memcpy(msg_it, from, strlen(from) + 1);
    msg_it += strlen(from) + 1;
    
    memcpy(msg_it, to, strlen(to) + 1);
    msg_it += strlen(to) + 1;
    
    memcpy(msg_it, fn_name, strlen(fn_name) + 1);
    msg_it += strlen(fn_name) + 1;
    
    memcpy(msg_it, args, strlen(args) + 1);
    msg_it += strlen(args) + 1;
    for(args_it = args;*args_it != 0; args_it++)
    {
        if(*args_it == 'i')
        {
            *(uint32_t *)msg_it = va_arg(b, uint32_t);
            msg_it += sizeof(uint32_t);
        }
        else if(*args_it == 'd')
        {
            *(double *)msg_it = va_arg(b, double);
            msg_it += sizeof(double);
        }
        else if(*args_it == 's')
        {
            char *s = va_arg(b, char *);
            memcpy(msg_it, s, strlen(s) + 1);
            msg_it += strlen(s) + 1;
        }
    }
    return msg;
}
char *cbus_construct_return( CBUS_msg *msg, char *args,va_list a)
{
    return v_cbus_construct_msg(CBUS_TYPE_FN_RETURN,
            msg->serial,
            msg->token,
            msg->to,
            msg->from,
            msg->fn_name,
            args, a);
}
char *cbus_construct_err(uint32_t serial, char *from, char *to, char * fn_name, int err, char * err_msg)
{
    return cbus_construct_msg(CBUS_TYPE_FN_ERR,
            serial,
            "",
            from,
            to,
            fn_name,
            "is", err, err_msg);
}
char *cbus_construct_signal(uint32_t serial, char *token, char *from, char *sig_name, char *args, va_list a)
{
    return v_cbus_construct_msg(CBUS_TYPE_SIGNAL,
            serial,
            token,
            from,
            "",
            sig_name,
            args, a);
}
