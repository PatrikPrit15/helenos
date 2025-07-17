#include <stdio.h>
#include <stdarg.h>
#include <str.h>
#include <mem.h>
#include <stdlib.h>
#include "dirent.h"
#include "main.h"
#include "string_datastructure.h"

size_t min(size_t a, size_t b);
int max(int a, int b);
bool contains_wildcard(const char *pattern);
StringList expand_patterns(const char *pattern, const char *path);
StringList find_matches(const char *pattern, const char *dirpath);
char *standardize_pattern(const char *pattern, const char *path);


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

bool contains_wildcard(const char *pattern) {
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


StringList expand_patterns(const char *non_standard_pattern, const char *path) {
	if (!contains_wildcard(non_standard_pattern)) { // Base case: no wildcards
		StringList single_item_list;
		list_init(&single_item_list);
		list_add(&single_item_list, non_standard_pattern);
		return single_item_list;
	} 

	// Checking whether the pattern is something that needs to be executed
	bool is_run_command = false;
	char* non_standard_pattern_copy = (char *)non_standard_pattern;
	if (non_standard_pattern_copy[0] == '.') {
		is_run_command = true;
		non_standard_pattern_copy++;
	}

	// Standardizing the pattern to absolute path
	const char *standardized_pattern = standardize_pattern(non_standard_pattern_copy, path);
	char *pattern = str_dup(standardized_pattern);


	StringList expanded_list;
	list_init(&expanded_list);
	list_add(&expanded_list, "/");


	// Splitting the pattern and processing each token
	char *start = str_chr(pattern, '/') + 1;
	while (*start != '\0') {
		char *slash = str_chr(start, '/');
		if (slash) {
			*slash = '\0';
		}

		// Process the current token
		printf("Processing token: %s\n", start);
		StringList current_list;
		list_init(&current_list);
		for (size_t i = 0; i < expanded_list.size; i++) {
			printf("  Expanded item %zu: %s\n", i, expanded_list.items[i]);
			StringList matches = find_matches(start, expanded_list.items[i]);

			for (size_t j = 0; j < matches.size; j++) {
				printf("    Match %zu: %s\n", j, matches.items[j]);
				// Create a new item by combining the expanded item and the match
				list_add(&current_list, matches.items[j]);
			}

			list_free(&matches);
		}


		list_free(&expanded_list);
		expanded_list = current_list;


		if (!slash){
			break;
		}
		start = slash + 1;
	}
	

	if (is_run_command) { // If the pattern was a run command, prepend "." to each item
		for (size_t i = 0; i < expanded_list.size; i++) {
			char *new_item = NULL;
			asprintf(&new_item, ".%s", expanded_list.items[i]);
			free(expanded_list.items[i]);
			expanded_list.items[i] = new_item;
		}
	}

	return expanded_list;
}

// Function to find matches for a given pattern in a directory, to depth = 1, shallow search
StringList find_matches(const char *pattern, const char *dirpath) {
	StringList matches;
	list_init(&matches);
	printf("Finding matches for pattern '%s' in directory '%s'\n", pattern, dirpath);

	DIR *dir = opendir(dirpath);
	if (!dir) {
		fprintf(stderr, "opendir failed on '%s'\n", dirpath);
		return matches;
	}

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		printf("Checking entry: %s\n", entry->d_name);
		if (wildcard_comp((char *)pattern, entry->d_name)) {
			char *full_path = NULL;
			if (dirpath[str_length(dirpath) - 1] == '/') {
				asprintf(&full_path, "%s%s", dirpath, entry->d_name);
			} else {
				asprintf(&full_path, "%s/%s", dirpath, entry->d_name);
			}
			list_add(&matches, full_path);
			printf("Match found: %s\n", full_path);
		}
	}

	closedir(dir);
	return matches;
}

char* standardize_pattern(const char *pattern, const char *path) {
	char *standardized_pattern = NULL;

	if (pattern[0] != '/') {
		if (path[str_length(path) - 1] == '/') {
			asprintf(&standardized_pattern, "%s%s", path, pattern);
		} else {
			asprintf(&standardized_pattern, "%s/%s", path, pattern);
		}
	} else {
		standardized_pattern = str_dup(pattern);
	}

	return standardized_pattern;
}

int main(int argc, char *argv[]){
	const char *pattern = ".";
	const char *path = "/";
	if (argc > 2) {
		pattern = argv[1];
		path = argv[2];
	}


	StringList expanded = expand_patterns(pattern, path);


	printf("Expanded patterns:\n");
	for (size_t i = 0; i < expanded.size; i++) {
		printf("%s\n", expanded.items[i]);
	}

	list_free(&expanded);

    return 0;
}