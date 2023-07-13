#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "vec2.h"
#include "aoc.h"

#define DAY24_QUEUE_CAPACITY (1024 * 256)

typedef enum {
	DIR24_UP,
	DIR24_DOWN,
	DIR24_LEFT,
	DIR24_RIGHT,
} day24_direction;
char g_day24_direction[] = {
	[DIR24_UP   ] = '^',
	[DIR24_DOWN ] = 'v',
	[DIR24_LEFT ] = '<',
	[DIR24_RIGHT] = '>'
};

typedef struct {
	vec2 pos;
	day24_direction direction;
} day24_blizzard;

typedef struct {
	day24_blizzard *blizzards;
	u32 blizzard_count;
	u32 width;
	u32 height;
} day24_data;

typedef struct {
	i8 x[DAY24_QUEUE_CAPACITY];
	i8 y[DAY24_QUEUE_CAPACITY];
	u16 time[DAY24_QUEUE_CAPACITY];

	i32 front;
	i32 rear;
} day24_queue;

static day24_direction day24_parse_direction(char symbol)
{
	switch(symbol) {
		case '>': return DIR24_RIGHT;
		case '<': return DIR24_LEFT;
		case 'v': return DIR24_DOWN;
		case '^': return DIR24_UP;
		default: assert(false && "Invalid direction symbol");
	}
}

static void* day24_parse(char** lines, int line_count)
{
	day24_data *data = malloc(sizeof(day24_data));
	data->width = strlen(lines[0]);
	data->height = line_count;
	data->blizzards = malloc(data->width * data->height * sizeof(day24_blizzard));
	data->blizzard_count = 0;

	for (int y = 1; y < data->height-1; y++) {
		for (int x = 1; x < data->width-1; x++) {
			char symbol = lines[y][x];
			if (symbol == '.') continue;
			size_t blizzard_idx = data->blizzard_count;

			data->blizzards[blizzard_idx].pos.x = x;
			data->blizzards[blizzard_idx].pos.y = y;
			data->blizzards[blizzard_idx].direction = day24_parse_direction(symbol);
			data->blizzard_count++;
		}
	}

	return data;
}

static void day24_step(u16 *from_map, u16 *to_map, u32 width, u32 height, day24_blizzard *blizzards, u32 blizzard_count)
{
	memcpy(to_map, from_map, sizeof(u16) * width * height);

	for (int i = 0; i < blizzard_count; i++) {
		day24_blizzard *blizz = &blizzards[i];
		vec2 *pos = &blizz->pos;

		to_map[pos->y * width + pos->x]--;

		if (blizz->direction == DIR24_UP) {
			pos->y--;
			if (pos->y == 0) {
				pos->y = height-2;
			}
		} else if (blizz->direction == DIR24_DOWN) {
			pos->y++;
			if (pos->y == height-1) {
				pos->y = 1;
			}
		} else if (blizz->direction == DIR24_LEFT) {
			pos->x--;
			if (pos->x == 0) {
				pos->x = width-2;
			}
		} else { // if (blizz->direction == DIR24_RIGHT) {
			pos->x++;
			if (pos->x == width-1) {
				pos->x = 1;
			}
		}

		to_map[pos->y * width + pos->x]++;
	}
}

static day24_blizzard *day24_get_blizzard_at(day24_blizzard *blizzards, u32 count, i32 x, i32 y)
{
	for (int i = 0; i < count; i++) {
		if (blizzards[i].pos.x == x && blizzards[i].pos.y == y) {
			return &blizzards[i];
		}
	}
	return NULL;
}

static void day24_map_print(u16 *map, u32 width, u32 height)
{
	printf("#.");
	for (int x = 2; x < width; x++) {
		printf("#");
	}
	printf("\n");

	for (int y = 1; y < height-1; y++) {
		printf("#");
		for (int x = 1; x < width-1; x++) {
			u32 idx = y * width + x;
			if (map[idx] == 0) {
				printf(".");
			} else {
				printf("%d", map[idx]);
			}
		}
		printf("#\n");
	}

	for (int x = 0; x < width-2; x++) {
		printf("#");
	}
	printf(".#\n");
}

static u32 day24_gcd(u32 a, u32 b) {
	u32 R;
	while ((a % b) != 0) {
		R = a % b;
		a = b;
		b = R;
	}
	return b;
}

static u32 day24_lcm(u32 a, u32 b) {
	return a * b / day24_gcd(a, b);
}

