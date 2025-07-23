#include <pcut/pcut.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <str.h>
#include "../wildcards.h"


PCUT_INIT;


PCUT_TEST(basic_and_user_cases)
{
	PCUT_ASSERT_FALSE(wildcard_comp("aho", "Ah"));
	PCUT_ASSERT_TRUE(wildcard_comp("n*", "nie"));
	PCUT_ASSERT_TRUE(wildcard_comp("a*c", "abc"));
	PCUT_ASSERT_TRUE(wildcard_comp("a*c", "axc"));
	PCUT_ASSERT_TRUE(wildcard_comp("a*c", "ac"));
	PCUT_ASSERT_FALSE(wildcard_comp("abc", "ab"));
	PCUT_ASSERT_FALSE(wildcard_comp("ab", "abc"));
	PCUT_ASSERT_TRUE(wildcard_comp("ab", "ab"));
	PCUT_ASSERT_TRUE(wildcard_comp("*", "hello"));
	PCUT_ASSERT_TRUE(wildcard_comp("he*lo", "hello"));
	PCUT_ASSERT_TRUE(wildcard_comp("he*lo", "helo"));
	PCUT_ASSERT_TRUE(wildcard_comp("*abc*", "xabcy"));
	PCUT_ASSERT_FALSE(wildcard_comp("abc", "xyz"));
}

PCUT_TEST(empty_string_cases)
{
	PCUT_ASSERT_TRUE(wildcard_comp("", ""));
	PCUT_ASSERT_TRUE(wildcard_comp("*", ""));
	PCUT_ASSERT_FALSE(wildcard_comp("", "a"));
	PCUT_ASSERT_FALSE(wildcard_comp("a", ""));
	PCUT_ASSERT_FALSE(wildcard_comp("a*", ""));
}

PCUT_TEST(multiple_and_consecutive_wildcard_cases)
{
	PCUT_ASSERT_TRUE(wildcard_comp("a**b", "axxb"));
	PCUT_ASSERT_TRUE(wildcard_comp("a**b", "ab"));
	PCUT_ASSERT_TRUE(wildcard_comp("*a*b*", "zzza_b_zzz"));
	PCUT_ASSERT_TRUE(wildcard_comp("*a*b*", "ab"));
	PCUT_ASSERT_TRUE(wildcard_comp("ab**", "abxyz"));
	PCUT_ASSERT_TRUE(wildcard_comp("***", "abc"));
}

PCUT_TEST(complex_and_backtracking_cases)
{
	PCUT_ASSERT_TRUE(wildcard_comp("*a*b", "sssaaasbb"));
	PCUT_ASSERT_FALSE(wildcard_comp("*a*b", "sssaccc"));
	PCUT_ASSERT_TRUE(wildcard_comp("a*d", "abcd"));
	PCUT_ASSERT_FALSE(wildcard_comp("a*d", "ab_c_e"));
	PCUT_ASSERT_TRUE(wildcard_comp("a*a*a", "aaaa"));
	PCUT_ASSERT_FALSE(wildcard_comp("f*f*", "f"));
	PCUT_ASSERT_TRUE(wildcard_comp("*b", "aabab"));
}

PCUT_TEST(additional_edge_cases)
{
	PCUT_ASSERT_TRUE(wildcard_comp("*bc", "abc"));
	PCUT_ASSERT_TRUE(wildcard_comp("*", "*"));
	PCUT_ASSERT_TRUE(wildcard_comp("*abc", "ababc"));
	PCUT_ASSERT_TRUE(wildcard_comp("*abc", "abcabc"));
}

PCUT_TEST(contains_wildcard)
{
	PCUT_ASSERT_TRUE(contains_wildcard("a*b"));
	PCUT_ASSERT_TRUE(contains_wildcard("a?b"));
	PCUT_ASSERT_FALSE(contains_wildcard("abc"));
	PCUT_ASSERT_FALSE(contains_wildcard(""));
	PCUT_ASSERT_TRUE(contains_wildcard("*"));
	PCUT_ASSERT_TRUE(contains_wildcard("?"));
	PCUT_ASSERT_TRUE(contains_wildcard("a?b?c"));
	PCUT_ASSERT_TRUE(contains_wildcard("a*b*c"));
}

