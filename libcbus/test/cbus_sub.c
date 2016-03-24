/*
 * cbus_sub.c - Catch a signal
 * Start a daemon at ~/.cbus, then execute
 *      make sub
 * and `make emit` to see the catched signal
 */
#include "../libcbus.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    int err;
    /*Connect to the daemon*/
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s [base dir]\n");
        return 1;
    }
     CBUS_conn *conn = cbus_connect(argv[1], &err);
    if(conn == NULL)
    {
        fprintf(stderr, "Couldn't connect: %s\n", cbus_errstr(err));
        return 1;
    }
    fprintf(stderr, "%s\n", conn->address);
    /*Subscribe to the signal */
    cbus_subscribe(conn, "/signal_emitter", "/connected");
    /*Read for messages, this will be most probably the signal we subscribed to*/
     CBUS_msg *msg = cbus_read(conn, &err, 0);
    if(msg == NULL)
    {
        fprintf(stderr, "Couldn't read: %s\n", cbus_errstr(err));
        return 1;
    }
    cbus_print_msg(msg);
    cbus_free_msg(msg);
    cbus_disconnect(conn);
}
