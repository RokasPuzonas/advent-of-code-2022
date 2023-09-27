#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/param.h>
#include <math.h>

#include "vec2.h"
#include "types.h"
#include "aoc.h"

typedef enum {
	TILE_VOID,
	TILE_EMPTY,
	TILE_WALL,
} day22_tile;
char g_day22_tiles[] = {
	[TILE_VOID ] = ' ',
	[TILE_EMPTY] = '.',
	[TILE_WALL ] = '#'
};

typedef enum {
	FACING_RIGHT,
	FACING_DOWN,
	FACING_LEFT,
	FACING_UP,
} day22_facing;
char *g_day22_facing_str[] = {
	[FACING_RIGHT] = "right",
	[FACING_DOWN ] = "down",
	[FACING_LEFT ] = "left",
	[FACING_UP   ] = "up",
};
vec2 g_day22_dirs[] = {
	[FACING_RIGHT] = { .x =  1, .y =  0},
	[FACING_DOWN ] = { .x =  0, .y =  1},
	[FACING_LEFT ] = { .x = -1, .y =  0},
	[FACING_UP   ] = { .x =  0, .y = -1},
};

typedef enum {
	INST_WALK,
	INST_LEFT,
	INST_RIGHT,
} day22_instruction_op;

typedef struct {
	day22_tile *tiles;
	u32 width;
	u32 height;
} day22_map;


typedef struct {
	day22_instruction_op op;
	u32 amount;
} day22_instruction;

typedef struct {
	day22_instruction *list;
	u32 count;
} day22_instructions;

typedef struct {
	day22_map map;
	day22_instructions instructions;
} day22_data;

typedef bool (*step_cb)(void*, vec2*, day22_facing*);

static day22_tile day22_tile_from_char(char c)
{
	switch(c) {
	case ' ': return TILE_VOID;
	case '.': return TILE_EMPTY;
	case '#': return TILE_WALL;
	default: assert(false && "Unknonwn tile symbol");
	}
}

static void* day22_parse(char** lines, int line_count)
{
	day22_data *data = malloc(sizeof(day22_data));
	data->map.height = line_count - 2;
	data->map.width = strlen(lines[0]);
	for (int i = 1; i < data->map.height; i++) {
		data->map.width = MAX(data->map.width, strlen(lines[i]));
	}

	u32 tiles_size = sizeof(day22_tile) * data->map.width * data->map.height;
	data->map.tiles = malloc(tiles_size);
	memset(data->map.tiles, TILE_VOID, tiles_size);

	for (int y = 0; y < data->map.height; y++) {
		for (int x = 0; x < MIN(strlen(lines[y]), data->map.width); x++) {
			int idx = y * data->map.width + x;
			data->map.tiles[idx] = day22_tile_from_char(lines[y][x]);
		}
	}

	u32 capacity = 1024*4;
	data->instructions.list = calloc(capacity, sizeof(day22_instruction));
	data->instructions.count = 0;
	char *instructions = lines[line_count-1];
	while (instructions[0]) {
		assert(data->instructions.count+1 < capacity);

		day22_instruction *inst = &data->instructions.list[data->instructions.count];
		if (instructions[0] == 'R') {
			inst->op = INST_RIGHT;
			instructions++;
		} else if (instructions[0] == 'L') {
			inst->op = INST_LEFT;
			instructions++;
		} else {
			inst->op = INST_WALK;
			inst->amount = strtol(instructions, &instructions, 10);
		}

		data->instructions.count++;
	}

	return data;
}

static void day22_print_map(day22_map *map)
{
	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			u32 idx = y * map->width + x;
			day22_tile tile = map->tiles[idx];
			printf("%c", g_day22_tiles[tile]);
		}
		printf("\n");
	}
}

static void day22_print_instructions(day22_instructions *insts)
{
	for (int i = 0; i < insts->count; i++) {
		day22_instruction *inst = &insts->list[i];
		switch (inst->op) {
		case INST_WALK:
			printf("%d", inst->amount);
			break;
		case INST_LEFT:
			printf("L");
			break;
		case INST_RIGHT:
			printf("R");
			break;
        }
	}
	printf("\n");
}

static vec2 day22_starting_pos(day22_map *map)
{
	for (int x = 0; x < map->width; x++) {
		if (map->tiles[x] == TILE_EMPTY) {
			vec2 pos = { .x = x, .y = 0 };
			return pos;
		}
	}
	assert(false && "Starting pos not found");
}

