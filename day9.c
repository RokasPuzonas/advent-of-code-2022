#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/param.h>

#include "aoc.h"
#include "point.h"

typedef enum {
	MOVE_DIR_UP,
	MOVE_DIR_DOWN,
	MOVE_DIR_LEFT,
	MOVE_DIR_RIGHT,
} MOVE_DIR;

typedef struct {
	MOVE_DIR dir;
	int count;
} RopeMove;

typedef struct {
	RopeMove *moves;
	int count;
} day9_Data;

static MOVE_DIR parse_move_dir(char dir)
{
	switch (dir) {
		case 'R': return MOVE_DIR_RIGHT;
		case 'L': return MOVE_DIR_LEFT;
		case 'U': return MOVE_DIR_UP;
		case 'D': return MOVE_DIR_DOWN;
	}
	abort();
}

static void *day9_parse(char **lines, int line_count)
{
	day9_Data *data = malloc(sizeof(day9_Data));
	data->moves = malloc(line_count * sizeof(RopeMove));
	data->count = line_count;

	for (int i = 0; i < line_count; i++) {
		char *line = lines[i];
		RopeMove *move = &data->moves[i];
		move->dir = parse_move_dir(line[0]);
		move->count = atoi(line+2);
	}

	return data;
}

static int sign(int x)
{
	return x > 0 ? 1 : -1;
}

static void get_bounding_box(day9_Data *moves, int *ox, int *oy, int *width, int *height)
{
	int minx = 0, maxx = 0;
	int miny = 0, maxy = 0;
	int x = 0, y = 0;
	for (int i = 0; i < moves->count; i++) {
		RopeMove *move = &moves->moves[i];
		switch (move->dir) {
			case MOVE_DIR_UP:
				y -= move->count;
				break;
			case MOVE_DIR_DOWN:
				y += move->count;
				break;
			case MOVE_DIR_LEFT:
				x -= move->count;
				break;
			case MOVE_DIR_RIGHT:
				x += move->count;
				break;
		}
		minx = MIN(x, minx);
		maxx = MAX(x, maxx);
		miny = MIN(y, miny);
		maxy = MAX(y, maxy);
	}

	*ox = -minx;
	*oy = -miny;
	*width = maxx - minx + 1;
	*height = maxy - miny + 1;
}

static void get_move_direction(MOVE_DIR dir, int *dx, int *dy)
{
	switch (dir) {
		case MOVE_DIR_UP:
			*dy = -1;
			break;
		case MOVE_DIR_DOWN:
			*dy = 1;
			break;
		case MOVE_DIR_LEFT:
			*dx = -1;
			break;
		case MOVE_DIR_RIGHT:
			*dx = 1;
			break;
	}
}

static void follow_point(Point *tail, Point *head)
{
	int diffx = abs(head->x - tail->x);
	int diffy = abs(head->y - tail->y);
	if (diffx > 1 || diffy > 1) {
		if (diffx + diffy > 2) {
			tail->x += sign(head->x - tail->x);
			tail->y += sign(head->y - tail->y);
		} else if (diffx > 1) {
			tail->x += sign(head->x - tail->x);
		} else if (diffy > 1) {
			tail->y += sign(head->y - tail->y);
		}
	}
}

static void day9_part1(void *p)
{
	day9_Data *moves = p;

	int ox, oy, width, height;
	get_bounding_box(moves, &ox, &oy, &width, &height);

	bool map[width][height];
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			map[x][y] = false;
		}
	}
	map[ox][oy] = true;

	Point head = { 0, 0 };
	Point tail = { 0, 0 };
	for (int i = 0; i < moves->count; i++) {
		RopeMove *move = &moves->moves[i];
		int dx = 0, dy = 0;
		get_move_direction(move->dir, &dx, &dy);
		for (int j = 0; j < move->count; j++) {
			head.x += dx;
			head.y += dy;

			follow_point(&tail, &head);
			map[tail.x+ox][tail.y+oy] = true;
		}
	}

	int result = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			result += map[x][y];
		}
	}
	printf("%d\n", result);
}

static void day9_part2(void *p)
{
	day9_Data *moves = p;

	int ox, oy, width, height;
	get_bounding_box(moves, &ox, &oy, &width, &height);

	bool map[width][height];
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			map[x][y] = false;
		}
	}
	map[ox][oy] = true;

	int rope_size = 10;
	Point rope[rope_size];
	for (int i = 0; i < rope_size; i++) {
		rope[i].x = 0;
		rope[i].y = 0;
	}

	for (int i = 0; i < moves->count; i++) {
		RopeMove *move = &moves->moves[i];
		int dx = 0, dy = 0;
		get_move_direction(move->dir, &dx, &dy);
		for (int _ = 0; _ < move->count; _++) {
			Point *head = &rope[0];
			head->x += dx;
			head->y += dy;

			for (int j = 1; j < rope_size; j++) {
				follow_point(&rope[j], &rope[j-1]);
			}

			Point *tail = &rope[rope_size-1];
			map[tail->x+ox][tail->y+oy] = true;
		}
	}

	int result = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			result += map[x][y];
		}
	}
	printf("%d\n", result);
}

ADD_SOLUTION(9, day9_parse, day9_part1, day9_part2);
