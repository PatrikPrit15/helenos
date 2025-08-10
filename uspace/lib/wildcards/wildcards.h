#ifndef MAIN_H
#define MAIN_H
#include <stdbool.h>

typedef errno_t (*wildcards_match_found_callback_t)(char *, void *arg);


bool contains_wildcard(const char *pattern);
errno_t wildcard_comp(const char *pattern, const char *file_name, bool *result);
errno_t expand_wildcard_patterns(const char *pattern, const char *path, wildcards_match_found_callback_t callback, void* arg);

#endif // MAIN_H
