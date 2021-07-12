#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

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
	const char *hosts_file_env = getenv("HOSTS_FILE");
	char *line = NULL;
	size_t line_len;

	if (!alias)
		return NULL;

	if (hosts_file_env) {
		hosts_file = fopen(hosts_file_env, "r");
	} else {
		const char *relative_host_file_path = "/.hosts";
		const char *home_env = getenv("HOME");
		if (!home_env)
			return NULL;

		char *hosts_file_path = malloc(strlen(home_env)+strlen(relative_host_file_path)+1);
		if (!hosts_file_path) {
			return NULL;
		}
		strcpy(hosts_file_path, home_env);
		strcat(hosts_file_path, relative_host_file_path);

		hosts_file = fopen(hosts_file_path, "r");

		free(hosts_file_path);
	}

	if (!hosts_file) {
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
	return ret;

}

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
