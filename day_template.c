#include "aoc.h"

typedef struct {
} day00_data;

static void* day00_parse(char** lines, int line_count)
{
	day00_data *data = malloc(sizeof(day00_data));
	return data;
}

static void day00_part1(void *p)
{
	day00_data *data = (day00_data*)p;
}

static void day00_part2(void *p)
{
	day00_data *data = (day00_data*)p;
}

ADD_SOLUTION(00, day00_parse, day00_part1, day00_part2);
