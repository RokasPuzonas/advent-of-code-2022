#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"
#include "vec.h"

typedef struct {
	int from, to, amount;
} Move;

typedef struct {
	Vec *towers;
	Vec *moves;
} day5_Data;

static Move* day5_parse_move(char *line)
{
	char* line_copy = strdup(line);
	char* line_copy_original = line_copy;
	strsep(&line_copy, " ");
	char* amount = strsep(&line_copy, " ");
	strsep(&line_copy, " ");
	char* from = strsep(&line_copy, " ");
	strsep(&line_copy, " ");
	char* to = strsep(&line_copy, " ");

	Move *move = malloc(sizeof(Move));
	move->amount = atoi(amount);
	move->from = atoi(from)-1;
	move->to = atoi(to)-1;
	free(line_copy);
	return move;
}

static void *day5_parse(char **lines, int line_count)
{
	int tower_count = 0;
	int max_tower_height;

	for (int i = 0; i < line_count; i++) {
		if (lines[i][0] == '\0') {
			max_tower_height = i-1;
			break;
		}

		tower_count = MAX(tower_count, (strlen(lines[i])+1)/4);
	}

	int move_count = line_count - max_tower_height - 2;
	Vec *moves = vec_malloc(move_count);
	for (int i = 0; i < move_count; i++) {
		char *line = lines[max_tower_height+2+i];
		vec_push(moves, day5_parse_move(line));
	}

	Vec *towers = vec_malloc(tower_count);
	for (int i = 0; i < tower_count; i++) {
		char* tower = calloc(26, sizeof(char));
		vec_push(towers, tower);
	}
	for (int i = max_tower_height-1; i >= 0; i--) {
		char* line = lines[i];
		int line_size = strlen(line);
		for (int j = 0; j < tower_count; j++) {
			int index = j*4 + 1;
			if (index >= line_size) break;
			if (line[index] == ' ') continue;

			char* tower = towers->data[j];
			tower[max_tower_height-i-1] = line[index];
		}
	}

	day5_Data *data = malloc(sizeof(day5_Data));
	data->moves  = moves;
	data->towers = towers;
	return data;
}

static void do_move(char **towers, int *tower_sizes, int from, int to)
{
	int from_size = tower_sizes[from];
	int to_size = tower_sizes[to];
	towers[to][to_size] = towers[from][from_size-1];

	tower_sizes[from]--;
	tower_sizes[to]++;
}

static void do_move_many(char **towers, int *tower_sizes, int from, int to, int amount)
{
	int from_size = tower_sizes[from];
	int to_size = tower_sizes[to];
	memcpy(towers[to] + to_size, towers[from] + from_size - amount, amount);

	tower_sizes[from] -= amount;
	tower_sizes[to]  += amount;
}

static char* form_answer(char **towers, int tower_count, int *tower_sizes)
{
	char *answer = malloc(tower_count + 1);
	answer[tower_count] = '\0';
	for (int i = 0; i < tower_count; i++) {
		char *tower = towers[i];
		answer[i] = tower[tower_sizes[i]-1];
	}
	return answer;
}

static void day5_part1(void *p)
{
	day5_Data *data = p;
	int tower_count = data->towers->count;
	char *towers[tower_count];
	int tower_sizes[tower_count];
	for (int i = 0; i < tower_count; i++) {
		towers[i] = calloc(26, sizeof(char));
		memcpy(towers[i], data->towers->data[i], 26);

		tower_sizes[i] = 0;
		for (int j = 0; j < 26; j++) {
			if (towers[i][j] == 0) break;
			tower_sizes[i]++;
		}
	}

	for (int i = 0; i < data->moves->count; i++) {
		Move *move = data->moves->data[i];
		for (int j = 0; j < move->amount; j++) {
			do_move(towers, tower_sizes, move->from, move->to);
		}
	}

	printf("%s\n", form_answer(towers, tower_count, tower_sizes));
}

static void day5_part2(void *p)
{
	day5_Data *data = p;
	int tower_count = data->towers->count;
	char *towers[tower_count];
	int tower_sizes[tower_count];
	for (int i = 0; i < tower_count; i++) {
		towers[i] = calloc(26, sizeof(char));
		memcpy(towers[i], data->towers->data[i], 26);

		tower_sizes[i] = 0;
		for (int j = 0; j < 26; j++) {
			if (towers[i][j] == 0) break;
			tower_sizes[i]++;
		}
	}

	for (int i = 0; i < data->moves->count; i++) {
		Move *move = data->moves->data[i];
		do_move_many(towers, tower_sizes, move->from, move->to, move->amount);
	}

	printf("%s\n", form_answer(towers, tower_count, tower_sizes));
}

ADD_SOLUTION(5, day5_parse, day5_part1, day5_part2);
