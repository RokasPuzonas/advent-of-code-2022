#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "types.h"
#include "vec2.h"
#include "aoc.h"

#define CHUNK_BITS 4
#define CHUNK_SIZE (1 << CHUNK_BITS)

typedef struct {
	vec2 *elves;
	u32 count;
} day23_data;

typedef bool day23_chunk[CHUNK_SIZE * CHUNK_SIZE];

typedef struct {
	day23_chunk *chunks;
	i32 ox, oy;
	i32 width, height;
} day23_world;

static void* day23_parse(char** lines, int line_count)
{
	day23_data *data = malloc(sizeof(day23_data));
	data->count = 0;

	for (int y = 0; y < line_count; y++) {
		for (int x = 0; x < strlen(lines[y]); x++) {
			data->count += (lines[y][x] == '#');
		}
	}

	data->elves = malloc(sizeof(vec2) * data->count);

	int i = 0;
	for (int y = 0; y < line_count; y++) {
		for (int x = 0; x < strlen(lines[y]); x++) {
			if (lines[y][x] == '#') {
				data->elves[i].x = x;
				data->elves[i].y = y;
				i++;
			}
		}
	}

	return data;
}

static void day23_world_init(day23_world *world, u32 width, u32 height)
{
	world->ox = 0;
	world->oy = 0;
	world->width  = width  / CHUNK_SIZE + 1;
	world->height = height / CHUNK_SIZE + 1;
	world->chunks = calloc(world->width * world->height, sizeof(day23_chunk));
}

static bool day23_is_between(i32 value, i32 min, i32 max)
{
	return min <= value && value <= max;
}

static vec2 day23_get_chunk_pos(day23_world *world, i32 x, i32 y)
{
	vec2 chunk_pos = {
		.x = (x >> CHUNK_BITS) - world->ox,
		.y = (y >> CHUNK_BITS) - world->oy
	};
	return chunk_pos;
}

static day23_chunk *day23_get_chunk(day23_world *world, i32 x, i32 y)
{
	vec2 chunk_pos = day23_get_chunk_pos(world, x, y);
	i32 chunk_x = chunk_pos.x;
	i32 chunk_y = chunk_pos.y;

	if (day23_is_between(chunk_x, 0, world->width-1) && day23_is_between(chunk_y, 0, world->height-1)) {
		u32 idx = chunk_y * world->width + chunk_x;
		return &world->chunks[idx];
	}

	return NULL;
}

static u32 day23_get_tile_offset_idx(day23_world *world, i32 x, i32 y)
{
	// u32 mask = (1 << CHUNK_BITS) - 1;
	u32 chunk_x = (x + CHUNK_SIZE) % CHUNK_SIZE;
	u32 chunk_y = (y + CHUNK_SIZE) % CHUNK_SIZE;
	return chunk_y * CHUNK_SIZE + chunk_x;
}

static void day23_world_set(day23_world *world, i32 x, i32 y, bool value)
{
	day23_chunk *chunk = day23_get_chunk(world, x, y);
	if (chunk == NULL) {
		i32 chunk_x = x >> CHUNK_BITS;
		i32 chunk_y = y >> CHUNK_BITS;

		i32 min_x = MIN(chunk_x, world->ox);
		i32 max_x = MAX(chunk_x, world->ox + (i32)world->width - 1);
		i32 min_y = MIN(chunk_y, world->oy);
		i32 max_y = MAX(chunk_y, world->oy + (i32)world->height - 1);

		i32 new_width = max_x - min_x + 1;
		i32 new_height = max_y - min_y + 1;

		i32 ox = abs(min_x - world->ox);
		i32 oy = abs(min_y - world->oy);

		day23_chunk *new_chunks = calloc(new_height * new_width, sizeof(day23_chunk));
		for (int y = 0; y < world->height; y++) {
			for (int x = 0; x < world->width; x++) {
				u32 from_idx = y * world->width + x;
				u32 to_idx = (y + oy) * new_width + (x + ox);
				memcpy(new_chunks[to_idx], world->chunks[from_idx], sizeof(day23_chunk));
			}
		}

		free(world->chunks);
		world->ox = min_x;
		world->oy = min_y;
		world->width = new_width;
		world->height = new_height;
		world->chunks = new_chunks;

		chunk = day23_get_chunk(world, x, y);
	}

	u32 tile_idx = day23_get_tile_offset_idx(world, x, y);
	(*chunk)[tile_idx] = value;
}

static bool day23_world_get(day23_world *world, i32 x, i32 y)
{
	day23_chunk *chunk = day23_get_chunk(world, x, y);
	if (chunk == NULL) return false;

	u32 tile_idx = day23_get_tile_offset_idx(world, x, y);
	return (*chunk)[tile_idx];
}

static void day23_world_print(day23_world *world)
{
	for (int y = 0; y < world->height * CHUNK_SIZE; y++) {
		for (int x = 0; x < (world->width * CHUNK_SIZE); x++) {
			bool value = day23_world_get(world, x + world->ox * CHUNK_SIZE, y + world->oy * CHUNK_SIZE);
			printf(value ? "#" : ".");
		}
		printf("\n");
	}
}

