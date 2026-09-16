CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra

.PHONY: all clean

all: exe

exe: algo_comp_v3.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f exe