PCUT_TEST(question_mark_wildcard)
{
	PCUT_ASSERT_TRUE(wildcard_comp("a?c", "abc"));
	PCUT_ASSERT_TRUE(wildcard_comp("?", "a"));
	PCUT_ASSERT_FALSE(wildcard_comp("?", ""));
	PCUT_ASSERT_FALSE(wildcard_comp("?", "ab"));
	PCUT_ASSERT_TRUE(wildcard_comp("??", "ab"));
	PCUT_ASSERT_FALSE(wildcard_comp("??", "a"));
	PCUT_ASSERT_TRUE(wildcard_comp("a?*", "abc"));
	PCUT_ASSERT_TRUE(wildcard_comp("?*", "abc"));
	PCUT_ASSERT_FALSE(wildcard_comp("a?c", "ac"));      
	PCUT_ASSERT_TRUE(wildcard_comp("a?*", "ab"));       
	PCUT_ASSERT_FALSE(wildcard_comp("a?c", "abbc"));    
}

PCUT_TEST(utf8_wildcard_tests)
{
    PCUT_ASSERT_TRUE(wildcard_comp("čau*", "čaučkovanie"));
    PCUT_ASSERT_FALSE(wildcard_comp("*ďeň", "pekný deň"));
    PCUT_ASSERT_TRUE(wildcard_comp("pä?eň", "päťeň"));
    PCUT_ASSERT_FALSE(wildcard_comp("pä?eň", "pápeň"));
    PCUT_ASSERT_TRUE(wildcard_comp("ž*š*", "žlté šaty"));
    PCUT_ASSERT_FALSE(wildcard_comp("ž?š", "žlté šaty"));
    PCUT_ASSERT_TRUE(wildcard_comp("Γειά*σου*", "Γειά σου Κόσμε"));
    PCUT_ASSERT_FALSE(wildcard_comp("Γειά?σου*", "Γεια σου Κόσμε"));
}

typedef struct {
	char **items;
	size_t count;
	size_t capacity;
} match_list_t;

static errno_t collect_match(char *path, void *arg) {
	// printf("Collected match: %s\n", path);
	match_list_t *list = (match_list_t *) arg;
	if (list->count >= list->capacity) {
		list->capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
		list->items = realloc(list->items, list->capacity * sizeof(char *));
	}
	list->items[list->count++] = str_dup(path);
	return EOK;
}

static bool list_contains(match_list_t *list, const char *value) {
	for (size_t i = 0; i < list->count; ++i) {
		if (str_cmp(list->items[i], value) == 0)
			return true;
	}
	return false;
}

static void free_list(match_list_t *list) {
	for (size_t i = 0; i < list->count; ++i)
		free(list->items[i]);
	free(list->items);
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
}

PCUT_TEST(wildcards_expand_tests)
{
	match_list_t matches = {0};
	
	// 1. Match all txt files in current dir
	expand_wildcard_patterns("*.txt", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/file.txt"));
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/čaute.txt"));
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/spaced name.txt"));
	PCUT_ASSERT_FALSE(list_contains(&matches, "/data/wildcards_test/another_file.md"));
	free_list(&matches);

	// 2. Match files in dir/*
	matches = (match_list_t){0};
	expand_wildcard_patterns("dir/*", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/dir/file_in_dir.txt"));
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/dir/čučoriedka.md"));
	PCUT_ASSERT_FALSE(list_contains(&matches, "/data/wildcards_test/dir/nested/deep.txt")); // not direct
	free_list(&matches);

	// 3. Match recursively
	matches = (match_list_t){0};
	expand_wildcard_patterns("dir/*/*.txt", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/dir/nested/deep.txt"));
	free_list(&matches);

	// 4. UTF-8 wildcard match
	matches = (match_list_t){0};
	expand_wildcard_patterns("*č*", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_contains(&matches, "/data/wildcards_test/čaute.txt"));
	PCUT_ASSERT_FALSE(list_contains(&matches, "/data/wildcards_test/dir/čučoriedka.md"));
	free_list(&matches);
}


PCUT_MAIN();

