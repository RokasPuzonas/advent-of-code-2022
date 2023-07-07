#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/param.h>

#include "types.h"
#include "aoc.h"

typedef enum {
	RESOURCE_ORE,
	RESOURCE_CLAY,
	RESOURCE_OBSIDIAN,
	RESOURCE_GEODE,
	__RESOURCE_COUNT
} day19_resource;
char *g_day19_resource_str[] = {
	[RESOURCE_ORE] = "ore",
	[RESOURCE_CLAY] = "clay",
	[RESOURCE_OBSIDIAN] = "obsidian",
	[RESOURCE_GEODE] = "geode"
};

typedef u8 day19_robot_cost[__RESOURCE_COUNT];

typedef struct {
	day19_robot_cost robot_costs[__RESOURCE_COUNT];
} day19_blueprint;

typedef struct {
	day19_blueprint *blueprints;
	size_t count;
} day19_data;

static void day19_parse_count_with_resource(char *str, u8 *robot_cost)
{
	char *after_count;
	u8 count = strtol(str, &after_count, 10);
	char *resource_str = after_count+1;

	day19_resource resource = -1;
	for (int i = 0; i < ARRAY_LEN(g_day19_resource_str); i++) {
		if (strncmp(resource_str, g_day19_resource_str[i], strlen(g_day19_resource_str[i])) == 0) {
			resource = i;
		}
	}
	assert(resource != -1);

	robot_cost[resource] = count;
}

static void day19_parse_robot_cost(char* sentence, u8 *robot_cost)
{
	char *costs_start = strstr(sentence, "costs")+6;
	char *and = strstr(costs_start, "and");
	if (and) {
		day19_parse_count_with_resource(costs_start, robot_cost);
		day19_parse_count_with_resource(and+4, robot_cost);
	} else {
		day19_parse_count_with_resource(costs_start, robot_cost);
	}
}

static void* day19_parse(char** lines, int line_count)
{
	day19_data *data = malloc(sizeof(day19_data));
	data->blueprints = calloc(line_count, sizeof(day19_blueprint));
	data->count = line_count;

	for (int i = 0; i < line_count; i++) {
		day19_blueprint *bp = &data->blueprints[i];
		char *line = strchr(lines[i], ':')+2; // Skip `Blueprint %d:` part at start

		day19_resource resource = RESOURCE_ORE;
		char *tok_save = NULL;
		for (char *tok = strtok_r(line, ".", &tok_save); tok; tok = strtok_r(NULL, ".", &tok_save)) {
			day19_parse_robot_cost(tok, bp->robot_costs[resource]);
			resource++;
			if (resource == __RESOURCE_COUNT) break;
		}
	}

	return data;
}

static void day19_print_blueprint(day19_blueprint *bp)
{
	printf("Blueprint:\n");
	for (int i = 0; i < __RESOURCE_COUNT; i++) {
		printf("  Each %s robot costs", g_day19_resource_str[i]);
		bool resource_printed = false;
		for (int j = 0; j < __RESOURCE_COUNT; j++) {
			u8 count = bp->robot_costs[i][j];
			if (count == 0) continue;

			if (resource_printed) printf(" and");
			printf(" %d %s", bp->robot_costs[i][j], g_day19_resource_str[j]);
			resource_printed = true;
		}
		printf(".\n");
	}
}

static bool day19_can_buy(day19_robot_cost cost, u32 storage[__RESOURCE_COUNT])
{
	for (int i = 0; i < __RESOURCE_COUNT; i++) {
		if (cost[i] > storage[i]) {
			return false;
		}
	}
	return true;
}

static void day19_subtract_cost(day19_robot_cost cost, u32 storage[__RESOURCE_COUNT])
{
	for (int i = 0; i < __RESOURCE_COUNT; i++) {
		storage[i] -= cost[i];
	}
}

static u32 div_ceil(u32 a, u32 b)
{
	return a / b + (a % b > 0);
}

typedef struct {
	u16 *bucket_values;
	u32 *bucket_keys;
	u32 capacity;
	u32 count;
} day19_cache;

static u32 day19_mix_and_mul(u32 key)
{
	// Robert Jenkins' 32 bit Mix Function
	key += (key << 12);
	key ^= (key >> 22);
	key += (key << 4);
	key ^= (key >> 9);
	key += (key << 10);
	key ^= (key >> 2);
	key += (key << 7);
	key ^= (key >> 12);

	// Knuth's Multiplicative Method
	key = (key >> 3) * 2654435761;

	return key;
}

static u32 day19_hash(
	u32 time_left,
	u32 robots[__RESOURCE_COUNT],
	u32 storage[__RESOURCE_COUNT])
{
	u32 key = day19_mix_and_mul(time_left);
	for (int i = 0; i < __RESOURCE_COUNT; i++) {
		key ^= day19_mix_and_mul(robots[i] << 16 | storage[i]);
	}
	return key;
}
static i16 day19_cache_get(
	day19_cache *cache,
	u32 time_left,
	u32 robots[__RESOURCE_COUNT],
	u32 storage[__RESOURCE_COUNT])
{
	u32 key = day19_hash(time_left, robots, storage);
	assert(key != 0 && "Cache key can't be zero");
	for (int i = 0; i < cache->count; i++) {
		u32 idx = (key + i) % cache->capacity;
		if (cache->bucket_keys[idx] == key) {
			return cache->bucket_values[idx];
		} else if (cache->bucket_keys[idx] == 0) {
			break;
		}
	}
	return -1;
}

