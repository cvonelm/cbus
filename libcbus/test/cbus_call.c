/*  cbus_call.c - Call a function
 *  
 *  To test this program first start a cbus daemon at ~/.cbus
 *  and start cbus_response then call:
 *      make call
 *
 */

#include "../libcbus.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage %s [base dir]\n", argv[0]);
        return 1;
    }
    int err = 0;
    /*Connect to the CBUS daemon given in the arguments */
    struct CBUS_conn *conn = cbus_connect(argv[1], &err);
    if(conn == NULL)
    {
        fprintf(stderr, "Couldn't connect: %s\n", cbus_errstr(err));
        exit(1);
    }
    /*Call the function*/
    cbus_call(conn,  "/test2", "/test_function", "ss", "foo", "bar");
    /*Read a message from the bus*/
    struct CBUS_msg *msg = cbus_read(conn, &err, 0);
    if(err != 0)
    {
        fprintf(stderr, "Couldn't read:%s\n", cbus_errstr(err));
        return -1;
    }
    /* Print it*/
    cbus_print_msg(msg);

    /*Always free your message and disconnect */
    cbus_free_msg(msg);
    cbus_disconnect(conn);
    return 0;
}
