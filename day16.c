#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/param.h>

#include "types.h"
#include "aoc.h"

struct day16_valve {
	u32 id;
	char name[3];

	u32 flowrate;
	struct day16_valve **valves;
	u32 valve_count;
};

struct day16_data {
	struct day16_valve *valves;
	size_t count;
};

static struct day16_valve* find_valve_by_name(struct day16_valve* valves, size_t count, char *name)
{
	for (size_t i = 0; i < count; i++) {
		if (valves[i].name[0] == name[0] && valves[i].name[1] == name[1]) {
			return &valves[i];
		}
	}
	return NULL;
}

static size_t count_char_in_str(char *str, char c)
{
	size_t count = 0;
	for (size_t i = 0; i < strlen(str); i++) {
		count += (str[i] == c);
	}
	return count;
}

static void* day16_parse(char** lines, int line_count)
{
	struct day16_data *data = malloc(sizeof(struct day16_data));
	data->count = line_count;
	data->valves = malloc(sizeof(struct day16_valve) * line_count);

	char *connections[line_count];

	for (size_t i = 0; i < line_count; i++) {
		struct day16_valve *valve = &data->valves[i];
		valve->id = i;
		valve->name[0] = lines[i][6];
		valve->name[1] = lines[i][7];
		valve->name[2] = 0;

		valve->flowrate = strtol(strchr(lines[i], '=')+1, NULL, 10);

		char *valves_word = strstr(lines[i], "valves");
		if (valves_word) {
			connections[i] = strstr(lines[i], "valves") + 7;
		} else {
			connections[i] = strstr(lines[i], "valve") + 6;
		}
	}

	for (int i = 0; i < line_count; i++) {
		struct day16_valve *valve = &data->valves[i];
		valve->valve_count = count_char_in_str(connections[i], ',') + 1;
		valve->valves = malloc(sizeof(struct day16_valve*) * valve->valve_count);

		int j = 0;
		for (char *tok = strtok(connections[i], ", "); tok; tok = strtok(NULL, ", ")) {
			valve->valves[j] = find_valve_by_name(data->valves, data->count, tok);
			j++;
		}
	}

	return data;
}

static void day16_get_nearby_closed_valves(
		struct day16_valve **valves, size_t valve_count,
		struct day16_valve **closed_valves, size_t *closed_count,
		size_t from_valve, u32 time_limit, size_t **distances,
		bool *opened
	)
{
	*closed_count = 0;

	for (size_t i = 0; i < valve_count; i++) {
		struct day16_valve *valve = valves[i];
		if (opened[valve->id]) continue;

		u32 distance = distances[from_valve][valve->id];
		if (distance+1 < time_limit) {
			closed_valves[*closed_count] = valve;
			(*closed_count)++;
		}
	}
}

static void day16_get_nonzero_valves(
		struct day16_valve *valves, size_t valve_count,
		struct day16_valve **nonzero_valves, size_t *nonzero_count
	)
{
	*nonzero_count = 0;

	for (size_t i = 0; i < valve_count; i++) {
		struct day16_valve *valve = &valves[i];
		if (valve->flowrate > 0) {
			nonzero_valves[*nonzero_count] = valve;
			(*nonzero_count)++;
		}
	}
}

static u32 day16_search(struct day16_valve **valves, size_t valve_count, size_t from_valve, u32 time_limit, size_t **distances, bool *opened)
{
	struct day16_valve *closed_valves[valve_count];
	size_t closed_count = 0;
	day16_get_nearby_closed_valves(valves, valve_count, closed_valves, &closed_count, from_valve, time_limit, distances, opened);

	u32 best_preassure = 0;
	for (size_t i = 0; i < closed_count; i++) {
		struct day16_valve *valve = closed_valves[i];
		u32 distance = distances[from_valve][valve->id];

		opened[valve->id] = true;
		u32 preasure_from_subsequent = day16_search(valves, valve_count, valve->id, time_limit - distance - 1, distances, opened);
		opened[valve->id] = false;

		u32 preassure_released = (time_limit - distance - 1) * valve->flowrate;
		best_preassure = MAX(best_preassure, preasure_from_subsequent  + preassure_released);
	}

	return best_preassure;
}

