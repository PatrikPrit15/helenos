#include <pcut/pcut.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <str.h>
#include "../wildcards.h"


PCUT_INIT;


static void assert_wildcard_true(const char *pattern, const char *text) {
    bool res = false;
    PCUT_ASSERT_ERRNO_VAL(EOK, wildcard_comp(pattern, text, &res));
	if (res == false) {
		fprintf(stderr, "Wildcard match failed for pattern '%s' and text '%s'\n", pattern, text);
	}	
    PCUT_ASSERT_TRUE(res);
}

static void assert_wildcard_false(const char *pattern, const char *text) {
    bool res = true;
    PCUT_ASSERT_ERRNO_VAL(EOK, wildcard_comp(pattern, text, &res));
	if (res == true) {
		fprintf(stderr, "Wildcard match succeeded for pattern '%s' and text '%s', but expected false\n", pattern, text);
	}
    PCUT_ASSERT_FALSE(res);
}

PCUT_TEST(basic_and_user_cases)
{
    assert_wildcard_false("aho", "Ah");
    assert_wildcard_true("n*", "nie");
    assert_wildcard_true("a*c", "abc");
    assert_wildcard_true("a*c", "axc");
    assert_wildcard_true("a*c", "ac");
    assert_wildcard_false("abc", "ab");
    assert_wildcard_false("ab", "abc");
    assert_wildcard_true("ab", "ab");
    assert_wildcard_true("*", "hello");
    assert_wildcard_true("he*lo", "hello");
    assert_wildcard_true("he*lo", "helo");
    assert_wildcard_true("*abc*", "xabcy");
    assert_wildcard_false("abc", "xyz");
}

PCUT_TEST(empty_string_cases)
{
    assert_wildcard_true("", "");
    assert_wildcard_true("*", "");
    assert_wildcard_false("", "a");
    assert_wildcard_false("a", "");
    assert_wildcard_false("a*", "");
}

PCUT_TEST(multiple_and_consecutive_wildcard_cases)
{
    assert_wildcard_true("a**b", "axxb");
    assert_wildcard_true("a**b", "ab");
    assert_wildcard_true("*a*b*", "zzza_b_zzz");
    assert_wildcard_true("*a*b*", "ab");
    assert_wildcard_true("ab**", "abxyz");
    assert_wildcard_true("***", "abc");
}

PCUT_TEST(complex_and_backtracking_cases)
{
    assert_wildcard_true("*a*b", "sssaaasbb");
    assert_wildcard_false("*a*b", "sssaccc");
    assert_wildcard_true("a*d", "abcd");
    assert_wildcard_false("a*d", "ab_c_e");
    assert_wildcard_true("a*a*a", "aaaa");
    assert_wildcard_false("f*f*", "f");
    assert_wildcard_true("*b", "aabab");
}

PCUT_TEST(additional_edge_cases)
{
    assert_wildcard_true("*bc", "abc");
    assert_wildcard_true("*", "*");
    assert_wildcard_true("*abc", "ababc");
    assert_wildcard_true("*abc", "abcabc");
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
    assert_wildcard_true("a?c", "abc");
    assert_wildcard_true("?", "a");
    assert_wildcard_false("?", "");
    assert_wildcard_false("?", "ab");
    assert_wildcard_true("??", "ab");
    assert_wildcard_false("??", "a");
    assert_wildcard_true("a?*", "abc");
    assert_wildcard_true("?*", "abc");
    assert_wildcard_false("a?c", "ac");
    assert_wildcard_true("a?*", "ab");
    assert_wildcard_false("a?c", "abbc");
}

PCUT_TEST(utf8_wildcard_tests)
{
    assert_wildcard_true("čau*", "čaučkovanie");
    assert_wildcard_false("*ďeň", "pekný deň");
    assert_wildcard_true("pä?eň", "päťeň");
    assert_wildcard_false("pä?eň", "pápeň");
    assert_wildcard_true("ž*š*", "žlté šaty");
    assert_wildcard_false("ž?š", "žlté šaty");
    assert_wildcard_true("Γειά*σου*", "Γειά σου Κόσμε");
    assert_wildcard_false("Γειά?σου*", "Γεια σου Κόσμε");
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

static bool list_check_at_index(match_list_t *list, int index, const char *value) {
	if (index < 0 || (size_t)index >= list->count) {
		return false;
	}
	return str_cmp(list->items[index], value) == 0;
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
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 0, "/data/wildcards_test/file.txt"));
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 1, "/data/wildcards_test/spaced name.txt"));
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 2, "/data/wildcards_test/čaute.txt"));
	PCUT_ASSERT_INT_EQUALS(3, matches.count);
	free_list(&matches);

	// 2. Match files in dir/*
	matches = (match_list_t){0};
	expand_wildcard_patterns("dir/*", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 0, "/data/wildcards_test/dir/file_in_dir.txt"));
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 1, "/data/wildcards_test/dir/nested"));
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 2, "/data/wildcards_test/dir/čučoriedka.md"));
	PCUT_ASSERT_INT_EQUALS(3, matches.count);
	free_list(&matches);

	// 3. Match recursively
	matches = (match_list_t){0};
	expand_wildcard_patterns("dir/*/*.txt", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 0, "/data/wildcards_test/dir/nested/deep.txt"));
	PCUT_ASSERT_INT_EQUALS(1, matches.count);
	free_list(&matches);

	// 4. UTF-8 wildcard match
	matches = (match_list_t){0};
	expand_wildcard_patterns("*č*", "/data/wildcards_test/", collect_match, &matches);
	PCUT_ASSERT_TRUE(list_check_at_index(&matches, 0, "/data/wildcards_test/čaute.txt"));
	PCUT_ASSERT_INT_EQUALS(1, matches.count);
	free_list(&matches);
}


PCUT_MAIN();