static day22_facing day22_turn_left(day22_facing facing)
{
	return (facing - 1 + 4) % 4;
}

static day22_facing day22_turn_right(day22_facing facing)
{
	return (facing + 1) % 4;
}

static day22_facing day22_turn_180(day22_facing facing)
{
	return (facing + 2) % 4;
}

static void day22_wrap_around(day22_map *map, vec2 *pos)
{
	if (pos->x >= (i32)map->width) {
		pos->x -= map->width;
	} else if (pos->x < 0) {
		pos->x += map->width;
	} else if (pos->y >= (i32)map->height) {
		pos->y -= map->height;
	} else if (pos->y < 0) {
		pos->y += map->height;
	}
}

static bool day22_in_bounds(day22_map *map, i32 x, i32 y)
{
	return (0 <= x && x < map->width) && (0 <= y && y < map->height);
}

static day22_tile day22_get_tile(day22_map *map, i32 x, i32 y)
{
	if (!day22_in_bounds(map, x, y)) {
		return TILE_VOID;
	}
	u32 idx = y * map->width + x;
	return map->tiles[idx];
}

static bool day22_step_part1(day22_map *map, vec2 *pos, day22_facing *dir)
{
	vec2 next_pos = *pos;
	vec2 *step = &g_day22_dirs[*dir];

	next_pos.x += step->x;
	next_pos.y += step->y;
	day22_wrap_around(map, &next_pos);

	while (day22_get_tile(map, next_pos.x, next_pos.y) == TILE_VOID) {
		next_pos.x += step->x;
		next_pos.y += step->y;
		day22_wrap_around(map, &next_pos);
	}

	if (day22_get_tile(map, next_pos.x, next_pos.y) == TILE_EMPTY) {
		pos->x = next_pos.x;
		pos->y = next_pos.y;
		return true;
	} else {
		return false;
	}
}

static void day22_walk(void *map, vec2 *pos, day22_facing *dir, u32 amount, step_cb do_step)
{
	for (int i = 0; i < amount; i++) {
		if (!do_step(map, pos, dir)) {
			break;
		}
	}
}

static void day22_follow_instructions(void *map, day22_instructions *insts, vec2 *pos, day22_facing *dir, step_cb do_step)
{
	for (int i = 0; i < insts->count; i++) {
		day22_instruction *inst = &insts->list[i];
		switch(inst->op) {
		case INST_WALK:
			day22_walk(map, pos, dir, inst->amount, do_step);
			break;
		case INST_LEFT:
			*dir = day22_turn_left(*dir);
			break;
		case INST_RIGHT:
			*dir = day22_turn_right(*dir);
			break;
		}
	}
}

static void day22_part1(void *p)
{
	day22_data *data = (day22_data*)p;

	day22_map *map = &data->map;
	day22_instructions *insts = &data->instructions;

	vec2 pos = day22_starting_pos(map);
	day22_facing dir = FACING_RIGHT;
	day22_follow_instructions(map, insts, &pos, &dir, (step_cb)day22_step_part1);

	u32 answer = (pos.y+1) * 1000 + (pos.x+1) * 4 + dir;
	printf("%d\n", answer);
}

static u32 day22_get_cube_size(day22_map *map)
{
	u32 surface_area = 0;
	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			day22_tile tile = map->tiles[y * map->width + x];
			surface_area += (tile != TILE_VOID);
		}
	}

	assert(surface_area % 6 == 0);
	return sqrtf((float)surface_area/6);
}

static bool day22_has_void_neighbour(day22_map *map, int x, int y)
{
	for (int i = 0; i < ARRAY_LEN(g_day22_dirs); i++) {
		vec2 dir = g_day22_dirs[i];
		if (day22_get_tile(map, x+dir.x, y+dir.y) == TILE_VOID) {
			return true;
		}
	}
	return false;
}

static bool day22_is_going_along_edge(day22_map *map, int from_x, int from_y, int dir_x, int dir_y, int steps)
{
	for (int i = 1; i < steps; i++) {
		int probe_x = from_x + dir_x * i;
		int probe_y = from_y + dir_y * i;
		if (!day22_has_void_neighbour(map, probe_x, probe_y)) {
			return false;
		}
	}
	return true;
}

