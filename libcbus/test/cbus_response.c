#include "../libcbus.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    int err = 0;
    if(argc < 2)
    {
        fprintf(stderr, "Usage %s [base_dir]\n", argv[0]);
        exit(10);
    }
     CBUS_conn *conn = cbus_connect(argv[1], &err);
    if(conn == NULL)
    {
        fprintf(stderr, "Couldn't connect: %s\n", cbus_errstr(err));
    }
    err = cbus_request_name(conn, "/test2");
    if(err != 0)
    {
        fprintf(stderr, "Couldn't request name: %s\n", cbus_errstr(err));
        exit(1);
    }
    while(1)
    {
         CBUS_msg *msg = cbus_read(conn, &err, 0);
        if(err != 0)
        {
            fprintf(stderr, "Couldn't read: %s\n", cbus_errstr(err));
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

