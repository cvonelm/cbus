all: cbusd libcbus doc
cbusd:
	make -C cbusd
libcbus:
	make -C libcbus
doc:
	make -C doc
install:
	make -C cbusd install
	make -C libcbus install
clean:
	make -C cbusd clean
	make -C libcbus clean
	make -C doc clean
.PHONY: all cbusd libcbus doc install clean