// Floyd-Warshall
static size_t** day16_compute_distances(struct day16_valve *valves, size_t valve_count)
{
	size_t **distances = malloc(sizeof(size_t*) * valve_count);
	for (size_t i = 0; i < valve_count; i++) {
		distances[i] = malloc(sizeof(size_t) * valve_count);
	}

	for (size_t i = 0; i < valve_count; i++) {
		struct day16_valve *valve = &valves[i];
		for (size_t j = 0; j < valve_count; j++) {
			distances[i][j] = valve_count;
		}
		distances[i][i] = 0;

		for (size_t j = 0; j < valve->valve_count; j++) {
			struct day16_valve *neighbour = valve->valves[j];

			distances[i][neighbour->id] = 1;
			distances[neighbour->id][i] = 1;
		}
	}

	for (size_t idx = 0; idx < valve_count; idx++) {
		for (size_t i = 0; i < valve_count; i++) {
			if (i == idx) continue;

			for (size_t j = 0; j < valve_count; j++) {
				if (j == idx) continue;
				if (i == j) continue;

				distances[i][j] = MIN(distances[i][j], distances[i][idx] + distances[idx][j]);
			}
		}
	}

	return distances;
}

static void day16_part1(void *p)
{
	struct day16_data *data = (struct day16_data *)p;

	struct day16_valve *starting_valve = find_valve_by_name(data->valves, data->count, "AA");
	size_t** distances = day16_compute_distances(data->valves, data->count);

	bool opened[data->count];
	memset(opened, false, data->count * sizeof(bool));

	struct day16_valve *nonzero_valves[data->count];
	size_t nonzero_count = 0;
	day16_get_nonzero_valves(data->valves, data->count, nonzero_valves, &nonzero_count);

	u32 answer = day16_search(nonzero_valves, nonzero_count, starting_valve->id, 30, distances, opened);
	printf("%d\n", answer);
}

struct day16_part2_entry {
	u32 visited_bitmask;
	size_t current;
	u32 time_left;
	u32 preassure;
};

static bool day16_is_visited(u32 visited_bitmask, size_t id)
{
	return (visited_bitmask & (1 << id)) > 0;
}

static void day16_part2(void *p)
{
	struct day16_data *data = (struct day16_data *)p;

	struct day16_valve *starting_valve = find_valve_by_name(data->valves, data->count, "AA");
	size_t** distances = day16_compute_distances(data->valves, data->count);

	bool opened[data->count];
	memset(opened, false, data->count * sizeof(bool));

	struct day16_valve *nonzero_valves[data->count];
	size_t nonzero_count = 0;
	day16_get_nonzero_valves(data->valves, data->count, nonzero_valves, &nonzero_count);

	#define DAY16_MAX_STACK 256
	struct day16_part2_entry stack[DAY16_MAX_STACK] = { 0 };
	size_t stack_size = 0;

	assert(nonzero_count < 32);

	u32 traversal_combinations = 1 << nonzero_count;

	i32 best_preassures[traversal_combinations];
	for (size_t i = 0; i < traversal_combinations; i++) {
		best_preassures[i] = -1;
	}

	stack[0].time_left = 26;
	stack[0].current = starting_valve->id;
	stack_size = 1;

	while (stack_size > 0) {
		struct day16_part2_entry entry = stack[stack_size-1];
		stack_size--;

		best_preassures[entry.visited_bitmask] = MAX(best_preassures[entry.visited_bitmask], (i32)entry.preassure);

		for (size_t i = 0; i < nonzero_count; i++) {
			struct day16_valve *valve = nonzero_valves[i];
			if (day16_is_visited(entry.visited_bitmask, i)) continue;

			u32 distance = distances[entry.current][valve->id];
			if (entry.time_left <= distance+1) continue;

			assert(stack_size+1 < DAY16_MAX_STACK);

			struct day16_part2_entry *new_entry = &stack[stack_size];
			i32 time_left = entry.time_left - distance - 1;
			new_entry->current = valve->id;
			new_entry->time_left = time_left;
			new_entry->preassure = entry.preassure + time_left * valve->flowrate;
			new_entry->visited_bitmask = entry.visited_bitmask | (1 << i);
			stack_size++;
		}
	}

	i32 best_preassure_overall = -1;
	for (size_t i = 1; i < traversal_combinations; i++) {
		if (best_preassures[i] == -1) continue;

		for (size_t j = 1; j < traversal_combinations; j++) {
			if ((i & j) > 0) continue;
			if (best_preassures[j] == -1) continue;

			best_preassure_overall = MAX(best_preassures[i] + best_preassures[j], best_preassure_overall);
		}
	}

	printf("%d\n", best_preassure_overall);
}

ADD_SOLUTION(16, day16_parse, day16_part1, day16_part2);
