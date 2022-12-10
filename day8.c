#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"

typedef struct {
	uint8_t **data;
	size_t width, height;
} day8_Map;

static void *day8_parse(char **lines, int line_count)
{
	day8_Map *map = malloc(sizeof(day8_Map));
	map->height = line_count;
	map->width = strlen(lines[0]);
	map->data = malloc(map->height*sizeof(uint8_t*));
	for (int y = 0; y < map->height; y++) {
		map->data[y] = malloc(map->width*sizeof(uint8_t));
		for (int x = 0; x < map->width; x++) {
			map->data[y][x] = lines[y][x] - '0';
		}
	}
	return map;
}

static bool is_visible_horizontal(day8_Map *map, uint8_t value, int x0, int x1, int y)
{
	for (int xi = x0; xi < x1; xi++) {
		if (map->data[y][xi] >= value) return false;
	}
	return true;
}

static bool is_visible_vertical(day8_Map *map, uint8_t value, int x, int y0, int y1)
{
	for (int yi = y0; yi < y1; yi++) {
		if (map->data[yi][x] >= value) return false;
	}
	return true;
}

static bool is_visible(day8_Map *map, int x, int y) {
	uint8_t current = map->data[y][x];
	return is_visible_horizontal (map, current, x+1, map->width, y) ||
					is_visible_horizontal(map, current, 0, x, y) ||
					is_visible_vertical  (map, current, x, y+1, map->height) ||
					is_visible_vertical  (map, current, x, 0, y);
}

static int get_horizontal_edge(day8_Map *map, uint8_t value, int x0, int x1, int y)
{
	if (x0 <= x1) {
		for (int xi = x0; xi <= x1; xi++) {
			if (map->data[y][xi] >= value) return xi;
		}
	} else {
		for (int xi = x0; xi >= x1; xi--) {
			if (map->data[y][xi] >= value) return xi;
		}
	}
	return x1;
}

static int get_vertical_edge(day8_Map *map, uint8_t value, int x, int y0, int y1)
{
	if (y0 <= y1) {
		for (int yi = y0; yi <= y1; yi++) {
			if (map->data[yi][x] >= value) return yi;
		}
	} else {
		for (int yi = y0; yi >= y1; yi--) {
			if (map->data[yi][x] >= value) return yi;
		}
	}
	return y1;
}

static int get_score(day8_Map *map, int x, int y) {
	uint8_t value = map->data[y][x];
	int right_edge  = get_horizontal_edge(map, value, x+1, map->width-1, y);
	int left_edge   = get_horizontal_edge(map, value, x-1, 0  , y);
	int bottom_edge = get_vertical_edge  (map, value, x, y+1, map->height-1);
	int top_edge    = get_vertical_edge  (map, value, x, y-1, 0);
	return (right_edge - x) * (x - left_edge) * (bottom_edge - y) * (y - top_edge);
}

static void day8_part1(void *p)
{
	day8_Map *map = p;

	int result = map->height * map->width - (map->height-2) * (map->width-2);
	for (int y = 1; y < map->height-1; y++) {
		for (int x = 1; x < map->width-1; x++) {
			result += is_visible(map, x, y);
		}
	}

	printf("%d\n", result);
}

static void day8_part2(void *p)
{
	day8_Map *map = p;

	int result = 0;
	for (int y = 1; y < map->height-1; y++) {
		for (int x = 1; x < map->width-1; x++) {
			result = MAX(result, get_score(map, x, y));
		}
	}

	printf("%d\n", result);
}

ADD_SOLUTION(8, day8_parse, day8_part1, day8_part2);
