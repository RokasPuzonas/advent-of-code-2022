#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "types.h"
#include "vec2.h"
#include "aoc.h"

typedef struct {
	vec2_u32 *points;
	size_t count;
} day14_line;

typedef struct {
	day14_line *lines;
	size_t count;
} day14_input;

typedef enum {
	TILE_AIR,
	TILE_SOLID,
	TILE_SAND,
} map_tile;

typedef struct {
	map_tile *tiles;
	size_t width;
	size_t height;
	vec2_u32 offset;
} day14_map;

static void day14_parse_point(vec2_u32 *result, char *str)
{
	char *x = strtok(str, ",");
	result->x = strtoul(x, NULL, 10);

	char *y = strtok(NULL, ",");
	result->y = strtoul(y, NULL, 10);
}

static void day14_parse_line(day14_line *result, char *line)
{
	result->count = 1;
	for (int i = 0; i < strlen(line); i++) {
		if (line[i] == '>') {
			result->count++;
		}
	}
	result->points = malloc(sizeof(vec2_u32)*result->count);

	int i = 0;
	char *save;
	char *token = strtok_r(line, " -> ", &save);
	while(token) {
		day14_parse_point(&result->points[i], token);
		token = strtok_r(NULL, " -> ", &save);
		i++;
	}
}

static void *day14_parse(char **lines, int line_count)
{
	day14_input *result = malloc(sizeof(day14_input));
	result->lines = malloc(sizeof(day14_line)*line_count);
	result->count = line_count;
	for (int i = 0; i < line_count; i++) {
		day14_parse_line(&result->lines[i], lines[i]);
	}
	return result;
}

static void printf_day14_line(day14_line *line)
{
	if (line->count > 0) {
		for (int i = 0; i < line->count-1; i++) {
			vec2_u32 point = line->points[i];
			printf("%d,%d -> ", point.x, point.y);
		}
		vec2_u32 last = line->points[line->count-1];
		printf("%d,%d", last.x, last.y);
	}
}

static void printf_day14_map(day14_map *map)
{
	char map_tile_chars[] = {
		[TILE_AIR]   = '.',
		[TILE_SOLID] = '#',
		[TILE_SAND]  = 'O'
	};

	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			int idx = y * map->width + x;
			char tile = map_tile_chars[map->tiles[idx]];
			printf("%c", tile);
		}
		printf("\n");
	}
}

static void printf_day14_segment(vec2_u32 *A, vec2_u32 *B)
{
	printf("%d,%d -> %d,%d\n", A->x, A->y, B->x, B->y);
}

static void day14_place_vertical(day14_map *map, u32 x, u32 y, i32 size)
{
	i32 from = size > 0 ? 0    : size+1;
	i32 to   = size > 0 ? size : 1;

	for (i32 i = from; i < to; i++) {
		size_t idx = (y - map->offset.y + i) * map->width + (x - map->offset.x);
		map->tiles[idx] = TILE_SOLID;
	}
}

static void day14_place_horizontal(day14_map *map, u32 x, u32 y, i32 size)
{
	i32 from = size > 0 ? 0    : size+1;
	i32 to   = size > 0 ? size : 1;

	for (i32 i = from; i < to; i++) {
		size_t idx = (y - map->offset.y) * map->width + (x - map->offset.x + i);
		map->tiles[idx] = TILE_SOLID;
	}
}

static day14_map *day14_create_map(day14_line *lines, size_t count, vec2_u32 *sand_spawner)
{
	day14_map *map = malloc(sizeof(day14_map));

	vec2_u32 min_point = *sand_spawner;
	vec2_u32 max_point = *sand_spawner;
	for (int i = 0; i < count; i++) {
		day14_line *line = &lines[i];
		for (int j = 0; j < line->count; j++) {
			vec2_u32 *point = &line->points[j];
			min_point.x = MIN(min_point.x, point->x);
			min_point.y = MIN(min_point.y, point->y);
			max_point.x = MAX(max_point.x, point->x);
			max_point.y = MAX(max_point.y, point->y);
		}
	}

	map->offset = min_point;
	map->width  = max_point.x - min_point.x + 1;
	map->height = max_point.y - min_point.y + 1;
	map->tiles  = calloc(map->width*map->height, sizeof(map_tile));

	for (int i = 0; i < count; i++) {
		day14_line *line = &lines[i];
		for (int j = 1; j < line->count; j++) {
			vec2_u32 *from = &line->points[j-1];
			vec2_u32 *to = &line->points[j];
			if (from->x == to->x) {
				i32 size = to->y - from->y;
				size += SIGN(size);
				day14_place_vertical(map, from->x, from->y, size);
			} else if (from->y == to->y) {
				i32 size = to->x - from->x;
				size += SIGN(size);
				day14_place_horizontal(map, from->x, from->y, size);
			} else {
				abort();
			}
		}
	}

	return map;
}

static bool day14_is_air(day14_map *map, u32 x, u32 y)
{
	if (x < map->offset.x || y < map->offset.y) return true;

	size_t oy = y - map->offset.y;
	if (oy >= map->height) return true;

	size_t ox = x - map->offset.x;
	if (ox >= map->width) return true;

	size_t idx = oy * map->width + ox;
	return map->tiles[idx] == TILE_AIR;
}

