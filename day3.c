#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aoc.h"
#include "vec.h"

static void *day3_parse(char **lines, int line_count)
{
	Vec *vec = vec_malloc(line_count);
	for (size_t i = 0; i < line_count; i++) {
		vec_push(vec, lines[i]);
	}
	return vec;
}

static bool contains(char needle, char* haystack, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (haystack[i] == needle) {
			return true;
		}
	}
	return false;
}

static char find_common(char *str1, char* str2, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (contains(str1[i], str2, len)) {
			return str1[i];
		}
	}
	return 0;
}

static int get_priority(char c)
{
		if ('a' <= c && c <= 'z') {
			return (c - 'a') + 1;
		} else if ('A' <= c && c <= 'Z') {
			return (c - 'A') + 27;
		}
		return 0;
}

static void day3_part1(void *p)
{
	Vec *vec = p;
	int result = 0;
	for (size_t i = 0; i < vec->count; i++) {
		char *b = vec->data[i];
		int size = strlen(b);
		char common = find_common(b, b + size/2, size/2);
		if (common) {
			result += get_priority(common);
		} else {
			fprintf(stderr, "Unknown common char at line: %zu\n", i+1);
		}
	}
	printf("%d\n", result);
}

static void day3_part2(void *p)
{
	Vec *vec = p;
	int result = 0;
	for (size_t i = 0; i < vec->count; i+=3) {
		char *b1 = vec->data[i+0];
		char *b2 = vec->data[i+1];
		char *b3 = vec->data[i+2];
		int size1 = strlen(b1);
		int size2 = strlen(b2);
		int size3 = strlen(b3);

		bool found = false;
		for (size_t j = 0; j < size1; j++) {
			char c = b1[j];
			if (contains(c, b2, size2) && contains(c, b3, size3)) {
				result += get_priority(c);
				found = true;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "Unknown common char at line: %zu-%zu\n", i+1, i+3);
		}
	}
	printf("%d\n", result);
}

ADD_SOLUTION(3, day3_parse, day3_part1, day3_part2);
