nbtq: main.c to_snbt.c
	cc $^ -o $@ -Wall -Wextra -g -lz

.PHONY: clean
clean:
	rm nbtq
