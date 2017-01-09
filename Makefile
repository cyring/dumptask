obj-m := dumptask.o
KVERSION = $(shell uname -r)
DESTDIR = $(HOME)

all:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
