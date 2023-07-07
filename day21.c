#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "types.h"
#include "aoc.h"

#define DAY21_MAX_MONKEYS 1048575 // 2^20-1

typedef enum {
	MONKEY21_OP_CONST,
	MONKEY21_OP_ADD,
	MONKEY21_OP_SUB,
	MONKEY21_OP_DIV,
	MONKEY21_OP_MUL,
} day21_monkey_op;

typedef struct {
	char name[5]; // null-terminated
	day21_monkey_op op;
	i32 constant;
	char monkey1[5]; // null-terminated
	char monkey2[5]; // null-terminated
} day21_monkey;

typedef struct {
	day21_monkey *monkeys;
	u32 count;
} day21_data;

static void* day21_parse(char** lines, int line_count)
{
	day21_data *data = malloc(sizeof(day21_data));
	data->monkeys = calloc(line_count, sizeof(day21_monkey));
	data->count = line_count;

	for (int i = 0; i < line_count; i++) {
		day21_monkey *monkey = &data->monkeys[i];
		char *line = lines[i];
		strncpy(monkey->name, line, sizeof(monkey->name)-1);

		char *after_colon = line+6;
		char *sep;
		if ((sep = strchr(after_colon, '+'))) {
			monkey->op = MONKEY21_OP_ADD;
			strncpy(monkey->monkey1, after_colon  , sizeof(monkey->monkey1)-1);
			strncpy(monkey->monkey2, after_colon+7, sizeof(monkey->monkey2)-1);
		} else if ((sep = strchr(after_colon, '-'))) {
			monkey->op = MONKEY21_OP_SUB;
			strncpy(monkey->monkey1, after_colon  , sizeof(monkey->monkey1)-1);
			strncpy(monkey->monkey2, after_colon+7, sizeof(monkey->monkey2)-1);
		} else if ((sep = strchr(after_colon, '*'))) {
			monkey->op = MONKEY21_OP_MUL;
			strncpy(monkey->monkey1, after_colon  , sizeof(monkey->monkey1)-1);
			strncpy(monkey->monkey2, after_colon+7, sizeof(monkey->monkey2)-1);
		} else if ((sep = strchr(after_colon, '/'))) {
			monkey->op = MONKEY21_OP_DIV;
			strncpy(monkey->monkey1, after_colon  , sizeof(monkey->monkey1)-1);
			strncpy(monkey->monkey2, after_colon+7, sizeof(monkey->monkey2)-1);
		} else {
			monkey->op = MONKEY21_OP_CONST;
			monkey->constant = strtol(after_colon, NULL, 10);
		}
	}

	return data;
}

static void day21_print_monkey(day21_monkey *monkey)
{
	printf("%s: ", monkey->name);
	switch (monkey->op) {
	case MONKEY21_OP_CONST:
		printf("%d\n", monkey->constant);
		break;
	case MONKEY21_OP_ADD:
		printf("%s + %s\n", monkey->monkey1, monkey->monkey2);
		break;
	case MONKEY21_OP_SUB:
		printf("%s - %s\n", monkey->monkey1, monkey->monkey2);
		break;
	case MONKEY21_OP_DIV:
		printf("%s / %s\n", monkey->monkey1, monkey->monkey2);
		break;
	case MONKEY21_OP_MUL:
		printf("%s * %s\n", monkey->monkey1, monkey->monkey2);
		break;
	}
}

static u32 day21_monkey_key(char *name)
{
	char c1 = name[0] - 'a';
	char c2 = name[1] - 'a';
	char c3 = name[2] - 'a';
	char c4 = name[3] - 'a';
	u32 key = (c4 << 15) | (c3 << 10) | (c2 << 5) | (c1 << 0);
	return key;
}
static day21_monkey *day21_monkey_by_name(day21_monkey *lut[DAY21_MAX_MONKEYS], char *name)
{
	return lut[day21_monkey_key(name)];
}

static day21_monkey **day21_create_monkey_lookup(day21_monkey *monkeys, u32 count)
{
	day21_monkey **monkey_lookup = calloc(DAY21_MAX_MONKEYS, sizeof(day21_monkey*));
	for (int i = 0; i < count; i++) {
		day21_monkey *monkey = &monkeys[i];
		u32 key = day21_monkey_key(monkey->name);
		assert(key <= DAY21_MAX_MONKEYS);
		assert(monkey_lookup[key] == NULL && "Key collision");
		monkey_lookup[key] = monkey;
	}

	return monkey_lookup;
}

