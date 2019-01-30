# showtask, dumptask
# Copyright (C) 2017-2019 CYRIL INGENIERIE
# Licenses: GPL2

obj-m := dumptask.o
KVERSION = $(shell uname -r)
DESTDIR = $(HOME)

all: showtask
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
	rm showtask

showtask: showtask.o
	cc showtask.c -o showtask
showtask.o: showtask.c
	cc -Wall -c showtask.c -o showtask.o
