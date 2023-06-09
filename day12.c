#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "point.h"
#include "aoc.h"

typedef struct {
	u8 *map;
	u32 width;
	u32 height;
	Point_u32 start;
	Point_u32 end;
} day12_map;

static void print_day12_map(day12_map *map)
{
	for (size_t y = 0; y < map->height; y++) {
		for (size_t x = 0; x < map->width; x++) {
			if (map->start.x == x && map->start.y == y) {
				printf("S");
			} else if (map->end.x == x && map->end.y == y) {
				printf("E");
			} else {
				size_t idx = y * map->width + x;
				printf("%c", 'a' + map->map[idx]);
			}
		}
		printf("\n");
	}
}

static void *day12_parse(char **lines, int line_count)
{
	day12_map *map = malloc(sizeof(day12_map));
	map->width = strlen(lines[0]);
	map->height = line_count;
	map->map = malloc(sizeof(u8) * map->height * map->width);

	for (int y = 0; y < line_count; y++) {
		for (int x = 0; x < map->width; x++) {
			size_t idx = y * map->width + x;
			if (lines[y][x] == 'S') {
				map->start.x = x;
				map->start.y = y;
				map->map[idx] = 0;
			} else if (lines[y][x] == 'E') {
				map->end.x = x;
				map->end.y = y;
				map->map[idx] = 'z' - 'a';
			} else {
				map->map[idx] = lines[y][x] - 'a';
			}
		}
	}

	return map;
}

static bool day12_is_in_bounds(day12_map *map, i32 x, i32 y)
{
	return x >= 0 && y >= 0 && x < map->width && y < map->height;
}

static int day12_find_by_coord(u32 *stack_x, u32 *stack_y, size_t stack_size, u32 x, u32 y)
{
	for (int i = 0; i < stack_size; i++) {
		if (stack_x[i] == x && stack_y[i] == y) {
			return i;
		}
	}
	return -1;
}

static void day12_remove_at(u32 *array, u32 size, size_t at)
{
	for (int i = at; i < size-1; i++) {
		array[i] = array[i+1];
	}
}

static void day12_insert_at(u32 *array, u32 size, u32 capacity, size_t at, u32 value)
{
	assert(size+1 <= capacity);
	for (int i = size; i > at; i--) {
		array[i] = array[i-1];
	}
	array[at] = value;
}

static size_t day12_find_sorted_idx(day12_map *map, u32 *cost_map, u32 *stack_x, u32 *stack_y, size_t stack_size, u32 cost)
{
	for (int i = 0; i < ((int)stack_size)-1; i++) {
		u32 x = stack_x[i];
		u32 y = stack_y[i];
		if (cost_map[y * map->width + x] <= cost) {
			return i;
		}
	}
	return stack_size;
}

static u32 *day12_djikstra(day12_map *map)
{
	u32 *cost_map = malloc(map->height*map->width * sizeof(u32));
	for (size_t y = 0; y < map->height; y++) {
		for (size_t x = 0; x < map->width; x++) {
			cost_map[y * map->width + x] = -1;
		}
	}
	cost_map[map->end.y * map->width + map->end.x] = 0;

	size_t stack_capacity = map->width*map->height;
	u32 stack_x[stack_capacity], stack_y[stack_capacity];
	size_t stack_size = 1;
	stack_x[0] = map->end.x;
	stack_y[0] = map->end.y;

	while (stack_size > 0)
	{
		u32 x = stack_x[stack_size-1];
		u32 y = stack_y[stack_size-1];
		u32 cost = cost_map[y * map->width + x];
		stack_size--;

		if (x == map->start.x && y == map->start.y) break;

		u8 height = map->map[y * map->width + x];

		i8 ox[] = { 0, 0, 1, -1 };
		i8 oy[] = { 1, -1, 0, 0 };
		for (int i = 0; i < 4; i++) {
			i32 new_x = x + ox[i];
			i32 new_y = y + oy[i];
			if (!day12_is_in_bounds(map, new_x, new_y)) continue;
			u8 new_height = map->map[new_y * map->width + new_x];
			if (height - new_height > 1) continue;

			if (cost_map[new_y * map->width + new_x] > cost + 1) {
				if (cost_map[new_y * map->width + new_x] == cost + 1) continue;

				int stack_idx = day12_find_by_coord(stack_x, stack_y, stack_size, new_x, new_y);
				if (stack_idx != -1) {
					day12_remove_at(stack_x, stack_size, stack_idx);
					day12_remove_at(stack_y, stack_size, stack_idx);
					stack_size--;
				}
				cost_map[new_y * map->width + new_x] = cost + 1;
				size_t sorted_idx = day12_find_sorted_idx(map, cost_map, stack_x, stack_y, stack_size, cost + 1);
				day12_insert_at(stack_x, stack_size, stack_capacity, sorted_idx, new_x);
				day12_insert_at(stack_y, stack_size, stack_capacity, sorted_idx, new_y);
				stack_size++;
			}
		}
	}

	return cost_map;
}

static void day12_part1(void *p)
{
	day12_map *map = (day12_map*)p;

	u32 *cost_map = day12_djikstra(map);
	printf("Cost: %d\n", cost_map[map->start.y * map->width + map->start.x]);
}

static void day12_part2(void *p)
{
	day12_map *map = (day12_map*)p;

	u32 *cost_map = day12_djikstra(map);
	u32 lowest_cost = -1;
	for (u32 y = 0; y < map->height; y++) {
		for (u32 x = 0; x < map->height; x++) {
			u32 idx = y * map->width + x;
			u32 cost = cost_map[idx];
			if (lowest_cost > cost && map->map[idx] == 0) {
				lowest_cost = cost;
			}
		}
	}

	printf("Cost: %d\n", lowest_cost);
}

ADD_SOLUTION(12, day12_parse, day12_part1, day12_part2);
