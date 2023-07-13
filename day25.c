#include <stdio.h>
#include <string.h>

#include "types.h"
#include "aoc.h"

typedef struct {
	char **snafu_numbers;
	u32 count;
} day25_data;

static void* day25_parse(char** lines, int line_count)
{
	day25_data *data = malloc(sizeof(day25_data));
	data->snafu_numbers = lines;
	data->count = line_count;
	return data;
}

static u64 day25_snafu_to_decimal(char *snafu)
{
	u64 decimal = 0;
	u64 power = 1;
	for (int i = strlen(snafu)-1; i >= 0; i--) {
		if (snafu[i] == '2') {
			decimal += 2 * power;
		} else if (snafu[i] == '1') {
			decimal += 1 * power;
		} else if (snafu[i] == '0') {
			decimal += 0 * power;
		} else if (snafu[i] == '-') {
			decimal += -1 * power;
		} else if (snafu[i] == '=') {
			decimal += -2 * power;
		}
		power *= 5;
	}
	return decimal;
}

static void day25_print_snafu(u64 number)
{
	u32 length = 0;
	char snafu[64] = { 0 };
	while (number > 0) {
		u32 remainder = number % 5;
		number = number / 5;

		if (0 <= remainder && remainder <= 2) {
			snafu[length++] = '0' + remainder;
		} else if (remainder == 3) {
			snafu[length++] = '=';
			number += 1;
		} else { //if (remainder == 4) {
			snafu[length++] = '-';
			number += 1;
		}
	}

	for (int i = length-1; i >= 0; i--) {
		printf("%c", snafu[i]);
	}
	printf("\n");
}

static void day25_part1(void *p)
{
	day25_data *data = (day25_data*)p;

	u64 sum = 0;
	for (int i = 0; i < data->count; i++) {
		sum += day25_snafu_to_decimal(data->snafu_numbers[i]);
	}

	day25_print_snafu(sum);
}

static void day25_part2(void *p)
{
	day25_data *data = (day25_data*)p;
}

ADD_SOLUTION(25, day25_parse, day25_part1, day25_part2);
