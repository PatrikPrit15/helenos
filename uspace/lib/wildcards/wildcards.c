#include <stdio.h>
#include <stdarg.h>
#include <str.h>
#include <mem.h>
#include <stdlib.h>
#include <errno.h>
#include "dirent.h"
#include "wildcards.h"


size_t min(size_t a, size_t b);
int max(int a, int b);
typedef errno_t (*Callback)(char *, void *arg);


size_t min(size_t a, size_t b) { return (a < b ? a : b); }
int max(int a, int b) { return (a > b ? a : b); }

/** Convert UTF-8 string to array of Unicode codepoints */
static size_t utf8_to_codepoints(const char *utf8, uint32_t **out_cp) {
    size_t len = str_length(utf8);
    uint32_t *cp = malloc((len + 1) * sizeof(uint32_t));
    if (!cp)
        exit(ENOMEM);

    size_t offset = 0;
    size_t count = 0;
    while (count < len) {
        uint32_t ch = str_decode(utf8, &offset, str_lsize(utf8, len));
        cp[count++] = ch;
    }
    *out_cp = cp;
    return len;
}

/** Returns whether wildcard pattern matches with provided target string */
bool wildcard_comp(const char *pattern, const char *target_string){ //! fixme UTF-8
    uint32_t *pattern_cp = NULL;
    uint32_t *target_string_cp = NULL;

    size_t pattern_len = utf8_to_codepoints(pattern, &pattern_cp) + 1;
    size_t target_string_len  = utf8_to_codepoints(target_string, &target_string_cp) + 1;

	
	bool **dp = malloc((pattern_len + 1) * sizeof(bool *) + (pattern_len + 1) * (target_string_len + 1) * sizeof(bool));
	if (dp == NULL) {
		exit(ENOMEM); 
	}
	bool *data = (bool *)(dp + pattern_len + 1);
	for (size_t i = 0; i <= pattern_len; i++) {
		dp[i] = data + i * (target_string_len + 1);
	}
	memset(data, 0, (pattern_len + 1) * (target_string_len + 1) * sizeof(bool));
	dp[0][0] = true;
	
	
	/*
	 * Dynamic programming comparator for wildcard matching 
	 */


	for (size_t id_sum = 0; id_sum <= pattern_len + target_string_len - 2; id_sum++){
		for (size_t i = max(0, id_sum - target_string_len + 1); i <= min(pattern_len - 1, id_sum); i++){
			size_t j = id_sum - i;

			if (pattern_cp[i] == '*'){
				dp[i + 1][j] = dp[i + 1][j] | dp[i][j];
				dp[i][j + 1] = dp[i][j + 1] | dp[i][j];
				dp[i + 1][j + 1] = dp[i + 1][j + 1] | dp[i][j];
			} else if (pattern_cp[i] == '?') {
				dp[i + 1][j + 1] = dp[i + 1][j + 1] | dp[i][j];
			} else {
				if (pattern_cp[i] == target_string_cp[j]){
					dp[i + 1][j + 1] = dp[i + 1][j + 1] | dp[i][j];
				}
			}
			//printf("%d %d -> %d\n", i,j, dp[i][j]);
		}
	}

	bool result = dp[pattern_len - 1][target_string_len - 1];

	free(dp);
	free(pattern_cp);
	free(target_string_cp);

	return result;
}

/** Returns whether string contains wildcard '*' or '?' */
bool contains_wildcard(const char *pattern) {
	if (pattern == NULL) {
		return false;
	}
	if (str_chr(pattern, '*') != NULL || str_chr(pattern, '?') != NULL) {
		return true;
	}
	return false;
}


/** Function that expands wildcard pattern and pushes all expanded items to callback */
errno_t expand_wildcard_patterns(const char *pattern, const char *path, Callback callback, void* arg) {
	// printf("Expanding pattern: '%s' in path: '%s'\n", pattern, path);
	if (!contains_wildcard(pattern)) { // Base case: no wildcards or end of pattern
		char *full_path = NULL;
		if (asprintf(&full_path, "%s%s", path, pattern) < 0) {
			return ENOMEM;
		}
		// printf("Expanding to: '%s'\n", full_path);
		errno_t rc = callback(full_path, arg);
		free(full_path);
		return rc;
	}

	// Processing next token

	char *start_orig = str_dup(pattern);
	if (start_orig == NULL) {
		return ENOMEM;
	}

	char *start = start_orig;
	// using absolute path
	if (start[0] == '/') {
		start++;
		path = "/";
	}

	char *slash = str_chr(start, '/');
	if (slash) {
		*slash = '\0';
	}


	DIR *dir = opendir(path);
	if (!dir) {
		// fprintf(stderr, "opendir failed on '%s'\n", path);
		free(start_orig);
		return 0; // Directory not found 
	}

	struct dirent *entry;
	errno_t rc = EOK;
	while ((entry = readdir(dir))) {
		// printf("Checking entry: %s\n", entry->d_name);
		if (wildcard_comp((char *)start, entry->d_name)) {
			char *full_path = NULL;
			
			if (slash) {
				if (asprintf(&full_path, "%s%s/", path, entry->d_name) < 0) {
					rc = ENOMEM;
					break;
				}
			} else {
				if (asprintf(&full_path, "%s%s", path, entry->d_name) < 0) {
					rc = ENOMEM;
					break;
				}
			}

			rc = expand_wildcard_patterns(slash ? slash + 1 : "", full_path, callback, arg);
			free(full_path);
			if (rc != EOK) {
				break;
			}
		}
	}

	closedir(dir);
	free(start_orig);
	return rc;
}

