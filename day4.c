#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"

typedef struct {
	int from, to;
} Range;

typedef struct {
	Range first;
	Range second;
} DoubleRange;

typedef struct {
	DoubleRange *ranges;
	size_t count;
} day4_Data;

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
	day4_Data *data = calloc(1, sizeof(day4_Data));
	DoubleRange *ranges = calloc(line_count, sizeof(DoubleRange));
	data->ranges = ranges;
	data->count = line_count;
	for (size_t i = 0; i < line_count; i++) {
		day4_parse_line(&data->ranges[i], lines[i]);
	}
	return data;
}

static int day4_part1(void *p)
{
	day4_Data *data = (day4_Data*)p;
	int result = 0;
	for (int i = 0; i < data->count; i++) {
		Range *range1 = &data->ranges[i].first;
		Range *range2 = &data->ranges[i].second;
		if ((range1->from <= range2->from && range1->to >= range2->to) ||
				(range2->from <= range1->from && range2->to >= range1->to)) {
			result++;
		}
	}
	return result;
}

static int day4_part2(void *p)
{
	day4_Data *data = (day4_Data*)p;
	int result = 0;
	for (int i = 0; i < data->count; i++) {
		Range *range1 = &data->ranges[i].first;
		Range *range2 = &data->ranges[i].second;
		if ((range1->from <= range2->from && range1->to >= range2->to) ||
				(range2->from <= range1->from && range2->to >= range1->to) ||
				MIN(range1->to, range2->to) >= MAX(range1->from, range2->from)) {
			result++;
		}
	}
	return result;
}


ADD_SOLUTION(4, day4_parse, day4_part1, day4_part2);
