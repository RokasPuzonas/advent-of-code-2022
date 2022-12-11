#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aoc.h"

typedef enum {
	INST_TYPE_NOOP,
	INST_TYPE_ADD,
} INST_TYPE;

typedef struct {
	INST_TYPE type;
	int amount;
} Instruction;

typedef struct {
	Instruction *instructions;
	int count;
} day10_Data;

static void *day10_parse(char **lines, int line_count)
{
	day10_Data *data = malloc(sizeof(day10_Data));
	data->count = line_count;
	data->instructions = malloc(line_count * sizeof(Instruction));

	for (int i = 0; i < line_count; i++) {
		char *line = lines[i];
		Instruction *inst = &data->instructions[i];
		if (!strcmp(line, "noop")) {
			inst->type = INST_TYPE_NOOP;
		} else { // addx
			inst->type = INST_TYPE_ADD;
			inst->amount = atoi(line + 5);
		}
	}

	return data;
}

static void day10_part1(void *p)
{
	day10_Data *data = p;

	int regx = 1;
	int result = 0;
	int inst_idx = 0;
	int cycle = 0;
	int add_timer = 0;
	bool add_started = false;
	while (inst_idx < data->count) {
		Instruction *inst = &data->instructions[inst_idx];
		if (!add_started) {
			if (inst->type == INST_TYPE_NOOP) {
				inst_idx++;
			} else if (inst->type == INST_TYPE_ADD) {
				add_started = true;
				add_timer = 2;
			}
		}

		cycle++;

		if ((cycle - 20) % 40 == 0 && cycle <= 220) {
			result += cycle*regx;
		}

		if (add_started) {
			add_timer--;
			if (add_timer == 0) {
				add_started = false;
				regx += inst->amount;
				inst_idx++;
			}
		}
	}

	printf("%d\n", result);
}

static void day10_part2(void *p)
{
	day10_Data *data = p;

	int regx = 1;
	int inst_idx = 0;
	int cycle = 0;
	int add_timer = 0;
	bool add_started = false;
	while (inst_idx < data->count) {
		Instruction *inst = &data->instructions[inst_idx];
		if (!add_started) {
			if (inst->type == INST_TYPE_NOOP) {
				inst_idx++;
			} else if (inst->type == INST_TYPE_ADD) {
				add_started = true;
				add_timer = 2;
			}
		}

		if (abs(cycle % 40 - regx) <= 1) {
			printf("#");
		} else {
			printf(".");
		}

		cycle++;

		if (add_started) {
			add_timer--;
			if (add_timer == 0) {
				add_started = false;
				regx += inst->amount;
				inst_idx++;
			}
		}

		if (cycle % 40 == 0) {
			printf("\n");
		}
	}
}

ADD_SOLUTION(10, day10_parse, day10_part1, day10_part2);
