#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#ifdef COMPILE_FOR_TEST
#include <assert.h>

#define ASSERT_EQUAL(str1, str2) do { \
 assert(str1 != NULL); \
 assert(str2 != NULL); \
 assert(strcmp(str1, str2)==0); } while (0)
#define ASSERT assert
#endif /* COMPILE_FOR_TEST */

#define DELIMITERS " \t\n\r"

static char *find_alias_in_line(char *hosts_line, const char *alias)
{
	char *strtok_saveptr, *ret, *hash, *token = NULL;

	if (NULL != (hash = index(hosts_line, '#'))) {
		*hash = '\0';
	}

	token = strtok_r(hosts_line, DELIMITERS, &strtok_saveptr);

	if (token) {
		ret = token;
	} else {
		return NULL;
	}

	while (NULL != (token = strtok_r(NULL, DELIMITERS, &strtok_saveptr))) {
		if (strcmp(alias, token) == 0)
			return ret;	
	}

	return NULL;
}

static char *lookup_alias(const char *alias)
{
	char *ret = NULL;
	FILE *hosts_file = NULL;
	char *home_env = getenv("HOME");
	char *fname = "/.hosts";
	char *line = NULL;
	size_t line_len;

	if (!alias)
		return NULL;

	if (home_env) {
		/* thread safety paranoia as env variables are global */
		home_env = strdup(home_env);
	}

	if (home_env) {
		char *tmp;
		if (NULL != (tmp = realloc(home_env, strlen(home_env)+strlen(fname)+1))) {
			home_env = tmp;
			strcat(home_env, fname);
			fname = home_env;
		}
	}

	hosts_file = fopen(fname, "r");
	if (!hosts_file) {
		free(home_env); /* NULL or strdup or realloc */
		return NULL;
	}

	while (getline(&line, &line_len, hosts_file)>=0) {
		ret = find_alias_in_line(line, alias);
		if (ret) {
			ret = strdup(ret);
			break;
		}
	}
	free(line);
	fclose(hosts_file);
	free(home_env);
	return ret;

}

#ifndef COMPILE_FOR_TEST

/* actual wrappers, i know it's ugly and c++ was invented in the meanwhile ;)
 */

#define FUN_BEGIN(fun_name, ret_type, alias_arg_name, ...) \
ret_type fun_name(__VA_ARGS__) \
{\
	ret_type ret; \
	char *alias = NULL; \
	ret_type (* orig_fun)(__VA_ARGS__) = dlsym(RTLD_NEXT, #fun_name); \
	alias = lookup_alias(alias_arg_name); \
	if (alias) alias_arg_name = alias;

#define FUN_END(...) \
	ret = orig_fun(__VA_ARGS__); \
	if (alias) free(alias); \
	return ret; \
}

/* each wrapped function has its own FUN_BEGIN and FUN_END macro invocations
 * syntax is:
 * FUN_BEGIN:
 *  fun_name: name of the function which is to be wrapped
 *  ret_type: type of value returned
 *  name: name of the variable which will have its value looked up in hosts file
 *  ...: all arguments with their types, including one with name as above
 * FUN_END: 
 *  ...: all arguments without their types, keep the order.
 */

FUN_BEGIN(getaddrinfo, int, name, char *name, void *a, void *b, void *c);
FUN_END(name, a, b, c);

FUN_BEGIN(gethostbyname, void *, name, char *name);
FUN_END(name);

FUN_BEGIN(gethostbyname2, void *, name, char *name, int af);
FUN_END(name, af);

FUN_BEGIN(gethostbyname_r, int, name, char *name, void *a, void *b, size_t c, void *d, void *e);
FUN_END(name,a,b,c,d,e);

FUN_BEGIN(gethostbyname2_r, int, name, char *name, int a, void *b, void *c, size_t d, void *e, void *f);
FUN_END(name,a,b,c,d,e,f);

FUN_BEGIN(inet_aton, int, name, char *name, void *a);
FUN_END(name, a);


#else /* COMPILE_FOR_TEST */

static char *find_alias_test(char *hosts_line, const char *alias)
{
	static char writable_test_buffer[128];
	strncpy(writable_test_buffer, hosts_line, sizeof(writable_test_buffer)-1);
	return find_alias_in_line(writable_test_buffer, alias);
}

static char *lookup_alias_test(const char *alias)
{
	static char writable_test_buffer[128];
	char *tmp = lookup_alias(alias);
	
	if (tmp != NULL) {
		strncpy(writable_test_buffer, tmp, sizeof(writable_test_buffer)-1);
		free(tmp);
		tmp = writable_test_buffer;
	}
	return tmp;
}

static int test_hosts()
{
	FILE *hosts;

	char test_hosts_contents[] = "\
10.0.0.1 sometest\n\
10.0.0.2 sometest2 sometest3 sometest4\n\
10.0.0.3 sometesttest # sometest\n\
10.0.0.4 \x00 some_evilness \n\
10.0.0.5 shouldwork\n\
\n\
\n\
\n\
10.0.0.6 thistoo#end\n\
";

	ASSERT_EQUAL(find_alias_test("10.0.0.1 github.com sometest", "sometest"), "10.0.0.1");
	ASSERT(find_alias_test("#sometestsometest sometest sometest sometest", "sometest")==NULL);
	ASSERT(find_alias_test("#abbb bbb bb #aaabbb bbb, bbb", "bbb")==NULL);
	ASSERT_EQUAL(find_alias_test("10.0.0.1 github.com", "github.com"), "10.0.0.1");
	ASSERT(find_alias_test("10.0.0.1 sometest # github.com", "github.com")==NULL);
	ASSERT(find_alias_test("10.0.0.1 sometest#github.com", "github.com")==NULL);
	ASSERT_EQUAL(find_alias_test("10.0.0.1 sometest#github.com", "sometest"), "10.0.0.1");
	ASSERT(find_alias_test("", "sometest")==NULL);
	ASSERT(find_alias_test("", "")==NULL);

	ASSERT(setenv("HOME", getenv("PWD"), 1)==0);
	hosts = fopen(".hosts", "w");
	ASSERT(hosts != NULL);

	ASSERT(fwrite(test_hosts_contents, 1, sizeof(test_hosts_contents), hosts)
		==sizeof(test_hosts_contents));
	fclose(hosts);

	ASSERT(lookup_alias_test("xyz") == NULL);
	ASSERT_EQUAL(lookup_alias_test("sometest2"), "10.0.0.2");
	ASSERT_EQUAL(lookup_alias_test("sometest3"), "10.0.0.2");
	ASSERT_EQUAL(lookup_alias_test("sometest4"), "10.0.0.2");
	ASSERT_EQUAL(lookup_alias_test("sometesttest"), "10.0.0.3");
	ASSERT_EQUAL(lookup_alias_test("shouldwork"), "10.0.0.5");
	ASSERT(lookup_alias_test("some_evilness")==NULL);
	ASSERT_EQUAL(lookup_alias_test("thistoo"), "10.0.0.6");
	ASSERT(lookup_alias_test(NULL) == NULL);

	return 0;
}

int main() {
	return test_hosts();
}

#endif /* COMPILE_FOR_TEST */
