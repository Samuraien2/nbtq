CC ?= cc

nbtq: main.c to_snbt.c
	$(CC) $^ -o $@ -Wall -Wextra -g -lz

.PHONY: clean
clean:
	rm nbtq
