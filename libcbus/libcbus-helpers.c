/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */

#include "libcbus.h"
#include <string.h>
#include <stdio.h>

int fn_call_matches( CBUS_msg *msg,char *to, char *fn_name, char *args)
{
    if(msg->type != CBUS_TYPE_FN_CALL)
    {
        return 0;
    }
    if(*to != 0)
    {

        if(strcmp(msg->to, to) != 0)
        {
            return 0;
        }
    }
    if(*fn_name != 0)
    {
        if(strcmp(msg->fn_name, fn_name) != 0)
        {
            return 0;
        }
    }
    if(*args != 0)
    {
        if(strcmp(msg->arg_str, args) != 0)
        {
            return 0;
        }
    }
    return 1;
}
int fn_return_matches( CBUS_msg *msg,char *from, char *fn_name, char *args)
{
    if(msg->type != CBUS_TYPE_FN_RETURN)
    {
        return 0;
    }
    if(*from != 0)
    {

        if(strcmp(msg->from, from) != 0)
        {
            return 0;
        }
    }
    if(*fn_name != 0)
    {
        if(strcmp(msg->fn_name, fn_name) != 0)
        {
            return 0;
        }
    }
    if(*args != 0)
    {
        if(strcmp(msg->arg_str, args) != 0)
        {
            return 0;
        }
    }
    return 1;
}
int signal_matches( CBUS_msg *msg, char *from,  char *sig_name, char *args)
{
    if(msg->type != CBUS_TYPE_SIGNAL)
    {
        return 0;
    }
    if(*from != 0) 
    {
        if(strcmp(msg->from, from) != 0)
        {
        return 0;
    }
    }
    if(*sig_name != 0)
    {

    if(strcmp(msg->fn_name, sig_name) != 0)
    {
        return 0;
    }
    }
    if(*args != 0)
    {
    
    if(strcmp(msg->arg_str, args) != 0)
    {
        return 0;
    }
    }
    return 1;
}

void cbus_print_msg( CBUS_msg *msg)
{
    fprintf(stderr, "msg->length=%d\n", msg->length);
    if(msg->type == CBUS_TYPE_FN_CALL)
    {
        fprintf(stderr, "msg->type=CALL\n");
    }
    else if(msg->type == CBUS_TYPE_FN_RETURN)
    {

        fprintf(stderr, "msg->type=RETURN\n");
    }
    else if(msg->type == CBUS_TYPE_FN_ERR)
    {
        fprintf(stderr, "msg->type=ERROR\n");
    }
    else if(msg->type == CBUS_TYPE_SIGNAL)
    {
        fprintf(stderr, "msg->type=SIGNAL\n");
    }
    fprintf(stderr, "msg->serial=%d\n", msg->serial);
    fprintf(stderr, "msg->token=%s\n", msg->token);
    fprintf(stderr, "msg->from=%s\n", msg->from);
    fprintf(stderr, "msg->to=%s\n", msg->to);
    fprintf(stderr, "msg->fn_name=%s\n", msg->fn_name);
    if(msg->type == CBUS_TYPE_FN_ERR)
    {
        fprintf(stderr, "Error(%d): %s\n", msg->args->int_value,
                msg->args->next->str_value);
    }
    else
    {
        fprintf(stderr, "ARGS(\"%s\"):\n", msg->arg_str);
         CBUS_arg *arg = msg->args;
        for(;arg != NULL;arg = arg->next)
        {
            if(arg->type == CBUS_TYPE_STRING)
            {
                fprintf(stderr, "\t%s [string]\n", arg->str_value);
            }
            else if(arg->type == CBUS_TYPE_INT)
            {
                fprintf(stderr, "\t%d [uint32_t]\n", arg->int_value);
            }
            else if(arg->type == CBUS_TYPE_DOUBLE)
            {
                fprintf(stderr, "\t%f [double]\n", arg->double_value);
            }
        }
    }
}
int cbus_check_name(char *name)
{
    if(strstr(name, "/_"))
    {
        if(strncmp(name, "/_unnamed/", strlen("/_unnamed/")))
        {
            return 1;
        }
        return 0;
    }
    return 1;
}

char *cbus_errstr(int err)
{
    if(err == CBUS_ERR_CONNECTION)
    {
        return "The connection to the cbus daemon failed";
    }
    else if(err == CBUS_ERR_DISCONNECT)
    {
        return "The connection to the cbus daemon was closed";
    }
    else if(err == CBUS_ERR_NO_AUTH)
    {
        return "Permission denied";
    }
    else if(err == CBUS_ERR_NOT_FOUND)
    {
        return "The requested name was not found";
    }
    else if(err == CBUS_ERR_CONFLICT)
    {
        return "The requested address already exists";
    }
    else if(err == CBUS_ERR_NOMEM)
    {
        return "Out of memory";
    }
    else if(err == CBUS_ERR_UNKNOWN_FN)
    {
        return "This function is not defined";
    }
    else if(err == CBUS_ERR_NAME)
    {
        return "Invalid name";
    }
    else
    {
        return "Unknown Error";
    }
}
