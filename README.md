#The CBUS Communication Daemon 0.1

This is the reference implementation of the CBUS communication protocol for doing
DBUS-like interprocess communication.

##Installing

cbusd and libcbus need a POSIX environment to work.

Compiling and installing:
```sh
    make 
    make install
```
Cleaning:
```sh
    make clean
```

##Documentation

The documentation is located in the doc folder, API.md covers the reference Implementation,
cbus.md the protocol

Examples are located in libcbus/test

##TODO

- More data types, at least an array type, maybe a byte type

##Changelog
- 0.1 "Daemon Days" : Initial release

##License

The source code, the documentation and everything else in this repository is given
to you under the terms of the MIT License
