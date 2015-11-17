CC := /usr/bin/gcc
LD := /usr/bin/ld
CP := /bin/cp
SUDO := sudo

.PHONY: install
install: /lib64/faketime.so

/lib64/faketime.so: faketime.so
        $(SUDO) $(CP) -a $< $@

faketime.so: faketime.o
        $(LD) -shared -o $@ $< -ldl

faketime.o: faketime.c
        $(CC) -Wall -fPIC -DPIC -o $@ -c $<
