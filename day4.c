#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"
#include "vec.h"

typedef struct {
	int from, to;
} Range;

typedef struct {
	Range first;
	Range second;
} DoubleRange;

static inline void day4_parse_range(Range *range, char *s)
{
	range->from = atoi(strsep(&s, "-"));
	range->to   = atoi(strsep(&s, "-"));
}

static void day4_parse_line(DoubleRange *double_range, char *line)
{
	char *line_copy = strdup(line);
	char *line_copy_start = line_copy;
	day4_parse_range(&double_range->first, strsep(&line_copy, ","));
	day4_parse_range(&double_range->second, strsep(&line_copy, ","));
	free(line_copy_start);
}

static void *day4_parse(char **lines, int line_count)
{
	Vec *vec = vec_malloc(line_count);
	for (size_t i = 0; i < line_count; i++) {
		DoubleRange *double_range = malloc(sizeof(DoubleRange));
		vec_push(vec, double_range);
		day4_parse_line(double_range, lines[i]);
	}
	return vec;
}

static int day4_part1(void *p)
{
	Vec *vec = p;
	int result = 0;
	for (int i = 0; i < vec->count; i++) {
		DoubleRange *double_range = vec->data[i];
		Range *range1 = &double_range->first;
		Range *range2 = &double_range->second;
		if ((range1->from <= range2->from && range1->to >= range2->to) ||
				(range2->from <= range1->from && range2->to >= range1->to)) {
			result++;
		}
	}
	return result;
}

static int day4_part2(void *p)
{
	Vec *vec = p;
	int result = 0;
	for (int i = 0; i < vec->count; i++) {
		DoubleRange *double_range = vec->data[i];
		Range *range1 = &double_range->first;
		Range *range2 = &double_range->second;
		if ((range1->from <= range2->from && range1->to >= range2->to) ||
				(range2->from <= range1->from && range2->to >= range1->to) ||
				MIN(range1->to, range2->to) >= MAX(range1->from, range2->from)) {
			result++;
		}
	}
	return result;
}


ADD_SOLUTION(4, day4_parse, day4_part1, day4_part2);