static bool day22_contains_vec2(vec2_u32 *list, int size, int x, int y) {
	for (int i = 0; i < size; i++) {
		if (list[i].x == x && list[i].y == y) {
			return true;
		}
	}
	return false;
}

static vec2 day22_apply_facing(int x, int y, day22_facing facing, int distance)
{
	vec2 *dir = &g_day22_dirs[facing];
	vec2 result = {
		x + dir->x * distance,
		y + dir->y * distance,
	};
	return result;
}

typedef struct { vec2_u32 start; vec2_u32 end; } day22_edge;

static day22_edge day22_flip_edge(day22_edge edge)
{
	day22_edge flipped_edge = { .start = edge.end, .end = edge.start };
	return flipped_edge;
}

static vec2 day22_get_edge_dir(day22_edge edge) {
	vec2 dir = {
		SIGN((int)edge.end.x - (int)edge.start.x),
		SIGN((int)edge.end.y - (int)edge.start.y),
	};
	return dir;
}

static bool day22_is_on_edge(day22_edge edge, int x, int y) {
	if (edge.start.x == edge.end.x) {
		int min_y = MIN(edge.start.y, edge.end.y);
		int max_y = MAX(edge.start.y, edge.end.y);
		return x == edge.start.x && (min_y <= y && y <= max_y);
	} else if (edge.start.y == edge.end.y) {
		int min_x = MIN(edge.start.x, edge.end.x);
		int max_x = MAX(edge.start.x, edge.end.x);
		return y == edge.start.y && (min_x <= x && x <= max_x);
	} else {
		assert(false && "unreachable");
	}
}

static bool day22_is_closer_to_end(day22_edge edge, int x, int y) {
	if (edge.start.x == edge.end.x) {
		return abs((int)edge.end.y - y) < abs((int)edge.start.y - y);
	} else if (edge.start.y == edge.end.y) {
		return abs((int)edge.end.x - x) < abs((int)edge.start.x - x);
	} else {
		assert(false && "unreachable");
	}
}

static void day22_print_map_with_edges(day22_map *map, day22_edge *from_edges, day22_edge *to_edges, int edge_pair_count)
{
	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			bool found = false;
			for (int i = 0; i < edge_pair_count; i++) {
				day22_edge edge = {};
				if (day22_is_on_edge(from_edges[i], x, y)) {
					edge = from_edges[i];
				} else if (day22_is_on_edge(to_edges[i], x, y)) {
					edge = to_edges[i];
				} else {
					continue;
				}

				printf("%c", 'a' + i - (day22_is_closer_to_end(edge, x, y) ? 32 : 0));
				found = true;
				break;
			}
			if (found) continue;

			u32 idx = y * map->width + x;
			day22_tile tile = map->tiles[idx];
			printf("%c", g_day22_tiles[tile]);
		}
		printf("\n");
	}
}

static int inc_cw(int cw, int size) {
	return (cw+1) % size;
}

static int inc_ccw(int ccw, int size) {
	return (ccw-1+size) % size;
}