static void day19_cache_put_with_key(day19_cache *cache, u32 key, u16 value)
{
	assert(cache->count < cache->capacity && "Cache reached capacity");

	for (int i = 0; i < cache->capacity; i++) {
		u32 idx = (key + i) % cache->capacity;
		if (cache->bucket_keys[idx] == key) {
			assert(false && "Already exists");
		} else if (cache->bucket_keys[idx] == 0) {
			cache->bucket_values[idx] = value;
			cache->bucket_keys[idx] = key;
			cache->count++;
			return;
		}
	}

	assert(false && "Failed to put to cache");
}

static void day19_cache_put(
	day19_cache *cache,
	u32 time_left,
	u32 robots[__RESOURCE_COUNT],
	u32 storage[__RESOURCE_COUNT],
	u16 max_geodes)
{
	u32 key = day19_hash(time_left, robots, storage);
	assert(key != 0 && "Cache key can't be zero");
	day19_cache_put_with_key(cache, key, max_geodes);
}

static void day19_cache_init(day19_cache *cache, u32 initial_capacity)
{
	cache->count = 0;
	cache->capacity = initial_capacity;
	cache->bucket_keys = calloc(initial_capacity, sizeof(u32));
	cache->bucket_values = calloc(initial_capacity, sizeof(u32));
}

static void day19_cache_free(day19_cache *cache)
{
	free(cache->bucket_values);
	free(cache->bucket_keys);
	cache->bucket_keys = NULL;
	cache->bucket_values = NULL;
}

static u16 day19_max_geodes_dfs(
		day19_blueprint *bp,
		u32 time_left,
		u32 max_spend[__RESOURCE_COUNT],
		u32 robots[__RESOURCE_COUNT],
		u32 storage[__RESOURCE_COUNT],
		day19_cache *cache)
{
	if (time_left == 0) return storage[RESOURCE_GEODE];

	i32 cached_value = day19_cache_get(cache, time_left, robots, storage);
	if (cached_value != -1) return cached_value;

	u16 max_geodes = storage[RESOURCE_GEODE] + robots[RESOURCE_GEODE] * time_left;

	u32 storage_copy[__RESOURCE_COUNT];

	for (int type = __RESOURCE_COUNT-1; type >= 0; type--) {
		if (robots[type] >= max_spend[type] && type != RESOURCE_GEODE) continue;

		bool can_be_built = true;
		u32 time_until_robot = 0;
		for (int cost_type = 0; cost_type < __RESOURCE_COUNT; cost_type++) {
			u8 cost = bp->robot_costs[type][cost_type];
			if (cost == 0) continue;

			if (robots[cost_type] == 0 && storage[cost_type] < cost) {
				can_be_built = false;
				break;
			}

			if (storage[cost_type] <= cost) {
				u8 missing = cost - storage[cost_type];
				u32 time_to_resource = div_ceil(missing, robots[cost_type]);
				time_until_robot = MAX(time_until_robot, time_to_resource);
			}
		}

		if (!can_be_built) continue;

		time_until_robot++;
		if (time_until_robot > time_left) continue;

		for (int i = 0; i < __RESOURCE_COUNT; i++) {
			storage_copy[i] = storage[i];
			storage_copy[i] += robots[i] * time_until_robot;
			storage_copy[i] -= bp->robot_costs[type][i];
		}
		for (int i = 0; i < __RESOURCE_COUNT-1; i++) {
			storage_copy[i] = MIN(storage_copy[i], max_spend[i] * time_left);
		}
		robots[type]++;
		max_geodes = MAX(max_geodes, day19_max_geodes_dfs(bp, time_left - time_until_robot, max_spend, robots, storage_copy, cache));
		robots[type]--;
	}

	day19_cache_put(cache, time_left, robots, storage, max_geodes);

	return max_geodes;
}

static u32 day19_max_geodes(day19_blueprint *bp, u32 time_limit)
{
	u32 max_spend[__RESOURCE_COUNT] = { 0 };
	for (int resource = 0; resource < __RESOURCE_COUNT; resource++) {
		for (int robot = 0; robot < __RESOURCE_COUNT; robot++) {
			max_spend[resource] = MAX(max_spend[resource], bp->robot_costs[robot][resource]);
		}
	}
	u32 robots[__RESOURCE_COUNT] = { 0 };
	robots[RESOURCE_ORE] = 1;

	u32 storage[__RESOURCE_COUNT] = { 0 };

	day19_cache cache = { 0 };
	day19_cache_init(&cache, 1024*1024*4); // 4 MiB
	u32 answer = day19_max_geodes_dfs(bp, time_limit, max_spend, robots, storage, &cache);
	day19_cache_free(&cache);

	return answer;
}

static void day19_part1(void *p)
{
	day19_data *data = (day19_data*)p;

	u32 time_limit = 24;

	u32 answer = 0;
	for (int i = 0; i < data->count; i++) {
		u32 max_geodes = day19_max_geodes(&data->blueprints[i], time_limit);
		answer += (i+1) * max_geodes;
	}
	printf("%d\n", answer);
}

static void day19_part2(void *p)
{
	day19_data *data = (day19_data*)p;

	u32 time_limit = 32;

	u32 answer = 1;
	answer *= day19_max_geodes(&data->blueprints[0], time_limit);
	answer *= day19_max_geodes(&data->blueprints[1], time_limit);
	answer *= day19_max_geodes(&data->blueprints[2], time_limit);
	printf("%d\n", answer);
}

ADD_SOLUTION(19, day19_parse, day19_part1, day19_part2);
