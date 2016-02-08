all: auth construct helpers parse libcbus
	cc -shared -o libcbus.so libcbus.o libcbus-auth.o libcbus-construct.o libcbus-helpers.o \
		libcbus-parse.o

auth:
	cc -c -Wall -Werror -fpic libcbus-auth.c
construct:
	cc -c -Wall -Werror  -fpic libcbus-construct.c
helpers:
	cc -c -Wall -Werror -fpic libcbus-helpers.c

parse:
	cc -c -Wall -Werror -fpic libcbus-parse.c

libcbus:
	cc -c -Wall -Werror -fpic libcbus.c
clean:
	rm -f *.o
	rm -f *.so
install:
	cp libcbus.so /usr/lib
	cp libcbus.h /usr/include
