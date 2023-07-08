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

//  N   T
// W E
//  S   B
//
// Reference image: https://media.printables.com/media/prints/209926/images/1928241_bf5690f2-3397-4519-aa4a-43afc07c0eaa/thumbs/inside/1920x1440/jpg/dice.webp
// Reference model: https://sketchfab.com/3d-models/6-sided-dice-7564358b73614881a1e838a0827f10f8
// Unwrapped faces, based on 6-sided die:
//  N    3
//  T    1
// WSE  542
//  B    6
//
typedef enum {
	FACE_TOP,    // 0
	FACE_EAST,   // 1
	FACE_NORTH,  // 2
	FACE_SOUTH,  // 3
	FACE_WEST,   // 4
	FACE_BOTTOM, // 5
} day22_face;
char *g_day22_face_str[] = {
	[FACE_TOP   ] = "top",
	[FACE_EAST  ] = "east",
	[FACE_NORTH ] = "north",
	[FACE_SOUTH ] = "south",
	[FACE_WEST  ] = "west",
	[FACE_BOTTOM] = "bottom",
};
day22_face g_day22_relface[6][4] = {
	[FACE_TOP] = {
		[FACING_RIGHT] = FACE_EAST,
		[FACING_DOWN ] = FACE_SOUTH,
		[FACING_LEFT ] = FACE_WEST,
		[FACING_UP   ] = FACE_NORTH,
	},
	[FACE_EAST] = {
		[FACING_RIGHT] = FACE_NORTH,
		[FACING_DOWN ] = FACE_BOTTOM,
		[FACING_LEFT ] = FACE_SOUTH,
		[FACING_UP   ] = FACE_TOP,
	},
	[FACE_NORTH] = {
		[FACING_RIGHT] = FACE_EAST,
		[FACING_DOWN ] = FACE_TOP,
		[FACING_LEFT ] = FACE_WEST,
		[FACING_UP   ] = FACE_BOTTOM,
	},
	[FACE_SOUTH] = {
		[FACING_RIGHT] = FACE_EAST,
		[FACING_DOWN ] = FACE_BOTTOM,
		[FACING_LEFT ] = FACE_WEST,
		[FACING_UP   ] = FACE_TOP,
	},
	[FACE_WEST] = {
		[FACING_RIGHT] = FACE_SOUTH,
		[FACING_DOWN ] = FACE_BOTTOM,
		[FACING_LEFT ] = FACE_NORTH,
		[FACING_UP   ] = FACE_TOP,
	},
	[FACE_BOTTOM] = {
		[FACING_RIGHT] = FACE_EAST,
		[FACING_DOWN ] = FACE_NORTH,
		[FACING_LEFT ] = FACE_WEST,
		[FACING_UP   ] = FACE_SOUTH,
	},
};

day22_facing g_day22_relfacing[6][4] = {
	[FACE_TOP] = {
		[FACING_RIGHT] = FACING_DOWN,
		[FACING_DOWN ] = FACING_DOWN,
		[FACING_LEFT ] = FACING_DOWN,
		[FACING_UP   ] = FACING_UP,
	},
	[FACE_EAST] = {
		[FACING_RIGHT] = FACING_LEFT,
		[FACING_DOWN ] = FACING_LEFT,
		[FACING_LEFT ] = FACING_LEFT,
		[FACING_UP   ] = FACING_LEFT,
	},
	[FACE_NORTH] = {
		[FACING_RIGHT] = FACING_LEFT,
		[FACING_DOWN ] = FACING_DOWN,
		[FACING_LEFT ] = FACING_RIGHT,
		[FACING_UP   ] = FACING_UP,
	},
	[FACE_SOUTH] = {
		[FACING_RIGHT] = FACING_RIGHT,
		[FACING_DOWN ] = FACING_DOWN,
		[FACING_LEFT ] = FACING_LEFT,
		[FACING_UP   ] = FACING_UP,
	},
	[FACE_WEST] = {
		[FACING_RIGHT] = FACING_RIGHT,
		[FACING_DOWN ] = FACING_RIGHT,
		[FACING_LEFT ] = FACING_RIGHT,
		[FACING_UP   ] = FACING_RIGHT,
	},
	[FACE_BOTTOM] = {
		[FACING_RIGHT] = FACING_UP,
		[FACING_DOWN ] = FACING_DOWN,
		[FACING_LEFT ] = FACING_UP,
		[FACING_UP   ] = FACING_UP,
	},
};