static void day24_generate_maps(u16 **maps, u32 count, day24_blizzard *blizzards, u32 blizzard_count, u32 width, u32 height)
{
	memset(maps[0], 0, width * height * sizeof(u16));
	for (int i = 0; i < blizzard_count; i++) {
		vec2 *pos = &blizzards[i].pos;
		u32 idx = pos->y * width + pos->x;
		maps[0][idx]++;
	}

	day24_blizzard moving_blizzards[blizzard_count];
	memcpy(moving_blizzards, blizzards, sizeof(day24_blizzard) * blizzard_count);

	for (int i = 1; i < count; i++) {
		day24_step(maps[i-1], maps[i], width, height, moving_blizzards, blizzard_count);
	}
}

static void day24_queue_push(day24_queue *queue, i8 x, i8 y, u16 time)
{
	assert(queue->rear < DAY24_QUEUE_CAPACITY);

	if (queue->front == -1) {
		queue->front = 0;
	}

	queue->rear++;
	queue->x[queue->rear] = x;
	queue->y[queue->rear] = y;
	queue->time[queue->rear] = time;
}


static void day24_queue_init(day24_queue *queue)
{
	queue->rear = -1;
	queue->front = -1;
}

static bool day24_queue_is_empty(day24_queue *queue)
{
	return queue->rear == -1;
}

static void day24_queue_pop(day24_queue *queue, i8 *x, i8 *y, u16 *time)
{
	assert(!day24_queue_is_empty(queue));

	*x = queue->x[queue->front];
	*y = queue->y[queue->front];
	*time = queue->time[queue->front];
	queue->front++;
	if (queue->front > queue->rear) {
		queue->front = -1;
		queue->rear = -1;
	}
}

static bool day24_between(i8 value, i8 min, i8 max)
{
	return min <= value && value <= max;
}

static u16 day24_bfs(u16 **maps, u32 map_count, u32 width, u32 height, vec2 *start, vec2 *goal, u32 start_time)
{
	u32 seen_size = 67108864; // 67108864 = 2^26
	bool *seen = calloc(seen_size, sizeof(bool));

	day24_queue queue;
	day24_queue_init(&queue);
	day24_queue_push(&queue, start->x, start->y, start_time);

	while (!day24_queue_is_empty(&queue)) {
		i8 x, y;
		u16 time;
		day24_queue_pop(&queue, &x, &y, &time);

		time++;

		u32 map_idx = time % map_count;
		u16 *map = maps[map_idx];

		vec2 offsets[] = { VEC2(1, 0), VEC2(-1, 0), VEC2(0, 1), VEC2(0, -1), VEC2(0, 0) };
		for (int i = 0; i < ARRAY_LEN(offsets); i++) {
			i8 new_x = x + offsets[i].x;
			i8 new_y = y + offsets[i].y;

			if (vec2_eq(goal, new_x, new_y)) {
				return time;
			}

			bool is_in_bounds = day24_between(new_x, 1, width-2) && day24_between(new_y, 1, height-2);
			if (!is_in_bounds && !vec2_eq(start, new_x, new_y)) continue;

			u32 tile_idx = new_y * width + new_x;
			if (map[tile_idx] > 0) continue;

			u32 seen_key = (map_idx << 16) | (new_x << 8) | (new_y);
			assert(seen_key < seen_size);
			if (seen[seen_key]) continue;
			seen[seen_key] = true;

			day24_queue_push(&queue, new_x, new_y, time);
		}
	}

	return 0;
}

static void day24_part1(void *p)
{
	day24_data *data = (day24_data*)p;

	u32 maps_count = day24_lcm(data->width-2, data->height-2);
	u16 *maps[maps_count];
	for (int i = 0; i < maps_count; i++) {
		maps[i] = malloc(data->width * data->height * sizeof(u16));
	}
	day24_generate_maps(maps, maps_count, data->blizzards, data->blizzard_count, data->width, data->height);

	vec2 start = { 1, 0 };
	vec2 goal  = { data->width-2, data->height-1 };
	printf("%d\n", day24_bfs(maps, maps_count, data->width, data->height, &start, &goal, 0));
}

static void day24_part2(void *p)
{
	day24_data *data = (day24_data*)p;

	u32 maps_count = day24_lcm(data->width-2, data->height-2);
	u16 *maps[maps_count];
	for (int i = 0; i < maps_count; i++) {
		maps[i] = malloc(data->width * data->height * sizeof(u16));
	}
	day24_generate_maps(maps, maps_count, data->blizzards, data->blizzard_count, data->width, data->height);

	vec2 start = { 1, 0 };
	vec2 goal  = { data->width-2, data->height-1 };

	u32 time1 = day24_bfs(maps, maps_count, data->width, data->height, &start, &goal , 0);
	u32 time2 = day24_bfs(maps, maps_count, data->width, data->height, &goal , &start, time1);
	u32 time3 = day24_bfs(maps, maps_count, data->width, data->height, &start, &goal , time2);
	printf("%d\n", time3);
}

ADD_SOLUTION(24, day24_parse, day24_part1, day24_part2);
