#include <stdio.h>
#include <stdarg.h>
#include <str.h>
#include <mem.h>
#include <stdlib.h>
#include "dirent.h"
#include "main.h"

size_t min(size_t a, size_t b);
int max(int a, int b);
typedef void (*Callback)(char*);

bool contains_wildcard(const char *pattern);
void expand_patterns(const char *pattern, const char *path, Callback callback);
void log_expansion(char *expanded_path);

size_t min(size_t a, size_t b) { return (a < b ? a : b); }

int max(int a, int b) { return (a > b ? a : b); }

bool wildcard_comp(char *pattern, char *file_name){ // fixme UTF-8
	const size_t pattern_len = str_length(pattern) + 1;
	const size_t file_name_len = str_length(file_name) + 1;

	/*
	 * Dynamic programming comparator for wildcard matching 
	 */
	bool **dp = malloc((pattern_len + 1) * sizeof(bool *)); //todo 1x malloc
	for (size_t i = 0; i < pattern_len + 1; i++) {
		dp[i] = malloc((file_name_len + 1) * sizeof(bool));
		for (size_t j = 0; j < file_name_len + 1; j++) {
			dp[i][j] = 0;
		}
	}
	dp[0][0] = 1;

	for (size_t id_sum = 0; id_sum <= pattern_len + file_name_len - 2; id_sum++){
		for (size_t i = max(0, id_sum - file_name_len + 1); i <= min(pattern_len - 1, id_sum); i++){
			size_t j = id_sum - i;

			if (pattern[i] == '*'){
				dp[i + 1][j] = dp[i + 1][j] | dp[i][j];
				dp[i][j + 1] = dp[i][j + 1] | dp[i][j];
				dp[i + 1][j + 1] = dp[i + 1][j + 1] | dp[i][j];
			} else {
				if (pattern[i] == file_name[j]){
					dp[i + 1][j + 1] = dp[i + 1][j + 1] | dp[i][j];
				}
			}
			//printf("%d %d -> %d\n", i,j, dp[i][j]);
		}
	}

	bool result = dp[pattern_len - 1][file_name_len - 1];

	// Free allocated memory
	for (size_t i = 0; i < pattern_len + 1; i++) {
		free(dp[i]);
	}
	free(dp);

	return result;
}

bool contains_wildcard(const char *pattern) { // fixme UTF-8
	if (pattern == NULL) {
		return false;
	}
	for (size_t i = 0; i < str_length(pattern); i++) { 
		if (pattern[i] == '*' || pattern[i] == '?') {
			return true;
		}
	}
	return false;
}


void expand_patterns(const char *pattern, const char *path, Callback callback) {
	// printf("Expanding pattern: '%s' in path: '%s'\n", pattern, path);
	if (!contains_wildcard(pattern)) { // Base case: no wildcards, or end of pattern
		char *full_path = NULL;
		asprintf(&full_path, "%s%s", path, pattern);
		callback(full_path);
		return ;
	} 

	// Processing next token

	char *start = str_dup(pattern);

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
		return ;
	}

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		// printf("Checking entry: %s\n", entry->d_name);
		if (wildcard_comp((char *)start, entry->d_name)) {
			char *full_path = NULL;
			
			if (slash) {
				asprintf(&full_path, "%s%s/", path, entry->d_name);
			} else {
				asprintf(&full_path, "%s%s", path, entry->d_name);
			}

			expand_patterns(slash ? slash + 1 : "", full_path, callback);
		}
	}

	closedir(dir);
}

void log_expansion(char *expanded_path) {
	printf("Expanded pattern: '%s'\n", expanded_path);
}

int main(int argc, char *argv[]){
	const char *pattern = ".";
	const char *path = "";
	if (argc > 1) {
		pattern = argv[1];
	}


	expand_patterns(pattern, path, log_expansion);

    return 0;
}