typedef struct {
	day22_map *map;
	vec2 faces[6];
	u32 face_size;
} day22_cube_map;

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

static bool day22_step(day22_map *map, vec2 *pos, day22_facing *dir)
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
	day22_follow_instructions(map, insts, &pos, &dir, (step_cb)day22_step);

	u32 answer = (pos.y+1) * 1000 + (pos.x+1) * 4 + dir;
	printf("%d\n", answer);
}

static day22_face day22_cube_map_face(day22_cube_map *cube_map, i32 x, i32 y)
{
	x = x / cube_map->face_size * cube_map->face_size;
	y = y / cube_map->face_size * cube_map->face_size;
	for (int i = 0; i < ARRAY_LEN(cube_map->faces); i++) {
		vec2 *face = &cube_map->faces[i];
		if (face->x == x && face->y == y) {
			return i;
		}
	}
	return 9;
}

static void day22_print_cube_map_faces(day22_cube_map *cube_map)
{
	day22_map *map = cube_map->map;
	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			u32 idx = y * map->width + x;
			if (map->tiles[idx] != TILE_VOID) {
				printf("%d", day22_cube_map_face(cube_map, x, y));
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}

static void day22_cube_map_init(day22_cube_map *cube_map, day22_map *map, vec2 *top_face)
{
	cube_map->map = map;

	u32 surface_area = 0;
	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			u32 idx = y * map->width + x;
			surface_area += (map->tiles[idx] != TILE_VOID);
		}
	}

	u32 face_size = sqrt((float)surface_area/6);
	assert(face_size != 0);

	cube_map->face_size = face_size;


	if (face_size == 4) { // Example case
		cube_map->faces[FACE_TOP] = *top_face;

		cube_map->faces[FACE_SOUTH] = cube_map->faces[FACE_TOP];
		cube_map->faces[FACE_SOUTH].y += 4;

		cube_map->faces[FACE_WEST] = cube_map->faces[FACE_SOUTH];
		cube_map->faces[FACE_WEST].x -= face_size;

		cube_map->faces[FACE_NORTH] = cube_map->faces[FACE_WEST];
		cube_map->faces[FACE_NORTH].x -= face_size;

		cube_map->faces[FACE_BOTTOM] = cube_map->faces[FACE_SOUTH];
		cube_map->faces[FACE_BOTTOM].y += face_size;

		cube_map->faces[FACE_EAST] = cube_map->faces[FACE_BOTTOM];
		cube_map->faces[FACE_EAST].x += face_size;
	} else { // User case
		cube_map->faces[FACE_TOP] = *top_face;

		cube_map->faces[FACE_EAST] = cube_map->faces[FACE_TOP];
		cube_map->faces[FACE_EAST].x += face_size;

		cube_map->faces[FACE_SOUTH] = cube_map->faces[FACE_TOP];
		cube_map->faces[FACE_SOUTH].y += face_size;

		cube_map->faces[FACE_BOTTOM] = cube_map->faces[FACE_SOUTH];
		cube_map->faces[FACE_BOTTOM].y += face_size;

		cube_map->faces[FACE_WEST] = cube_map->faces[FACE_BOTTOM];
		cube_map->faces[FACE_WEST].x -= face_size;

		cube_map->faces[FACE_NORTH] = cube_map->faces[FACE_WEST];
		cube_map->faces[FACE_NORTH].y += face_size;
	}

	// TODO: To hell with this, too complicated for now. Stitching cube faces
	// https://www.youtube.com/watch?v=qWgLdNFYDDo
	/*
	bool found_faces[6] = { 0 };
	day22_face stack_faces[6];
	day22_facing stack_incoming_facing[6];
	vec2 stack_positions[6];
	int stack_size;
	static_assert(ARRAY_LEN(stack_faces) == ARRAY_LEN(stack_positions), "stack size is inconsistent");

	stack_positions[0] = *top_face;
	stack_faces[0] = FACE_TOP;
	stack_incoming_facing[0] = FACING_DOWN;
	found_faces[FACE_TOP] = true;
	stack_size = 1;

	while (stack_size > 0) {
		vec2 pos = stack_positions[stack_size-1];
		day22_face face = stack_faces[stack_size-1];
		day22_facing initial_facing = stack_incoming_facing[stack_size-1];
		stack_size--;

		cube_map->faces[face] = pos;

		day22_facing new_facings[] = {
			initial_facing,
			day22_turn_left(initial_facing),
			day22_turn_right(initial_facing)
		};
		for (int i = 0; i < ARRAY_LEN(new_facings); i++) {
			day22_facing new_facing = new_facings[i];
			vec2 *dir = &g_day22_dirs[new_facing];
			// if (face == FACE_NORTH) {
			// 	dir = &g_day22_dirs[day22_turn_180(new_facing)];
			// }

			i32 x = pos.x + face_size * dir->x;
			i32 y = pos.y + face_size * dir->y;
			if (day22_in_bounds(map, x, y) && day22_get_tile(map, x, y) != TILE_VOID) {
				// day22_facing rel_new_facing = (4 + initial_facing - new_facing) % 4;
				day22_face new_face = g_day22_relface[face][new_facing];

				printf("found face %s(%d) from %s(%d) by going %s(%d)\n", g_day22_face_str[new_face], new_face, g_day22_face_str[face], face, g_day22_facing_str[new_facing], new_facing);
				if (found_faces[new_face]) {
					printf("already found\n");
					continue;
				}
				stack_positions[stack_size].x = x;
				stack_positions[stack_size].y = y;
				stack_faces[stack_size] = new_face;
				stack_incoming_facing[stack_size] = new_facing;
				found_faces[new_face] = true;

				stack_size++;
				assert(stack_size < ARRAY_LEN(stack_positions));
			}
		}
	}
	*/
}

