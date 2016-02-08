/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */
#include "libcbus.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> 
#include <stdio.h> 

struct CBUS_msg *cbus_parse_msg(char *msg)
{
    struct CBUS_msg *result = calloc(1, sizeof(struct CBUS_msg));
    result->msg = msg;
    char *msg_it = msg;
    
    /*LENGTH*/

    result->length = *(uint32_t *)msg;
    char *msg_end = msg + result->length; 
    /* this is the absolute minimum, assuming all char *fields are just one char of NULL */
    if(result->length < sizeof(uint32_t) * 3 + 5)
    {
        free(result);
        return NULL;
    }
    msg_it += sizeof(uint32_t);
    
    /*TYPE*/
    result->type = *(uint32_t *)msg_it;
    msg_it += sizeof(uint32_t);
    
    /*SERIAL*/
    result->serial = *(uint32_t *)msg_it;
    msg_it += sizeof(uint32_t);
    
    /*TOKEN*/
    result->token = msg_it;
    if(strnlen(result->token, msg_end - msg_it) == msg_end - msg_it)
    {
        free(result);
        return NULL;
    }
    /* it is safe to us strlen here as we proved that result->token is null-terminated above */
    msg_it += strlen(msg_it) + 1;
    
    /*FROM*/
    result->from = msg_it;
    if(strnlen(result->from, msg_end - msg_it) == msg_end - msg_it)
    {
        free(result);
        return NULL;
    }
    msg_it += strlen(result->from) + 1;
    
    /* TO */
    result->to = msg_it;
    if(strnlen(result->to, msg_end - msg_it) == msg_end - msg_it)
    {
        free(result);
        return NULL;
    }
    msg_it += strlen(result->to) + 1;
   
    /* FN_NAME */

    result->fn_name = msg_it;
    if(strnlen(result->fn_name, msg_end - msg_it) == msg_end - msg_it)
    {
        free(result);
        return NULL;
    }
    msg_it += strlen(result->fn_name) + 1;
    /*ARG_STR */
    result->arg_str = msg_it;
    if(strnlen(result->arg_str, msg_end - msg_it) == msg_end - msg_it)
    {
        
        free(result);
        return NULL;
    }
    msg_it += strlen(result->arg_str) + 1;
    char *arg_str_it;
    struct CBUS_arg *arg_it;
    for(arg_str_it = result->arg_str; *arg_str_it != 0;arg_str_it++)
    {
        if(*arg_str_it == 'i')
        {
            if(msg_it + sizeof(uint32_t) <= msg_end)
            {
                if(result->args == NULL)
                {
                    result->args = malloc(sizeof(struct CBUS_arg));
                    arg_it = result->args;
                }
                else
                {
                    arg_it->next = malloc(sizeof(struct CBUS_arg));
                    arg_it = arg_it->next;
                }
                arg_it->int_value = *(uint32_t *)msg_it;
                arg_it->type = CBUS_TYPE_INT;
                msg_it += sizeof(uint32_t);
            }
            else
            {
                free(result);
                return NULL;
            }
        }
        else if(*arg_str_it == 'd')
        {
            if(msg_it + sizeof(double) <= msg_end)
            {
                if(result->args == NULL)
                {
                    result->args = malloc(sizeof(struct CBUS_arg));
                    arg_it = result->args;
                }
                else
                {
                    arg_it->next = malloc(sizeof(struct CBUS_arg));
                    arg_it = arg_it->next;
                }
                arg_it->double_value = *(double *)msg_it;
                msg_it += sizeof(double);
                arg_it->type = CBUS_TYPE_DOUBLE;
            }
            else
            {
                free(result);
                return NULL;
            }
        }
        else if(*arg_str_it == 's')
        {
            if(strnlen(msg_it, msg_end-msg_it) !=  msg_end - msg_it)
            {
                if(result->args == NULL)
                {
                    result->args = malloc(sizeof(struct CBUS_arg));
                    arg_it = result->args;
                }
                else
                {
                    arg_it->next = malloc(sizeof(struct CBUS_arg));
                    arg_it = arg_it->next;
                }
                arg_it->str_value = msg_it;
                msg_it += strlen(arg_it->str_value) + 1;
                arg_it->type = CBUS_TYPE_STRING;
            }
            else
            {

                free(result);
                return NULL;
            }
        }
    }
    return result;
}