static void day22_get_edge_transitions(day22_map *map, day22_edge *from_edges, day22_edge *to_edges, int *edge_pair_count)
{
	u32 cube_size = day22_get_cube_size(map);

	vec2_u32 face_cursor = { -1, -1 };
	for (int x = 0; x < map->width; x += cube_size) {
		if (map->tiles[x] != TILE_VOID) {
			face_cursor.x = x;
			face_cursor.y = 0;
			break;
		}
	}
	assert(face_cursor.x != -1 && face_cursor.y != -1);

	// Step 1 - find faces
	vec2_u32 found_faces[6];
	int face_count = 0;
	{
		vec2 first_face = day22_starting_pos(map);
		face_count = 1;
		found_faces[0].x = first_face.x;
		found_faces[0].y = first_face.y;

		vec2_u32 face_stack[6];
		int stack_size = 1;

		face_stack[0].x = first_face.x;
		face_stack[0].y = first_face.y;

		while (stack_size > 0) {
			vec2_u32 cur_face = face_stack[stack_size-1];
			stack_size--;

			for (int i = 0; i < ARRAY_LEN(g_day22_dirs); i++) {
				vec2 *dir = &g_day22_dirs[i];
				vec2 next_face = {
					cur_face.x + dir->x * cube_size,
					cur_face.y + dir->y * cube_size,
				};
				if (day22_get_tile(map, next_face.x, next_face.y) == TILE_VOID) continue;
				if (day22_contains_vec2(found_faces, face_count, next_face.x, next_face.y)) continue;

				found_faces[face_count].x = next_face.x;
				found_faces[face_count].y = next_face.y;
				face_count++;

				face_stack[stack_size].x = next_face.x;
				face_stack[stack_size].y = next_face.y;
				stack_size++;
			}
		}
	}
	assert(face_count == 6);

	// Step 2 - find all edges
	int max_edges = 16;
	day22_edge edges[max_edges];
	int edge_count = 0;

	{
		for (int i = 0; i < face_count; i++) {
			// printf("face (%d, %d)\n", found_faces[i].x, found_faces[i].y);
			for (int facing = 0; facing < ARRAY_LEN(g_day22_dirs); facing++) {
				vec2 dir = g_day22_dirs[facing];
				if (day22_get_tile(map, found_faces[i].x + dir.x * cube_size, found_faces[i].y + dir.y * cube_size) == TILE_VOID) {
					assert(edge_count < max_edges-1);
					vec2_u32 edge_start = found_faces[i];
					vec2_u32 edge_end = found_faces[i];
					switch(facing) {
					case FACING_UP:
						edge_end.x += cube_size-1;
						break;
					case FACING_DOWN:
						edge_start.x += cube_size-1;
						edge_start.y += cube_size-1;
						edge_end.y += cube_size-1;
						break;
					case FACING_LEFT:
						edge_start.y += cube_size-1;
						break;
					case FACING_RIGHT:
						edge_start.x += cube_size-1;
						edge_end.x += cube_size-1;
						edge_end.y += cube_size-1;
						break;
					}

					// printf("edge (%d, %d) -> (%d, %d)\n", edge_start.x, edge_start.y, edge_end.x, edge_end.y);

					edges[edge_count].start.x = edge_start.x;
					edges[edge_count].start.y = edge_start.y;
					edges[edge_count].end.x   = edge_end.x;
					edges[edge_count].end.y   = edge_end.y;
					edge_count++;
				}
			}
		}
	}

	// Step 3 - stich edges into a clockwise loop
	int loop_edges[edge_count];
	int loop_length = 0;
	{
		loop_edges[0] = 0;
		loop_length++;

		while (loop_length < edge_count) {
			day22_edge last_edge = edges[loop_edges[loop_length-1]];
			// printf("loop_length: %d (%d, %d) -> (%d, %d)\n", loop_length, last_edge.start.x, last_edge.start.y, last_edge.end.x, last_edge.end.y);
			vec2 edge_dir = day22_get_edge_dir(last_edge);
			// printf("dir: (%d, %d)\n", edge_dir.x, edge_dir.y);

			for (int i = 0; i < edge_count; i++) {
				bool is_edge_used = false;
				for (int j = 0; j < loop_length; j++) {
					if (loop_edges[j] == i) {
						is_edge_used = true;
						break;
					}
				}
				if (is_edge_used) continue;

				// printf("check: %d\n", i);
				vec2 check_offsets[] = {
					{ 0, 0 },
					edge_dir,
					{ edge_dir.x + -edge_dir.y, edge_dir.y +  edge_dir.x },
					{ edge_dir.x +  edge_dir.y, edge_dir.y + -edge_dir.x },
				};
				bool edge_connects = false;
				for (int j = 0; j < ARRAY_LEN(check_offsets); j++) {
					if (last_edge.end.x + check_offsets[j].x == edges[i].start.x &&
						last_edge.end.y + check_offsets[j].y == edges[i].start.y) {
						edge_connects = true;
						break;
					}
				}

				if (edge_connects) {
					// printf("next: (%d, %d) -> (%d, %d)\n", edges[i].start.x, edges[i].start.y, edges[i].end.x, edges[i].end.y);
					loop_edges[loop_length] = i;
					loop_length++;
					break;
				}
			}
		}
	}

	// Step 4 - find inner corners
	int inner_corners[edge_count];
	int inner_corner_count = 0;

	for (int i = 0; i < loop_length; i++) {
		day22_edge edge = edges[loop_edges[i]];
		day22_edge next_edge = edges[loop_edges[(i + 1) %  loop_length]];

		int dx = edge.end.x - next_edge.start.x;
		int dy = edge.end.y - next_edge.start.y;
		if (dx != 0 && dy != 0) {
			inner_corners[inner_corner_count] = i;
			inner_corner_count++;
		}
	}

	#define APPEND_EDGE_PAIR(edge_a, edge_b) \
		from_edges[*edge_pair_count] = edge_a;                                                               \
		to_edges[*edge_pair_count] = edge_b;                                                                 \
		(*edge_pair_count)++;

	*edge_pair_count = 0;

	// Step 5 - stich edges to form cube
	for (int i = 0; i < inner_corner_count; i++) {
		int cw_idx  = (inner_corners[i]+1) % loop_length;
		int ccw_idx = inner_corners[i];

		while (true) {
			APPEND_EDGE_PAIR(
				edges[loop_edges[cw_idx]],
				day22_flip_edge(edges[loop_edges[ccw_idx]])
			);

			int next_cw_idx = inc_cw(cw_idx, loop_length);
			int next_ccw_idx = inc_ccw(ccw_idx, loop_length);

			bool is_cw_straight = vec2_eq2(
					day22_get_edge_dir(edges[loop_edges[cw_idx]]),
					day22_get_edge_dir(edges[loop_edges[next_cw_idx]])
				);
			bool is_ccw_straight = vec2_eq2(
					day22_get_edge_dir(edges[loop_edges[ccw_idx]]),
					day22_get_edge_dir(edges[loop_edges[next_ccw_idx]])
				);

			if ((!is_cw_straight && !is_ccw_straight) || (is_cw_straight && is_ccw_straight)) {
				break;
			}

			cw_idx = next_cw_idx;
			ccw_idx = next_ccw_idx;
		}
	}

	// Extra step - Edge case when not all edges were mapped, when searching from corners
	if ((*edge_pair_count)*2 < edge_count) {
		int cw_idx  = (inner_corners[0]+1) % loop_length;
		while (true) {
			bool is_point_accounted = false;
			vec2_u32 point = edges[loop_edges[cw_idx]].start;
			for (int i = 0; i < *edge_pair_count; i++) {
				vec2_u32 from_point = from_edges[loop_edges[i]].start;
				vec2_u32 to_point = to_edges[loop_edges[i]].end;
				if (vec2_u32_eq2(from_point, point) || vec2_u32_eq2(to_point, point)) {
					is_point_accounted = true;
					break;
				}
			}
			if (!is_point_accounted) break;

			cw_idx = inc_cw(cw_idx, loop_length);
		}

		int ccw_idx = inner_corners[0];
		while (true) {
			bool is_point_accounted = false;
			vec2_u32 point = edges[loop_edges[ccw_idx]].start;
			for (int i = 0; i < *edge_pair_count; i++) {
				vec2_u32 from_point = from_edges[loop_edges[i]].start;
				vec2_u32 to_point = to_edges[loop_edges[i]].end;
				if (vec2_u32_eq2(from_point, point) || vec2_u32_eq2(to_point, point)) {
					is_point_accounted = true;
					break;
				}
			}
			if (!is_point_accounted) break;

			ccw_idx = inc_ccw(ccw_idx, loop_length);
		}

		while ((*edge_pair_count)*2 < edge_count) {
			APPEND_EDGE_PAIR(
				edges[loop_edges[cw_idx]],
				day22_flip_edge(edges[loop_edges[ccw_idx]])
			);

			cw_idx = inc_cw(cw_idx, loop_length);
			ccw_idx = inc_ccw(ccw_idx, loop_length);
		}
	}
}

