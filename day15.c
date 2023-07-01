#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"
#include "vec2.h"

typedef struct {
	vec2 sensor;
	vec2 beacon;
} day15_reading;

typedef struct {
	day15_reading *readings;
	size_t count;
} day15_readings;

static vec2 day15_parse_point(char *line)
{
	vec2 point;
	point.x = strtol(strstr(line, "x=")+2, NULL, 10);
	point.y = strtol(strstr(line, "y=")+2, NULL, 10);
	return point;
}

static void *day15_parse(char **lines, int line_count)
{
	day15_readings *readings = malloc(sizeof(day15_readings));
	readings->readings = malloc(line_count * sizeof(day15_reading));
	readings->count = line_count;

	for (size_t i = 0; i < line_count; i++) {
		day15_reading *r = &readings->readings[i];
		r->sensor = day15_parse_point(lines[i]);
		r->beacon = day15_parse_point(strchr(lines[i], ':'));
	}

	return readings;
}

static u32 manhattan_dist(vec2 *A, vec2 *B)
{
	return  abs(A->x - B->x) + abs(A->y - B->y);
}

static void print_reading(day15_reading *r)
{
	printf("Sensor{%d,%d}, Beacon{%d,%d}\n", r->sensor.x, r->sensor.y, r->beacon.x, r->beacon.y);
}

static void day15_get_bounds(
		day15_reading *readings, size_t readings_count,
		i32 target_row,
		i32 *left_bounds, i32 *right_bounds,
		size_t *bounds_count
	)
{
	for (size_t i = 0; i < readings_count; i++) {
		day15_reading *r = &readings[i];

		u32 range = manhattan_dist(&r->beacon, &r->sensor);
		u32 dist_to_target = abs(target_row - r->sensor.y);
		if (dist_to_target <= range) {
			u32 margin = range - dist_to_target;

			left_bounds[*bounds_count] = r->sensor.x - margin;
			right_bounds[*bounds_count] = r->sensor.x + margin;

			(*bounds_count)++;
		}
	}
}

static void swap_i32(i32 *A, i32 *B)
{
	i32 C = *A;
	*A = *B;
	*B = C;
}

static void day15_sort_bounds(i32 *left_bounds, i32 *right_bounds, size_t bounds_count)
{
	if (bounds_count <= 1) return;

	for (int i = 0; i < bounds_count - 1; i++) {
		for (int j = i+1; j < bounds_count; j++) {
			if (left_bounds[i] > left_bounds[j]) {
				swap_i32(&left_bounds[i], &left_bounds[j]);
				swap_i32(&right_bounds[i], &right_bounds[j]);
			}
		}
	}
}

static void day15_remove_bound(i32 *left_bounds, i32 *right_bounds, size_t *bounds_count, int k)
{
	for (size_t i = k; i < *bounds_count - 1; i++) {
		left_bounds[i]  = left_bounds[i+1];
		right_bounds[i] = right_bounds[i+1];
	}
	(*bounds_count)--;
}

static void day15_merge_bounds(i32 *left_bounds, i32 *right_bounds, size_t *bounds_count)
{
	if (*bounds_count <= 1) return;

	for (size_t i = 0; i < *bounds_count-1; i++) {
		if (right_bounds[i] >= left_bounds[i+1] && right_bounds[i] >= right_bounds[i+1]) {
			day15_remove_bound(right_bounds, left_bounds, bounds_count, i+1);
			i--;
		} else if (right_bounds[i] >= left_bounds[i+1]) {
			right_bounds[i] = right_bounds[i+1];
			day15_remove_bound(right_bounds, left_bounds, bounds_count, i+1);
			i--;
		}
	}
}

static bool is_vec2_in_bounds(vec2 *point, i32 y, i32 min_x, i32 max_x)
{
	return point->y == y && (min_x <= point->x && point->x <= max_x);
}

static u32 day15_get_findings_count_in_bounds(day15_reading *readings, size_t reading_count, i32 y, i32 min_x, i32 max_x)
{
	u32 count = 0;
	for (int i = 0; i < reading_count; i++) {
		if (is_vec2_in_bounds(&readings->sensor, y, min_x, max_x) || is_vec2_in_bounds(&readings->beacon, y, min_x, max_x)) {
			count++;
		}
	}
	return count;
}

static void day15_part1(void *p)
{
	day15_readings *data = (day15_readings*)p;

	i32 target_row = 2000000;

	i32 *left_bounds = malloc(sizeof(i32) * data->count);
	i32 *right_bounds = malloc(sizeof(i32) * data->count);
	size_t bounds_count = 0;

	day15_get_bounds(data->readings, data->count, target_row, left_bounds, right_bounds, &bounds_count);
	day15_sort_bounds(left_bounds, right_bounds, bounds_count);
	day15_merge_bounds(left_bounds, right_bounds, &bounds_count);

	u32 answer = 0;
	for (size_t i = 0; i < bounds_count; i++) {
		answer += abs(right_bounds[i] - left_bounds[i]);
		answer -= day15_get_findings_count_in_bounds(data->readings, data->count, target_row, left_bounds[i], right_bounds[i]);
	}

	printf("%d\n", answer);
}

static void day15_part2(void *p)
{
	day15_readings *data = (day15_readings*)p;

	i32 *left_bounds = malloc(sizeof(i32) * data->count);
	i32 *right_bounds = malloc(sizeof(i32) * data->count);

	for (i32 y = 0; y < 4000000; y++) {
		size_t bounds_count = 0;

		day15_get_bounds(data->readings, data->count, y, left_bounds, right_bounds, &bounds_count);
		day15_sort_bounds(left_bounds, right_bounds, bounds_count);
		day15_merge_bounds(left_bounds, right_bounds, &bounds_count);
		if (bounds_count > 1) {
			i32 x = right_bounds[0] + 1;
			printf("%lu\n", (u64)x * 4000000 + (u64)y);
			break;
		}
	}
}

ADD_SOLUTION(15, day15_parse, day15_part1, day15_part2);
