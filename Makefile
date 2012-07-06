CFLAGS+=-Wall

all: test libhostspriv.so

libhostspriv.so: hosts.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -D_GNU_SOURCE -fPIC -shared hosts.c -o libhostspriv.so -ldl 

test: hosts.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -g -DCOMPILE_FOR_TEST hosts.c -o test
	./test

clean:
	rm -f libhostspriv.so test
