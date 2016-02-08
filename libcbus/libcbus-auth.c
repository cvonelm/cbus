/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this repository
 *  for more information
 */

#include "libcbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *cbus_get_auth(struct CBUS_conn *conn, char *address, char *fn_name)
{
    char *rule_path = calloc(1, strlen(conn->path) +
            strlen("/auth") +
            strlen(address) + 
            strlen(fn_name) +
            1);
    char *rule_it = rule_path;
    
    memcpy(rule_it, conn->path, strlen(conn->path));
    rule_it += strlen(conn->path);
    memcpy(rule_it, "/auth", strlen("/auth"));
    rule_it += strlen("/auth");
    memcpy(rule_it, address, strlen(address));
    rule_it += strlen(address);
    memcpy(rule_it, fn_name, strlen(fn_name));
    FILE *rule_file = fopen(rule_path, "rb");
    if(rule_file == NULL)
    {
        if(errno == ENOENT)
        {
            char *str = malloc(1);
            *str = 0;
            return str;
        }
        else
        {
            return NULL;
        }
    }
    fseek(rule_file, 0, SEEK_END);
    size_t length = ftell(rule_file);
    fseek(rule_file, 0, SEEK_SET);
    char *result = malloc(length + 1);
    fread(result, length, 1, rule_file);
    result[length] = 0;
    return result;

}
int cbus_check_auth(struct CBUS_conn *conn, struct CBUS_msg *msg)
{
    if(msg->type == CBUS_TYPE_FN_ERR || msg->type == CBUS_TYPE_FN_RETURN)
    {
        return 1;
    }
    char *auth_str = cbus_get_auth(conn, msg->to, msg->fn_name);
    if(auth_str == NULL)
    {
        return 0;
    }
    if(strcmp(auth_str, msg->token) == 0)
    {
        return 1;
    }
    return 0;
}
