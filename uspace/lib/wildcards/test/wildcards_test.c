/*
 * Copyright (c) 2025 Patrik Pritrsky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <pcut/pcut.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <str.h>
#include "../wildcards.h"


PCUT_INIT;


static bool test_wildcard_comp(const char *pattern, const char *text) {
    bool res = false;
    PCUT_ASSERT_ERRNO_VAL(EOK, wildcard_comp(pattern, text, &res));
	return res;
}



PCUT_TEST(basic_and_user_cases)
{
    PCUT_ASSERT_FALSE(test_wildcard_comp("aho", "Ah"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("n*", "nie"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a*c", "abc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a*c", "axc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a*c", "ac"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("abc", "ab"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("ab", "abc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("ab", "ab"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*", "hello"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("he*lo", "hello"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("he*lo", "helo"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*abc*", "xabcy"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("abc", "xyz"));
}

PCUT_TEST(empty_string_cases)
{
    PCUT_ASSERT_TRUE(test_wildcard_comp("", ""));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*", ""));
    PCUT_ASSERT_FALSE(test_wildcard_comp("", "a"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("a", ""));
    PCUT_ASSERT_FALSE(test_wildcard_comp("a*", ""));
}

PCUT_TEST(multiple_and_consecutive_wildcard_cases)
{
    PCUT_ASSERT_TRUE(test_wildcard_comp("a**b", "axxb"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a**b", "ab"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*a*b*", "zzza_b_zzz"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*a*b*", "ab"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("ab**", "abxyz"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("***", "abc"));
}

PCUT_TEST(complex_and_backtracking_cases)
{
    PCUT_ASSERT_TRUE(test_wildcard_comp("*a*b", "sssaaasbb"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("*a*b", "sssaccc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a*d", "abcd"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("a*d", "ab_c_e"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a*a*a", "aaaa"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("f*f*", "f"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*b", "aabab"));
}

PCUT_TEST(additional_edge_cases)
{
    PCUT_ASSERT_TRUE(test_wildcard_comp("*bc", "abc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*", "*"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*abc", "ababc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("*abc", "abcabc"));
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
    PCUT_ASSERT_TRUE(test_wildcard_comp("a?c", "abc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("?", "a"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("?", ""));
    PCUT_ASSERT_FALSE(test_wildcard_comp("?", "ab"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("??", "ab"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("??", "a"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a?*", "abc"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("?*", "abc"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("a?c", "ac"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("a?*", "ab"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("a?c", "abbc"));
}

PCUT_TEST(utf8_wildcard_tests)
{
    PCUT_ASSERT_TRUE(test_wildcard_comp("čau*", "čaučkovanie"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("*ďeň", "pekný deň"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("pä?eň", "päťeň"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("pä?eň", "pápeň"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("ž*š*", "žlté šaty"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("ž?š", "žlté šaty"));
    PCUT_ASSERT_TRUE(test_wildcard_comp("Γειά*σου*", "Γειά σου Κόσμε"));
    PCUT_ASSERT_FALSE(test_wildcard_comp("Γειά?σου*", "Γεια σου Κόσμε"));
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

    // 5. Recursive wildcard **
    matches = (match_list_t){0};
    expand_wildcard_patterns("**/*.txt", "/data/wildcards_test/", collect_match, &matches);
    PCUT_ASSERT_TRUE(list_check_at_index(&matches, 0, "/data/wildcards_test/file.txt"));
    PCUT_ASSERT_TRUE(list_check_at_index(&matches, 1, "/data/wildcards_test/spaced name.txt"));
    PCUT_ASSERT_TRUE(list_check_at_index(&matches, 2, "/data/wildcards_test/čaute.txt"));
    PCUT_ASSERT_TRUE(list_check_at_index(&matches, 3, "/data/wildcards_test/dir/file_in_dir.txt"));
    PCUT_ASSERT_TRUE(list_check_at_index(&matches, 4, "/data/wildcards_test/dir/nested/deep.txt"));
    PCUT_ASSERT_INT_EQUALS(5, matches.count);
    free_list(&matches);
}


PCUT_MAIN();

