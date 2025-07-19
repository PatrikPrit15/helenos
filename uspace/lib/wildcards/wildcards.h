#ifndef MAIN_H
#define MAIN_H
#include <stdbool.h>

bool contains_wildcard(const char *pattern);
bool wildcard_comp(const char *pattern, const char *file_name);
errno_t expand_wildcard_patterns(const char *pattern, const char *path, errno_t (*callback)(char*, void *arg), void* arg);

#endif // MAIN_H
