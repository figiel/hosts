CFLAGS+=-Wall

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

all: test libuserhosts.so

libuserhosts.so: userhosts.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -D_GNU_SOURCE -fPIC -shared userhosts.c -o libuserhosts.so -ldl

test_getaddrinfo: test_getaddrinfo.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -g test_getaddrinfo.c -o test_getaddrinfo

.hosts: test_hosts
	cp test_hosts .hosts

test: test_getaddrinfo libuserhosts.so .hosts
	LD_PRELOAD=${PWD}/libuserhosts.so HOSTS_FILE=test_hosts ./test_getaddrinfo
	LD_PRELOAD=${PWD}/libuserhosts.so HOME=${PWD} ./test_getaddrinfo

install: libuserhosts.so
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 libuserhosts.so $(DESTDIR)$(PREFIX)/lib/

clean:
	rm -f libuserhosts.so test_getaddrinfo .hosts
