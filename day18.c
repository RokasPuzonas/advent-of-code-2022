#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/param.h>

#include "types.h"
#include "aoc.h"

#define DAY18_WORLD_SIZE 32768 // 2^15

i8 g_day18_ox[] = { 1, -1, 0,  0, 0,  0 };
i8 g_day18_oy[] = { 0,  0, 1, -1, 0,  0 };
i8 g_day18_oz[] = { 0,  0, 0,  0, 1, -1 };
static_assert(ARRAY_LEN(g_day18_ox) == ARRAY_LEN(g_day18_oy) && ARRAY_LEN(g_day18_oy) == ARRAY_LEN(g_day18_oz), "Number of elements between ox, oy, oz don't match");

typedef struct {
	u8 x, y, z;
} day18_droplet;

typedef struct {
	day18_droplet *droplets;
	size_t count;
} day18_data;

static void* day18_parse(char** lines, int line_count)
{
	day18_data *data = malloc(sizeof(day18_data));
	data->count = line_count;
	data->droplets = malloc(sizeof(day18_droplet)*line_count);

	for (int i = 0; i < line_count; i++) {
		char *comma1 = strchr(lines[i], ',');
		char *comma2 = strchr(comma1+1, ',');

		day18_droplet *droplet = &data->droplets[i];
		droplet->x = strtol(lines[i], NULL, 10);
		droplet->y = strtol(comma1+1, NULL, 10);
		droplet->z = strtol(comma2+1, NULL, 10);
	}

	return data;
}

// Assumes that x, y, z each are at most 5 bits
static u16 day18_get_key(u8 x, u8 y, u8 z)
{
	return (z & 0b11111) << 10 | (y & 0b11111) << 5 | (x & 0b11111) << 0;
}

static bool day18_is_droplet(bool world[], i8 x, i8 y, i8 z)
{
	if (x < 0 || y < 0 || z < 0) return false;
	if (x > 32 || y > 32 || z > 32) return false;

	u16 key = day18_get_key(x, y, z);
	return world[key];
}

static void day18_place_droplets(bool world[], day18_droplet *droplets, size_t count, u16 ox, u16 oy, u16 oz)
{
	// Ensure that all droplet coordinates don't go over 5 bits
	for (int i = 0; i < count; i++) {
		day18_droplet *droplet = &droplets[i];
		assert(droplet->x + ox < 32);
		assert(droplet->y + oy < 32);
		assert(droplet->z + oz < 32);
	}

	for (int i = 0; i < count; i++) {
		day18_droplet *droplet = &droplets[i];
		u16 key = day18_get_key(droplet->x + ox, droplet->y + oy, droplet->z + oz);
		world[key] = true;
	}
}

static void day18_part1(void *p)
{
	day18_data *data = (day18_data*)p;

	bool world[DAY18_WORLD_SIZE] = { false };
	day18_place_droplets(world, data->droplets, data->count, 0, 0, 0);


	u32 area = 0;
	for (int i = 0; i < data->count; i++) {
		day18_droplet *droplet = &data->droplets[i];

		for (int j = 0; j < ARRAY_LEN(g_day18_ox); j++) {
			i8 x = droplet->x + g_day18_ox[j];
			i8 y = droplet->y + g_day18_oy[j];
			i8 z = droplet->z + g_day18_oz[j];
			area += !day18_is_droplet(world, x, y, z);
		}
	}
	printf("%d\n", area);
}

static void day18_part2(void *p)
{
	day18_data *data = (day18_data*)p;

	bool world[DAY18_WORLD_SIZE] = { false };
	day18_place_droplets(world, data->droplets, data->count, 1, 1, 1);

	u32 area = 0;

	bool visited[DAY18_WORLD_SIZE] = { false };
	u32 stack_capacity = DAY18_WORLD_SIZE;
	u8 stack_x[stack_capacity];
	u8 stack_y[stack_capacity];
	u8 stack_z[stack_capacity];
	u32 stack_size = 0;

	stack_x[0] = 29;
	stack_y[0] = 29;
	stack_z[0] = 29;
	visited[day18_get_key(31, 31, 31)] = true;
	stack_size = 1;

	while (stack_size > 0) {
		u8 x = stack_x[stack_size-1];
		u8 y = stack_y[stack_size-1];
		u8 z = stack_z[stack_size-1];
		stack_size--;

		for (int j = 0; j < ARRAY_LEN(g_day18_ox); j++) {
			i8 new_x = x + g_day18_ox[j];
			i8 new_y = y + g_day18_oy[j];
			i8 new_z = z + g_day18_oz[j];
			if (new_x < 0 || new_y < 0 || new_z < 0) continue;
			if (new_x > 32 || new_y > 32 || new_z > 32) continue;

			u16 key = day18_get_key(new_x, new_y, new_z);

			if (day18_is_droplet(world, new_x, new_y, new_z)) {
				area++;
			} else {
				if (visited[key]) continue;
				visited[key] = true;

				assert(stack_size < stack_capacity);
				stack_x[stack_size] = new_x;
				stack_y[stack_size] = new_y;
				stack_z[stack_size] = new_z;
				stack_size++;
			}
		}
	}

	printf("%d\n", area);
}

ADD_SOLUTION(18, day18_parse, day18_part1, day18_part2);