static bool day23_are_tiles_empty(day23_world *world, vec2 *pos, vec2 *offsets, u32 offset_count)
{
	for (int i = 0; i < offset_count; i++) {
		if (day23_world_get(world, pos->x + offsets[i].x, pos->y + offsets[i].y)) {
			return false;
		}
	}
	return true;
}

static bool day23_are_neighours_empty(day23_world *world, vec2 *pos)
{
	vec2 neighbours[] = {
		VEC2(-1, -1), VEC2( 0, -1), VEC2( 1, -1),
		VEC2(-1,  0)              , VEC2( 1,  0),
		VEC2(-1,  1), VEC2( 0,  1), VEC2( 1,  1),
	};

	return day23_are_tiles_empty(world, pos, neighbours, ARRAY_LEN(neighbours));
}

static vec2 day23_propose(day23_world *world, vec2 *elf, u32 step_idx)
{
	// NOTE: The second second position in each direction MUST be the middle one.
	vec2 check_positions[4][3] = {
		[0] = { VEC2(-1, -1), VEC2( 0, -1), VEC2( 1, -1) }, // North
		[1] = { VEC2(-1,  1), VEC2( 0,  1), VEC2( 1,  1) }, // South
		[2] = { VEC2(-1, -1), VEC2(-1,  0), VEC2(-1,  1) }, // West
		[3] = { VEC2( 1, -1), VEC2( 1,  0), VEC2( 1,  1) }  // East
	};
	for (int i = 0; i < 4; i++) {
		vec2 *offsets = check_positions[(step_idx + i) % 4];
		if (day23_are_tiles_empty(world, elf, offsets, ARRAY_LEN(check_positions[0]))) {
			vec2 proposed = VEC2(elf->x + offsets[1].x, elf->y + offsets[1].y);
			return proposed;
		}
	}

	return *elf;
}

static bool day23_step(day23_world *world, vec2 *elves, u32 elves_count, u32 step_idx)
{
	bool no_moves = true;

	vec2 proposed_positions[elves_count];
	for (int i = 0; i < elves_count; i++) {
		if (day23_are_neighours_empty(world, &elves[i])) {
			proposed_positions[i] = elves[i];
		} else {
			proposed_positions[i] = day23_propose(world, &elves[i], step_idx);
			no_moves = false;
		}
	}

	bool overlapping[elves_count];
	memset(overlapping, false, sizeof(bool) * elves_count);

	for (int i = 0; i < elves_count - 1; i++) {
		for (int j = i + 1; j < elves_count; j++) {
			vec2 *pos1 = &proposed_positions[i];
			vec2 *pos2 = &proposed_positions[j];
			if (pos1->x == pos2->x && pos1->y == pos2->y) {
				overlapping[i] = true;
				overlapping[j] = true;
			}
		}
	}

	for (int i = 0; i < elves_count; i++) {
		if (overlapping[i]) continue;
		day23_world_set(world, elves[i].x, elves[i].y, false);
		day23_world_set(world, proposed_positions[i].x, proposed_positions[i].y, true);
		elves[i] = proposed_positions[i];
	}

	return no_moves;
}

static void day23_setup_world(day23_world *world, vec2 *elves, u32 count)
{
	u32 initial_width = 0;
	u32 initial_height = 0;
	for (int i = 0; i < count; i++) {
		initial_width = MAX(elves[i].x+1, initial_width);
		initial_height = MAX(elves[i].y+1, initial_height);
		elves[i] = elves[i];
	}

	day23_world_init(world, initial_width, initial_height);
	for (int i = 0; i < count; i++) {
		day23_world_set(world, elves[i].x, elves[i].y, true);
	}
}

static void day23_part1(void *p)
{
	day23_data *data = (day23_data*)p;

	vec2 elves[data->count];
	memcpy(elves, data->elves, sizeof(vec2) * data->count);

	day23_world world;
	day23_setup_world(&world, data->elves, data->count);

	for (int i = 0; i < 10; i++) {
		day23_step(&world, elves, data->count, i);
	}

	i32 min_x = elves[0].x;
	i32 min_y = elves[0].y;
	i32 max_x = elves[0].x;
	i32 max_y = elves[0].y;
	for (int i = 1; i < data->count; i++) {
		min_x = MIN(min_x, elves[i].x);
		min_y = MIN(min_y, elves[i].y);
		max_x = MAX(max_x, elves[i].x);
		max_y = MAX(max_y, elves[i].y);
	}

	u32 answer = 0;
	for (int y = min_y; y <= max_y; y++) {
		for (int x = min_x; x <= max_x; x++) {
			answer += !day23_world_get(&world, x, y);
		}
	}

	printf("%d\n", answer);
}

static void day23_part2(void *p)
{
	day23_data *data = (day23_data*)p;

	vec2 elves[data->count];
	memcpy(elves, data->elves, sizeof(vec2) * data->count);

	day23_world world;
	day23_setup_world(&world, data->elves, data->count);

	u32 steps = 0;
	while (true) {
		bool no_moves = day23_step(&world, elves, data->count, steps);
		steps++;
		if (no_moves) break;
	}

	printf("%d\n", steps);
}

ADD_SOLUTION(23, day23_parse, day23_part1, day23_part2);
