#include <stdio.h>
#include <assert.h>

#include "types.h"
#include "aoc.h"

typedef struct {
	i64 *numbers;
	u32 count;
} day20_data;

typedef struct day20_node *day20_node_ptr;
struct day20_node {
	i64 value;
	day20_node_ptr next;
	day20_node_ptr prev;
};

static void* day20_parse(char** lines, int line_count)
{
	day20_data *data = malloc(sizeof(day20_data));
	data->numbers = malloc(sizeof(i64) * line_count);
	data->count = line_count;

	for (int i = 0; i < line_count; i++) {
		data->numbers[i] = strtol(lines[i], NULL, 10);
	}

	return data;
}

static struct day20_node *day20_list_alloc(i64 *values, u32 count)
{
	struct day20_node *start = calloc(1, sizeof(struct day20_node));
	start->value = values[0];
	start->next = NULL;

	struct day20_node *prev = start;
	for (int i = 1; i < count; i++) {
		struct day20_node *curr = calloc(1, sizeof(struct day20_node));
		curr->value = values[i];
		curr->prev = prev;
		prev->next = curr;
		prev = curr;
	}

	prev->next = start;
	start->prev = prev;

	return start;
}

static void day20_list_free(struct day20_node *list)
{
	struct day20_node *curr = list;
	curr->prev->next = NULL;
	while (curr) {
		struct day20_node *next = curr->next;
		curr->next = NULL;
		free(curr);
		curr = next;
	}
}

static struct day20_node *day20_list_get(struct day20_node *list, i64 nth, u32 list_size)
{
	struct day20_node *curr = list;
	if (nth > 0) {
		for (int i = 0; i < nth % list_size; i++) {
			curr = curr->next;
		}
	} else if (nth < 0) {
		for (int i = 0; i < (-nth+1) % list_size; i++) {
			curr = curr->prev;
		}
	}
	return curr;
}

static struct day20_node *day20_list_find(struct day20_node *list, i64 value)
{
	struct day20_node *curr = list;
	do {
		if (curr->value == value) {
			return curr;
		}
		curr = curr->next;
	} while (curr != list);
	return NULL;
}

static void day20_node_remove(struct day20_node *node)
{
	struct day20_node *prev = node->prev;
	struct day20_node *next = node->next;

	prev->next = next;
	next->prev = prev;
}

static void day20_node_insert_after(struct day20_node *node, struct day20_node *new_node)
{
	struct day20_node *next = node->next;

	new_node->prev = node;
	new_node->next = next;
	node->next = new_node;
	next->prev = new_node;
}

static void day20_print_list(struct day20_node *list)
{
	struct day20_node *curr = list;
	do {
		printf("%ld ", curr->value);
		curr = curr->next;
	} while (curr != list);
	printf("\n");
}

static void day20_mix_list(struct day20_node **order, u32 count)
{
	for (int i = 0; i < count; i++) {
		struct day20_node *curr = order[i];
		if (curr->value == 0) continue;

		day20_node_remove(curr);
		struct day20_node *other = day20_list_get(curr, curr->value, count-1);
		day20_node_insert_after(other, curr);
	}
}

static void day20_part1(void *p)
{
	day20_data *data = (day20_data*)p;

	struct day20_node *list = day20_list_alloc(data->numbers, data->count);

	struct day20_node *order[data->count];
	order[0] = list;
	for (int i = 1; i < data->count; i++)
	{
		order[i] = order[i-1]->next;
	}

	day20_mix_list(order, data->count);

	i64 answer = 0;
	struct day20_node *zero_node = day20_list_find(list, 0);
	answer += day20_list_get(zero_node, 1000, data->count)->value;
	answer += day20_list_get(zero_node, 2000, data->count)->value;
	answer += day20_list_get(zero_node, 3000, data->count)->value;
	printf("%ld\n", answer);

	day20_list_free(list);
}

static void day20_part2(void *p)
{
	day20_data *data = (day20_data*)p;

	i64 decryption_key = 811589153;
	for (int i = 0; i < data->count; i++) {
		data->numbers[i] *= decryption_key;
	}

	struct day20_node *list = day20_list_alloc(data->numbers, data->count);

	struct day20_node *order[data->count];
	order[0] = list;
	for (int i = 1; i < data->count; i++)
	{
		order[i] = order[i-1]->next;
	}

	for (int i = 0; i < 10; i++) {
		day20_mix_list(order, data->count);
	}

	i64 answer = 0;
	struct day20_node *zero_node = day20_list_find(list, 0);
	answer += day20_list_get(zero_node, 1000, data->count)->value;
	answer += day20_list_get(zero_node, 2000, data->count)->value;
	answer += day20_list_get(zero_node, 3000, data->count)->value;
	printf("%ld\n", answer);

	day20_list_free(list);
}

ADD_SOLUTION(20, day20_parse, day20_part1, day20_part2);
