#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"

typedef struct {
	int *calories;
	size_t count;
} Calories;

typedef struct {
	Calories *elfs;
	size_t count;
} Data;

static void *parse(char **lines, int line_count) {
	int elf_indexes[300] = { 0 };
	int elf_count = 1;
	for (int i = 1; i < line_count; i++) {
		if (strlen(lines[i-1]) == 0) {
			elf_indexes[elf_count++] = i;
		}
	}
	elf_indexes[elf_count] = line_count+1;

	Data *data = calloc(1, sizeof(Data));
	data->elfs = calloc(elf_count, sizeof(Calories));
	data->count = elf_count;

	for (int i = 0; i < elf_count; i++) {
		int count = (elf_indexes[i+1]-elf_indexes[i])-1;
		Calories *calories = &data->elfs[i];
		calories->calories = calloc(count, sizeof(int));
		calories->count    = count;

		for (int j = 0; j < count; j++) {
			int line = elf_indexes[i]+j;
			calories->calories[j] = atoi(lines[line]);
		}
	}

	return data;
}

static int part1(void *p) {
	Data *data = (Data*)p;
	int max_calories = 0;
	for (int i = 0; i < data->count; i++) {
		int calories = 0;
		for (int j = 0; j < data->elfs[i].count; j++) {
			calories += data->elfs[i].calories[j];
		}
		max_calories = MAX(max_calories, calories);
	}
	return max_calories;
}

static int part2(void *p) {
	Data *data = (Data*)p;
	int max_calories1 = 0;
	int max_calories2 = 0;
	int max_calories3 = 0;
	for (int i = 0; i < data->count; i++) {
		int calories = 0;
		for (int j = 0; j < data->elfs[i].count; j++) {
			calories += data->elfs[i].calories[j];
		}
		if (calories > max_calories1) {
			max_calories3 = max_calories2;
			max_calories2 = max_calories1;
			max_calories1 = calories;
		} else if (calories > max_calories2) {
			max_calories3 = max_calories2;
			max_calories2 = calories;
		} else if(calories > max_calories3) {
			max_calories3 = calories;
		}
	}
	return max_calories1 + max_calories2 + max_calories3;
}

ADD_SOLUTION(1, parse, part1, part2);