static void day22_map_edge(day22_cube_map *cube_map, day22_face from_face, day22_facing from_facing, day22_face to_face, day22_facing to_facing, vec2 *pos)
{
	// TODO:
}

static bool day22_cube_step(day22_cube_map *cube_map, vec2 *pos, day22_facing *dir)
{
	day22_facing next_dir = *dir;
	vec2 next_pos = *pos;

	next_pos.x += g_day22_dirs[next_dir].x;
	next_pos.y += g_day22_dirs[next_dir].y;

	if (day22_get_tile(cube_map->map, next_pos.x, next_pos.y) == TILE_VOID) {
		day22_face face = day22_cube_map_face(cube_map, pos->x, pos->y);
		day22_face new_face = g_day22_relface[face][*dir];
		next_dir = g_day22_relfacing[face][*dir];
		day22_map_edge(cube_map, face, *dir, new_face, next_dir, &next_pos);
	}

	if (day22_get_tile(cube_map->map, next_pos.x, next_pos.y) == TILE_EMPTY) {
		pos->x = next_pos.x;
		pos->y = next_pos.y;
		*dir = next_dir;
		return true;
	} else {
		return false;
	}
}

static void assert_face_table_is_correct()
{
	for (int face = 0; face < ARRAY_LEN(g_day22_relface); face++) {
		for (int facing = 0; facing < ARRAY_LEN(g_day22_relface[face]); facing++) {
			day22_face new_face = g_day22_relface[face][facing];
			day22_facing new_facing = g_day22_relfacing[face][facing];

			int new_facing_180 = day22_turn_180(new_facing);
			assert(face == g_day22_relface[new_face][new_facing_180]);
		}
	}
}

static void day22_part2(void *p)
{
	assert_face_table_is_correct();

	day22_data *data = (day22_data*)p;

	day22_map *map = &data->map;
	day22_instructions *insts = &data->instructions;

	vec2 start = day22_starting_pos(map);

	day22_cube_map cube_map = { 0 };
	day22_cube_map_init(&cube_map, map, &start);

	day22_print_cube_map_faces(&cube_map);

	vec2 pos = start;
	day22_facing dir = FACING_RIGHT;
	// day22_follow_instructions(&cube_map, insts, &pos, &dir, (step_cb)day22_cube_step);

	// u32 answer = (pos.y+1) * 1000 + (pos.x+1) * 4 + dir;
	// printf("%d\n", answer);
}

ADD_SOLUTION(22, day22_parse, day22_part1, day22_part2);
