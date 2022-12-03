#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aoc.h"

typedef struct {
	char *contents;
	size_t size;
} Backpack;

typedef struct {
	Backpack *backpacks;
	size_t count;
} day3_Data;

static void *day3_parse(char **lines, int line_count)
{
	day3_Data *data = calloc(1, sizeof(day3_Data));
	Backpack *backpacks = calloc(line_count, sizeof(Backpack));
	data->backpacks = backpacks;
	data->count = line_count;
	for (size_t i = 0; i < line_count; i++) {
		backpacks[i].size = strlen(lines[i]);
		backpacks[i].contents = lines[i];
	}
	return data;
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

static int day3_part1(void *p)
{
	day3_Data *data = (day3_Data*)p;
	int result = 0;
	for (size_t i = 0; i < data->count; i++) {
		Backpack *b = &data->backpacks[i];
		char common = find_common(b->contents, b->contents+b->size/2, b->size/2);
		if (common) {
			result += get_priority(common);
		} else {
			fprintf(stderr, "Unknown common char at line: %zu\n", i+1);
		}
	}
	return result;
}

static int day3_part2(void *p)
{
	day3_Data *data = (day3_Data*)p;
	int result = 0;
	for (size_t i = 0; i < data->count; i+=3) {
		Backpack *b1 = &data->backpacks[i+0];
		Backpack *b2 = &data->backpacks[i+1];
		Backpack *b3 = &data->backpacks[i+2];

		bool found = false;
		for (size_t j = 0; j < b1->size; j++) {
			char c = b1->contents[j];
			if (contains(c, b2->contents, b2->size) && contains(c, b3->contents, b3->size)) {
				result += get_priority(c);
				found = true;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "Unknown common char at line: %zu-%zu\n", i+1, i+3);
		}
	}
	return result;
}

ADD_SOLUTION(3, day3_parse, day3_part1, day3_part2);