static bool day14_is_abyss(day14_map *map, u32 y)
{
	return y >= (map->offset.y + map->height);
}

static bool day14_part1_spawn_sand(day14_map *map, u32 x, u32 y)
{
	while (true) {
		if (day14_is_abyss(map, y)) {
			return false;
		}

		if (day14_is_air(map, x, y+1)) {
			y++;
			continue;
		}
		if (day14_is_air(map, x-1, y+1)) {
			x--;
			y++;
			continue;
		}
		if (day14_is_air(map, x+1, y+1)) {
			x++;
			y++;
			continue;
		}

		size_t idx = (y - map->offset.y) * map->width + (x - map->offset.x);
		map->tiles[idx] = TILE_SAND;
		return true;
	}
}

static bool day14_is_in_bounds(day14_map *map, u32 x, u32 y)
{
	if (x < map->offset.x || y < map->offset.y) return false;
	u32 ox = x - map->offset.x;
	u32 oy = y - map->offset.y;
	return ox < map->width && oy < map->height;
}

static void day14_copy_tiles(
		map_tile *from_map, u32 from_x, u32 from_y, size_t from_width, size_t from_height,
		map_tile *to_map  , u32 to_x  , u32 to_y  , size_t to_width  , size_t to_height)
{
	for (u32 oy = 0; oy < from_height; oy++) {
		for (u32 ox = 0; ox < from_width; ox++) {
			u32 x = from_x + ox;
			u32 y = from_y + oy;
			u32 from_idx = oy * from_width + ox;
			u32 to_idx = (y - to_y) * to_width + (x - to_x);
			to_map[to_idx] = from_map[from_idx];
		}
	}
}

static void day14_ensure_includes(day14_map *map, u32 x, u32 y, bool floor)
{
	if (day14_is_in_bounds(map, x, y)) return;

	u32 new_offset_x = MIN(map->offset.x, x);
	u32 new_offset_y = MIN(map->offset.y, y);
	size_t new_width = MAX(map->offset.x + map->width - 1, x) - new_offset_x + 1;
	size_t new_height = MAX(map->offset.y + map->height - 1, y) - new_offset_y + 1;

	map_tile *new_tiles = calloc(new_width*new_height, sizeof(map_tile));
	day14_copy_tiles(
		map->tiles, map->offset.x, map->offset.y, map->width, map->height,
		new_tiles, new_offset_x, new_offset_y, new_width, new_height
	);

	free(map->tiles);
	map->tiles = new_tiles;
	map->offset.x = new_offset_x;
	map->offset.y = new_offset_y;
	map->width = new_width;
	map->height = new_height;

	if (floor) {
		for (int ox = 0; ox < map->width; ox++) {
			size_t idx = (map->height-1) * map->width + ox;
			map->tiles[idx] = TILE_SOLID;
		}
	}
}

static void day14_map_place_at(day14_map *map, u32 x, u32 y, map_tile tile)
{
	day14_ensure_includes(map, x, y, true);

	size_t idx = (y - map->offset.y) * map->width + (x - map->offset.x);
	map->tiles[idx] = TILE_SAND;
}

static bool day14_part2_spawn_sand(day14_map *map, u32 x, u32 y)
{
	if (!day14_is_air(map, x, y)) return false;

	while (true) {
		if (y < map->offset.y + map->height-2) {
			if (day14_is_air(map, x, y+1)) {
				y++;
				continue;
			}
			if (day14_is_air(map, x-1, y+1)) {
				x--;
				y++;
				continue;
			}
			if (day14_is_air(map, x+1, y+1)) {
				x++;
				y++;
				continue;
			}
		}

		day14_map_place_at(map, x, y, TILE_SAND);
		return true;
	}
}

static void day14_add_floor(day14_map *map)
{
	size_t height = map->height;
	map->height += 2;
	map->tiles  = realloc(map->tiles, map->width*map->height * sizeof(map_tile));

	for (size_t ox = 0; ox < map->width; ox++) {
		size_t idx = height * map->width + ox;
		map->tiles[idx] = TILE_AIR;
	}

	for (size_t ox = 0; ox < map->width; ox++) {
		size_t idx = (height+1) * map->width + ox;
		map->tiles[idx] = TILE_SOLID;
	}
}

static void day14_part1(void *p)
{
	day14_input *input = (day14_input*)p;
	vec2_u32 sand_spawner = { .x = 500, .y = 0 };
	day14_map *map = day14_create_map(input->lines, input->count, &sand_spawner);

	size_t count = 0;
	while (day14_part1_spawn_sand(map, sand_spawner.x, sand_spawner.y)) {
		count++;
	}
	// printf_day14_map(map);
	printf("Answer: %ld\n", count);
}

static void day14_part2(void *p)
{
	day14_input *input = (day14_input*)p;
	vec2_u32 sand_spawner = { .x = 500, .y = 0 };
	day14_map *map = day14_create_map(input->lines, input->count, &sand_spawner);
	day14_add_floor(map);

	size_t count = 0;
	while (day14_part2_spawn_sand(map, sand_spawner.x, sand_spawner.y)) {
		count++;
	}
	// printf_day14_map(map);
	printf("Answer: %ld\n", count);
}

ADD_SOLUTION(14, day14_parse, day14_part1, day14_part2);