static i64 day21_eval(day21_monkey *lut[DAY21_MAX_MONKEYS], char *name)
{
	day21_monkey *monkey = day21_monkey_by_name(lut, name);
	switch(monkey->op) {
	case MONKEY21_OP_CONST: return monkey->constant;
	case MONKEY21_OP_ADD:   return day21_eval(lut, monkey->monkey1) + day21_eval(lut, monkey->monkey2);
	case MONKEY21_OP_SUB:   return day21_eval(lut, monkey->monkey1) - day21_eval(lut, monkey->monkey2);
	case MONKEY21_OP_DIV:   return day21_eval(lut, monkey->monkey1) / day21_eval(lut, monkey->monkey2);
	case MONKEY21_OP_MUL:   return day21_eval(lut, monkey->monkey1) * day21_eval(lut, monkey->monkey2);
	}
}

static void day21_part1(void *p)
{
	day21_data *data = (day21_data*)p;

	day21_monkey **monkey_lookup = day21_create_monkey_lookup(data->monkeys, data->count);
	printf("%ld\n", day21_eval(monkey_lookup, "root"));
}

static bool day21_has_params(day21_monkey_op op)
{
	return op == MONKEY21_OP_ADD || op == MONKEY21_OP_SUB || op == MONKEY21_OP_DIV || op == MONKEY21_OP_MUL;
}

static u32 day21_walk_to(day21_monkey *lut[DAY21_MAX_MONKEYS], day21_monkey *from, char *to, day21_monkey **chain)
{
	if (day21_monkey_key(from->name) == day21_monkey_key(to)) {
		chain[0] = from;
		return 1;
	}

	if (!day21_has_params(from->op)) return 0;

	chain[0] = from;
	u32 chain_length;

	day21_monkey *monkey1 = day21_monkey_by_name(lut, from->monkey1);
	chain_length = day21_walk_to(lut, monkey1, to, chain+1);
	if (chain_length) {
		return chain_length+1;
	}

	day21_monkey *monkey2 = day21_monkey_by_name(lut, from->monkey2);
	chain_length = day21_walk_to(lut, monkey2, to, chain+1);
	if (chain_length) {
		return chain_length+1;
	}

	chain[0] = NULL;

	return 0;
}

static void day21_part2(void *p)
{
	day21_data *data = (day21_data*)p;
	day21_monkey **lut = day21_create_monkey_lookup(data->monkeys, data->count);

	day21_monkey *root = day21_monkey_by_name(lut, "root");

	day21_monkey *chain[data->count];
	memset(chain, 0, sizeof(day21_monkey*) * data->count);

	u32 chain_length = day21_walk_to(lut, root, "humn", chain);
	if (chain_length == 0) {
		printf("Failed to find 'humn' monkey\n");
		return;
	}

	day21_monkey *curr;
	i64 expected;
	if (day21_monkey_key(root->monkey1) == day21_monkey_key(chain[1]->name)) {
		expected = day21_eval(lut, root->monkey2);
		curr = lut[day21_monkey_key(root->monkey1)];
	} else {
		expected = day21_eval(lut, root->monkey1);
		curr = lut[day21_monkey_key(root->monkey2)];
	}

	u32 chain_idx = 2;
	for (int chain_idx = 2; chain_idx < chain_length; chain_idx++) {

		u32 monkey1_key = day21_monkey_key(curr->monkey1);
		u32 monkey2_key = day21_monkey_key(curr->monkey2);
		bool is_humn_lhs = (monkey1_key == day21_monkey_key(chain[chain_idx]->name));

		day21_monkey *side_branch = lut[is_humn_lhs ? monkey2_key : monkey1_key];
		day21_monkey *humn_branch = lut[is_humn_lhs ? monkey1_key : monkey2_key];
		i64 side_value = day21_eval(lut, side_branch->name);
		switch (curr->op) {
		case MONKEY21_OP_ADD:
			expected = expected - side_value;
			break;
		case MONKEY21_OP_SUB:
			if (is_humn_lhs) {
				expected = expected + side_value;
			} else {
				expected = side_value - expected;
			}
			break;
		case MONKEY21_OP_MUL:
			expected = expected / side_value;
			break;
		case MONKEY21_OP_DIV:
			if (is_humn_lhs) {
				expected = expected * side_value;
			} else {
				expected = side_value / expected ;
			}
			break;
		case MONKEY21_OP_CONST:
			assert(false && "Should never happen");
			break;
		}

		curr = humn_branch;
	}

	printf("%ld\n", expected);
}

ADD_SOLUTION(21, day21_parse, day21_part1, day21_part2);