typedef struct {
	day22_map *map;
	day22_edge from_edges[16];
	day22_edge to_edges[16];
	int edge_pair_count;
} map_with_edges;

// Assumes that both directions are one of the cardinals
static bool do_dirs_cross(vec2 dir1, vec2 dir2) {
	return (dir1.x != dir2.x && (dir1.x == 0 || dir2.x == 0)) ||
			(dir1.y != dir2.y && (dir1.y == 0 || dir2.y == 0));
}

static day22_facing dir_to_facing(vec2 dir) {
	for (int facing = 0; facing < ARRAY_LEN(g_day22_dirs); facing++) {
		if (g_day22_dirs[facing].x == dir.x && g_day22_dirs[facing].y == dir.y) {
			return facing;
		}
	}
	assert(false && "unreachable");
}

static bool day22_step_part2(map_with_edges *map_with_edges, vec2 *pos, day22_facing *dir)
{
	vec2 next_pos = *pos;
	day22_facing next_facing = *dir;
	vec2 *step = &g_day22_dirs[*dir];

	next_pos.x += step->x;
	next_pos.y += step->y;
	if (day22_get_tile(map_with_edges->map, next_pos.x, next_pos.y) == TILE_VOID) {
		bool found_match = false;
		day22_edge src_edge;
		day22_edge dst_edge;

		for (int i = 0; i < map_with_edges->edge_pair_count; i++) {
			day22_edge from_edge = map_with_edges->from_edges[i];
			day22_edge to_edge = map_with_edges->to_edges[i];

			if (day22_is_on_edge(from_edge, pos->x, pos->y)) {
				if (do_dirs_cross(day22_get_edge_dir(from_edge), *step)) {
					src_edge = from_edge;
					dst_edge = to_edge;
					found_match = true;
					// printf("use %c\n", 'a' + i);
					break;
				}
			} else if (day22_is_on_edge(to_edge, pos->x, pos->y)) {
				if (do_dirs_cross(day22_get_edge_dir(to_edge), *step)) {
					src_edge = to_edge;
					dst_edge = from_edge;
					found_match = true;
					// printf("use %c\n", 'a' + i);
					break;
				}
			}
		}

		if (found_match) {
			int dist_along_src = -1;
			if (src_edge.start.x == src_edge.end.x) {
				dist_along_src = abs((int)src_edge.start.y - pos->y);
			} else if (src_edge.start.y == src_edge.end.y) {
				dist_along_src = abs((int)src_edge.start.x - pos->x);
			} else {
				assert(false && "unreachable");
			}

			vec2 dst_dir = day22_get_edge_dir(dst_edge);
			next_pos.x = dst_edge.start.x + dst_dir.x * dist_along_src;
			next_pos.y = dst_edge.start.y + dst_dir.y * dist_along_src;

			vec2 possible_dirs[] = {
				{ -abs(dst_dir.y), -abs(dst_dir.x) },
				{ abs(dst_dir.y), abs(dst_dir.x) }
			};
			for (int i = 0; i < ARRAY_LEN(possible_dirs); i++) {
				if (day22_get_tile(map_with_edges->map, next_pos.x + possible_dirs[i].x, next_pos.y + possible_dirs[i].y) != TILE_VOID) {
					// printf("(%d,%d)\n", possible_dirs[i].x, possible_dirs[i].y);
					next_facing = dir_to_facing(possible_dirs[i]);
					break;
				}
			}

			// printf("(%d,%d) - (%d,%d) => (%d,%d) - (%d,%d)\n",
			// 	src_edge.start.x, src_edge.start.y, src_edge.end.x, src_edge.end.y,
			// 	dst_edge.start.x, dst_edge.start.y, dst_edge.end.x, dst_edge.end.y
			// );
		}
	}

	if (day22_get_tile(map_with_edges->map, next_pos.x, next_pos.y) == TILE_EMPTY) {
		pos->x = next_pos.x;
		pos->y = next_pos.y;
		*dir = next_facing;
		return true;
	} else {
		return false;
	}
}

// Thank god for this video: https://www.youtube.com/watch?v=qWgLdNFYDDo
// Could not have done it without this.
static void day22_part2(void *p)
{
	day22_data *data = (day22_data*)p;

	day22_map *map = &data->map;
	day22_instructions *insts = &data->instructions;

	map_with_edges custom_map = {
		.map = map,
		.edge_pair_count = 0
	};
	day22_get_edge_transitions(map, custom_map.from_edges, custom_map.to_edges, &custom_map.edge_pair_count);
	// day22_print_map_with_edges(map, custom_map.from_edges, custom_map.to_edges, custom_map.edge_pair_count);

	vec2 pos = day22_starting_pos(map);
	day22_facing dir = FACING_RIGHT;
	day22_follow_instructions(&custom_map, insts, &pos, &dir, (step_cb)day22_step_part2);

	u32 answer = (pos.y+1) * 1000 + (pos.x+1) * 4 + dir;
	printf("%d\n", answer);
}

ADD_SOLUTION(22, day22_parse, day22_part1, day22_part2);
