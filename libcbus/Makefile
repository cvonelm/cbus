all: auth construct helpers parse libcbus
	cc -shared -o libcbus.so libcbus.o libcbus-auth.o libcbus-construct.o libcbus-helpers.o \
		libcbus-parse.o -g3

auth:
	cc -c -fpic libcbus-auth.c -g3
construct:
	cc -c -fpic libcbus-construct.c -g3
helpers:
	cc -c -fpic libcbus-helpers.c -g3

parse:
	cc -c -fpic libcbus-parse.c -g3

libcbus:
	cc -c -fpic libcbus.c -g3
clean:
	rm -f *.o
	rm -f *.so
install:
	cp libcbus.so /usr/lib
	cp libcbus.h /usr/include
