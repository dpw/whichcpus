CFLAGS=-Wall -Wextra -Werror -g -D_GNU_SOURCE

whichcpus: whichcpus.o
	gcc -o $@ $^ -lpthread

.PHONY: clean
clean::
	rm -f *.o *~ whichcpus
