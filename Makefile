CFLAGS+=-Wall

all: test libhostspriv.so

libhostspriv.so: hosts.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -D_GNU_SOURCE -fPIC -shared hosts.c -o libhostspriv.so -ldl

test_getaddrinfo: test_getaddrinfo.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -g test_getaddrinfo.c -o test_getaddrinfo

.hosts: test_hosts
	cp test_hosts .hosts

test: test_getaddrinfo libhostspriv.so .hosts
	LD_PRELOAD=${PWD}/libhostspriv.so HOSTS_FILE=test_hosts ./test_getaddrinfo
	LD_PRELOAD=${PWD}/libhostspriv.so HOME=${PWD} ./test_getaddrinfo

clean:
	rm -f libhostspriv.so test_getaddrinfo .hosts
