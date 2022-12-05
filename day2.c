#include <stdio.h>
#include <stdlib.h>

#include "aoc.h"
#include "vec.h"

typedef struct {
	char you, opponent;
} Round;

static void *day2_parse(char **lines, int line_count)
{
	Vec *vec = vec_malloc(line_count);

	for (int i = 0; i < line_count; i++) {
		Round *round = malloc(sizeof(Round));
		round->opponent = lines[i][0];
		round->you = lines[i][2];
		vec_push(vec, round);
	}

	return vec;
}

static void day2_part1(void *p)
{
	Vec *rounds = p;
	int result = 0;
	for (int i = 0; i < rounds->count; i++) {
		Round *round = rounds->data[i];
		char you = round->you;
		char opponent = round->opponent;

		if (you == 'X') {
			result += 1;
		} else if (you == 'Y') {
			result += 2;
		} else if (you == 'Z') {
			result += 3;
		}

		if ((you == 'X' && opponent == 'A') ||
				(you == 'Y' && opponent == 'B') ||
				(you == 'Z' && opponent == 'C')) {
			result += 3;
		} else if ((you == 'X' && opponent == 'C') ||
								(you == 'Y' && opponent == 'A') ||
								(you == 'Z' && opponent == 'B')) {
			result += 6;
		}
	}
	printf("%d\n", result);
}

static void day2_part2(void *p)
{
	Vec *data = p;
	int result = 0;
	for (int i = 0; i < data->count; i++) {
		Round *round = data->data[i];
		char you = round->you;
		char opponent = round->opponent;

		if (you == 'X') { // lose
			if (opponent == 'A') result += 3; // opponent = rock    , me = scissors
			if (opponent == 'B') result += 1; // opponent = paper   , me = rock
			if (opponent == 'C') result += 2; // opponent = scissors, me = paper
		} else if (you == 'Y') { // draw
			if (opponent == 'A') result += 1;
			if (opponent == 'B') result += 2;
			if (opponent == 'C') result += 3;
		} else if (you == 'Z') { // win
			if (opponent == 'A') result += 2; // opponent = rock    , me = paper
			if (opponent == 'B') result += 3; // opponent = paper   , me = scissors
			if (opponent == 'C') result += 1; // opponent = scissors, me = rock
		}

		if (you == 'Y') {
			result += 3;
		} else if (you == 'Z') {
			result += 6;
		}
	}
	printf("%d\n", result);
}

ADD_SOLUTION(2, day2_parse, day2_part1, day2_part2);
