CC=gcc
LD=gcc

CFLAGS=-W -Wall -g -DMAME_DEBUG=1
LDFLAGS=-d

d7810:	dasm.o 7810dasm.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o%.c:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o d7810
