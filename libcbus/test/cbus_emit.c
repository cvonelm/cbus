/*
 * cbus_emit.c - emit a signal
 * First start a daemon on ~/.cbus, then call
 *     make sub
 * and then `make emit`
 */
#include "../libcbus.h"
#include <stdio.h>
int main(int argc, char **argv)
{
    int err;
    /*connect to the daemon*/
     CBUS_conn *conn = cbus_connect("/home/cve/.cbus", &err);
    if(conn == NULL)
    {
        fprintf(stderr, "Couldn't connect: %s\n", cbus_errstr(err));
        return 1;
    }
    /*Request the name. This is integral as the cbus_subs are listening for this name*/
    if((err = cbus_request_name(conn, "/signal_emitter")) < 0)
    {
        fprintf(stderr, "Couldn't request name: %s\n", cbus_errstr(err));
        return 1;
    }
    /*Emit the signal*/
    if((err = cbus_emit(conn, "/connected", "")) < 0)
    {
        fprintf(stderr, "Couldn't emit signal: %s\n", cbus_errstr(err));
        return 1;
    }
    /*and disconnect*/
    cbus_disconnect(conn);
}
