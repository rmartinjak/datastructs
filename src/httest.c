#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

#if _SVID_SOURCE || _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED ||  _POSIX_C_SOURCE >= 200809L
#else
char *strdup(const char *s)
{
    char *r;
    size_t n = strlen(s)+1;

    r = malloc(n);

    if (r)
        memcpy(r, s, n);

    return r;
}
#endif


hash_t my_hash(const void *key, const void *arg);
int my_cmp(const void *key1, const void *key2, const void *arg);

hash_t my_hash(const void *key, const void *arg) {
	hash_t hash = 5381;
	const char *str = (const char*)key;
	size_t n;
	int c;

	if (!arg) {
		while ((c = *str++))
			hash = ((hash << 5) + hash) + c;
	}
	else {
		n = *(size_t*)arg;
		while ((c = *str++) && n--)
			hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

int my_cmp(const void *key1, const void *key2, const void *arg) {
	size_t n;

	if (!arg) {
		return strcmp((const char*)key1, (const char*)key2);
	}
	else {
		n = *(size_t*)arg;
		return strncmp((const char*)key1, (const char*)key2, n);
	}
}

#define HELP "insert: \"i <key> <data>\n" \
				"set: \"s <key> <data>\n" \
				"get: \"g <key>\n" \
				"remove: \"r <key>\n"
#define PROMPT "> "

int main(int argc, char **argv) {
	hashtable *ht;
	htiter *it;

	FILE *f;

	char input[1024];
	char input2[1024];
	char *p1, *p2;
	size_t len_k, len_d;
	char *key, *data;
	int res;
	int exit = 0;

	ht_init_f(&ht, my_hash, my_cmp, free, free);

	if (argc >= 1) {
		printf("lol opening file\n");
		f = fopen(argv[1], "r");
		if (!f) {
			perror("opening file");
			return EXIT_FAILURE;
		}

		while (fscanf(f, "%s %s", input, input2) != EOF) {
			key = strdup(input);
			data = strdup(input2);
			ht_insert(ht, key, data);
		}

		fclose(f);
	}

	do {
		printf(PROMPT);
		if (fgets(input, sizeof input, stdin) == NULL)
			break;
		input[strlen(input)-1] = '\0';

		len_k = 0;
		len_d = 0;

		switch (*input) {
			case 'g':
			case 'r':
			case 'i':
			case 's':
				p1 = input+2;
				p2 = p1;
				while (*p2 != ' ' && *p2 != '\0') {
					len_k++;
					p2++;
				}
				key = malloc(len_k+1);
				memcpy(key, p1, len_k);
				key[len_k] = '\0';
				p1 += len_k;
				if (*input == 'i' || *input == 's') {
					p1 += 1;
					p2 = p1;
					while (*p2 != ' ' && p2 != '\0') {
						len_d++;
						p2++;
					}
					data = malloc(len_d+1);
					memcpy(data, p1, len_d);
					data[len_d] = '\0';
				}
		}

		switch (*input) {
			case 'g':
				data = ht_get(ht, key);
				printf("get \"%s\": \"%s\"\n", key, data);
				break;
			case 'r':
				data = ht_remove(ht, key);	
				printf("remove \"%s\": \"%s\"\n", key, data);
				break;
			case 'i':
				res = ht_insert(ht, key, data);
				printf("insert \"%s\": \"%s\" (%d)\n", key, data, res);
				break;
			case 's':
				res = ht_set(ht, key, data);
				printf("set \"%s\": \"%s\" (%d)\n", key, data, res);
				break;
			case 'q':
				exit = 1;
				break;
			default:
				printf(HELP);
		}
	} while (!exit);

	it = ht_iter(ht);

	printf("iterating hashtable\n");
	while (htiter_next(it, (void**)&key, (void**)&data)) {
		printf("\"%s\": \"%s\"\n", key, data);
	}
	
	return 0;
}
