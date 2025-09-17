BIN = nbtq
OBJS = main.o to_snbt.o

CC ?= cc
CFLAGS ?= -Wall -Wextra
PREFIX ?= /usr

$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) -lz

main.o: main.c common.h to_snbt.h
to_snbt.o: to_snbt.c common.h to_snbt.h

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m755 $(BIN) $(DESTDIR)$(PREFIX)/bin/

clean:
	rm $(BIN) $(OBJS)
