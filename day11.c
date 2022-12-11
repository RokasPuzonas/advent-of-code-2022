#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aoc.h"

typedef enum {
	MONKEY_OP_ADD,
	MONKEY_OP_MUL,
	MONKEY_OP_SQR
} MONKEY_OP;

typedef struct {
	int *items;
	int item_count;
	MONKEY_OP op;
	int op_value;
	int test_value;
	int test_true;
	int test_false;
} Monkey;

typedef struct {
	Monkey *monkeys;
	int count;
} day11_Data;

static int count_char(char *str, char target)
{
	int count = 0;
	for (int i = 0; i < strlen(str); i++) {
		count += str[i] == target;
	}
	return count;
}

static void parse_items(char *items, int **result, int *result_count)
{
	int i = 0;
	*result_count = count_char(items, ',')+1;
	*result = malloc(*result_count * sizeof(int));
	for (char *num = strtok(items, ","); num; num = strtok(NULL, ",")) {
		(*result)[i] = atoi(num+1);
		i++;
	}
}

static void parse_operation(char *operation, MONKEY_OP *op, int *op_value)
{
	if (operation[4] == '+') {
		*op = MONKEY_OP_ADD;
		*op_value = atoi(operation + 6);
	} else if (operation[4] == '*' && operation[6] == 'o') {
		*op = MONKEY_OP_SQR;
	} else {
		*op = MONKEY_OP_MUL;
		*op_value = atoi(operation + 6);
	}
}

static inline void apply_operation(uint64_t *value, MONKEY_OP op, int op_value)
{
	switch (op) {
	case MONKEY_OP_ADD:
		*value += op_value;
		break;
	case MONKEY_OP_MUL:
		*value *= op_value;
		break;
	case MONKEY_OP_SQR:
		*value *= *value;
		break;
	default:
		printf("abort: %d\n", op);
		fflush(stdout);
		abort();
	}
}

static void *day11_parse(char **lines, int line_count)
{
	day11_Data *data = malloc(sizeof(day11_Data));
	data->count = (line_count + 1)/7;
	data->monkeys = malloc(data->count * sizeof(Monkey));

	for (int i = 0; i < line_count; i+=7) {
		Monkey *monkey = &data->monkeys[i/7];
		char *starting_items = lines[i+1] + 17;
		char *operation      = lines[i+2] + 19;
		char *test_value     = lines[i+3] + 21;
		char *test_true      = lines[i+4] + 29;
		char *test_false     = lines[i+5] + 30;
		parse_items(starting_items, &monkey->items, &monkey->item_count);
		parse_operation(operation, &monkey->op, &monkey->op_value);
		monkey->test_value = atoi(test_value);
		monkey->test_true  = atoi(test_true);
		monkey->test_false = atoi(test_false);
	}

	return data;
}

static uint64_t solve(day11_Data *data, int rounds, bool reduce_worry)
{
	int monkey_count = data->count;

	int inspections[monkey_count];
	for (int i = 0; i < data->count; i++) {
		inspections[i] = 0;
	}

	int total_item_count = 0;
	for (int i = 0; i < monkey_count; i++) {
		total_item_count += data->monkeys[i].item_count;
	}

	size_t items[monkey_count][total_item_count];
	size_t item_counts[monkey_count];
	for (int i = 0; i < monkey_count; i++) {
		Monkey *monkey = &data->monkeys[i];
		item_counts[i] = monkey->item_count;
		for (int j = 0; j < monkey->item_count; j++) {
			items[i][j] = monkey->items[j];
		}
	}

	int modulo = 1;
	for (int i = 0; i < monkey_count; i++) {
		modulo *= data->monkeys[i].test_value;
	}

	for (int round = 1; round <= rounds; round++) {
		for (int i = 0; i < monkey_count; i++) {
			Monkey *monkey = &data->monkeys[i];
			for (int j = 0; j < item_counts[i]; j++) {
				uint64_t item_worry = items[i][j];
				apply_operation(&item_worry, monkey->op, monkey->op_value);
				if (reduce_worry) {
					item_worry /= 3;
				} else {
					item_worry %= modulo;
				}

				int target_monkey;
				if (item_worry % monkey->test_value == 0) {
					target_monkey = monkey->test_true;
				} else {
					target_monkey = monkey->test_false;
				}

				items[target_monkey][item_counts[target_monkey]++] = item_worry;
			}

			inspections[i] += item_counts[i];
			item_counts[i] = 0;
		}

	}

	uint64_t top_inspection1 = 0;
	uint64_t top_inspection2 = 0;
	for (int i = 0; i < monkey_count; i++) {
		if (inspections[i] > top_inspection1) {
			top_inspection2 = top_inspection1;
			top_inspection1 = inspections[i];
		} else if (inspections[i] > top_inspection2) {
			top_inspection2 = inspections[i];
		}
	}

	return top_inspection1*top_inspection2;
}

static void day11_part1(void *p)
{
	printf("%lu\n", solve(p, 20, true));
}


static void day11_part2(void *p)
{
	printf("%lu\n", solve(p, 10000, false));
}

ADD_SOLUTION(11, day11_parse, day11_part1, day11_part2);
