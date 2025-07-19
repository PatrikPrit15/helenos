#include <pcut/pcut.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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



PCUT_MAIN();

