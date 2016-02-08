#include "../libcbus.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    int err = 0;
    struct CBUS_conn *conn = cbus_connect("/home/cve/.cbus", &err);
    if(conn == NULL)
    {
        fprintf(stderr, "Couldn't connect: %d\n", err);
    }
    if(cbus_request_name(conn, "/test2") < 0)
    {
        fprintf(stderr, "CBUS Error\n");
    }
    if(err != 0)
    {
        fprintf(stderr, "CBUS Error: %d\n", err);
        exit(1);
    }
    while(1)
    {
        struct CBUS_msg *msg = cbus_read(conn, &err, 0);
        if(err != 0)
        {
            fprintf(stderr, "CBUS Error: %d\n", err);
            return -1;
        }
        cbus_print_msg(msg);
        if(fn_call_matches(msg, "/test2", "/test_function", "ss"))
        {
            cbus_answer(conn, msg, "ss", "foo", "bar");
        }
    }
    cbus_disconnect(conn);
    return 0;
}
