/*  Copyright (c) 2016 Christian von Elm <c.vonelm@gmail.com>
 *
 *  This file is open source, see the LICENSE file in the top directory of this 
 *  repository for more information
 */
#ifndef _CBUSD_H_
#define _CBUSD_H_
#include <stdint.h>
#include "../libcbus/libcbus.h"
 CBUS_conn *cbusd_get_conn_by_address(char *address);
int cbusd_process( CBUS_conn *sender,  CBUS_msg *msg);

int cbusd_clear_dir(char *path);
void cbusd_disconnect( CBUS_conn *conn);
void parse_args(int argc, char **argv);
int cbusd_handle_request( CBUS_conn *sender,  CBUS_msg *msg);
void print_usage();
char *str_copy(char *str);
int cbusd_check_name(char *name);
void cbusd_rand_str(char *str, int len);
int cbusd_gen_auth(char *source, char *dest);
enum CBUS_proto_err
{
    CBUS_PROTO_NOT_FOUND=-1
};
#endif
