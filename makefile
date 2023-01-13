all: pagingdemand

pagingdemand: pagingdemand.c
	gcc -Wall -Wextra -Wshadow pagingdemand.c -o pagingdemand

.PHONY: clean

clean:
	rm -rf pagingdemand
