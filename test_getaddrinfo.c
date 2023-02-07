#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <assert.h>

#define ASSERT_EQUAL(str1, str2) \
	do { \
		assert(str1 != NULL); \
		assert(str2 != NULL); \
		assert(strcmp(str1, str2) == 0); \
	} while (0)
#define ASSERT assert

static const void *getaddr(const struct addrinfo *info)
{
	if (info->ai_family == AF_INET) {
		const struct sockaddr_in *addr =
			(struct sockaddr_in *)info->ai_addr;
		return &addr->sin_addr;
	}
	if (info->ai_family == AF_INET6) {
		const struct sockaddr_in6 *addr =
			(struct sockaddr_in6 *)info->ai_addr;
		return &addr->sin6_addr;
	}
	return NULL;
}

void resolve(const char *hostname, char *out)
{
	struct addrinfo *result;
	struct addrinfo hints = { .ai_family = AF_UNSPEC,
				  .ai_socktype = SOCK_DGRAM };

	if (getaddrinfo(hostname, NULL, &hints, &result)) {
		return;
	}
	if (result == NULL) {
		return;
	}

	inet_ntop(result->ai_family, getaddr(result), out, INET6_ADDRSTRLEN);

	freeaddrinfo(result);
}

void assert_resolve(const char *input, const char *expected_output)
{
	char result[INET6_ADDRSTRLEN] = "";
	resolve(input, result);
	assert(strcmp(result, expected_output) == 0);
}

int main()
{
	assert_resolve("sometest", "10.0.0.1");
	assert_resolve("sometest2", "10.0.0.2");
	assert_resolve("sometest3", "10.0.0.2");
	assert_resolve("sometest4", "10.0.0.2");

	assert_resolve("sometesttest", "10.0.0.3");
	assert_resolve("some_evilness", "");
	assert_resolve("shouldwork", "10.0.0.5");
	assert_resolve("thistoo", "10.0.0.6");
	assert_resolve("unknown", "");

	char resolved_localhost[INET6_ADDRSTRLEN] = "";
	resolve("localhost", resolved_localhost);
	assert_resolve("localhostalias", resolved_localhost);

	assert_resolve("someipv6test1", "::1");
	assert_resolve("someipv6test2", "::ffff:10.0.0.7");
	assert_resolve("someipv6test3", "::ffff:10.0.0.8");

	// Name resolution is not case sensitive
	assert_resolve("someTest", "10.0.0.1");
	assert_resolve("casedoesntmatter", "10.0.0.9");

	return 0;
}
