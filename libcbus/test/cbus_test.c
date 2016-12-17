/**
 * cbus_test.c - the simplest possible libcbus example
 * 
 * start a cbus daemon at ~/.cbus
 * and call make test
 */
#include "../libcbus.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    int err = 0;
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s [base dir]\n", argv[0]);
        return 1;
    }
    /*Connect to the daemon*/
     CBUS_conn *conn = cbus_connect(argv[1], &err);
    if(err != 0)
    {
        fprintf(stderr, "Couldn't connect: %s\n", cbus_errstr(err));
        exit(1);
    }
    /*..and disconnect */
    cbus_disconnect(conn);
    return 0;
}
