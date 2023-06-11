#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "types.h"
#include "aoc.h"

typedef struct day13_packet *day13_packet_ptr;

struct day13_packet {
	bool is_number;
	union {
		u8 number;
		struct {
			day13_packet_ptr list;
			size_t list_size;
		};
	};
};

typedef struct {
	struct day13_packet left;
	struct day13_packet right;
} day13_packet_pair;

typedef struct {
	day13_packet_pair* pairs;
	size_t count;
} day13_packet_pairs;

static bool day13_is_digit(char c)
{
	return '0' <= c && c <= '9';
}

static size_t day13_parse_packet(struct day13_packet *result, char *packet)
{
	size_t bytes_read = 0;

	if (packet[0] == '[') {
		bytes_read++;
		result->is_number = false;
		result->list_size = 0;
		result->list = NULL;
		if (packet[1] != ']') {
			while (true) {
				result->list = realloc(result->list, sizeof(struct day13_packet)*(result->list_size+1));
				bytes_read += day13_parse_packet(&result->list[result->list_size], packet+bytes_read);
				result->list_size++;

				if (packet[bytes_read] == ',') {
					bytes_read++;
					continue;
				}
				if (packet[bytes_read] == ']') break;
				assert(false && "fuck");
			}
		}
		bytes_read++;
	} else if (day13_is_digit(packet[0])) {
		bytes_read++;
		result->is_number = true;
		result->number = (packet[0] - '0');
		for (int i = 1; i < strlen(packet); i++) {
			if (!day13_is_digit(packet[i])) break;
			result->number *= 10;
			result->number += (packet[i] - '0');
			bytes_read++;
		}
	} else {
		assert(false && "Failed to parse packet");
	}

	return bytes_read;
}

static void printf_day13_packet(struct day13_packet *packet)
{
	if (packet->is_number) {
		printf("%d", packet->number);
	} else {
		printf("[");
		if (packet->list_size > 0) {
			for (int i = 0; i < packet->list_size-1; i++) {
				printf_day13_packet(&packet->list[i]);
				printf(",");
			}
			printf_day13_packet(&packet->list[packet->list_size-1]);
		}
		printf("]");
	}
}

static void *day13_parse(char **lines, int line_count)
{
	size_t n = (line_count+1)/3;
	day13_packet_pair *pairs = malloc(sizeof(day13_packet_pair)*n);

	for (int i = 0; i < n; i++)
	{
		day13_parse_packet(&pairs[i].left , lines[3*i+0]);
		day13_parse_packet(&pairs[i].right, lines[3*i+1]);
	}

	day13_packet_pairs *result = malloc(sizeof(day13_packet_pairs));
	result->pairs = pairs;
	result->count = n;
	return result;
}

// Return values:
//  -1 = left is lower
//   0 = equal
//   1 = right is lower
static int day13_compare(struct day13_packet *left, struct day13_packet *right)
{
	if (left->is_number && right->is_number) {
		if (left->number == right->number) {
			return 0;
		} else if (left->number < right->number) {
			return -1;
		} else {
			return 1;
		}
	} else if (!left->is_number && !right->is_number) {
		for (int i = 0; i < MIN(left->list_size, right->list_size); i++) {
			int item_result = day13_compare(&left->list[i], &right->list[i]);
			if (item_result != 0) {
				return item_result;
			}
		}
		if (left->list_size < right->list_size) {
			return -1;
		} else if (left->list_size > right->list_size) {
			return 1;
		}
	} else if (left->is_number && !right->is_number) {
		struct day13_packet packet = {
			.is_number = false,
			.list = left,
			.list_size = 1
		};
		return day13_compare(&packet, right);
	} else if (!left->is_number && right->is_number) {
		struct day13_packet packet = {
			.is_number = false,
			.list = right,
			.list_size = 1
		};
		return day13_compare(left, &packet);
	}
	return 0;
}

static void day13_part1(void *p)
{
	day13_packet_pairs *pairs = (day13_packet_pairs*)p;
	int result = 0;
	for (int i = 0; i < pairs->count; i++) {
		if (day13_compare(&pairs->pairs[i].left, &pairs->pairs[i].right) == -1) {
			result += (i+1);
		}
	}
	printf("Answer: %d\n", result);
}

static int day13_find_packet(struct day13_packet **packets, size_t count, struct day13_packet *target)
{
	for (int i = 0; i < count; i++) {
		if (packets[i] == target) {
			return i;
		}
	}
	return -1;
}

static void day13_swap(struct day13_packet **A, struct day13_packet **B)
{
	struct day13_packet *C = *A;
	*A = *B;
	*B = C;
}

// bubble sort
static void day13_sort_packets(struct day13_packet **packets, size_t count)
{
	for (int i = 0; i < count-1; i++) {
		for (int j = i+1; j < count; j++) {
			int cmp = day13_compare(packets[i], packets[j]);
			if (cmp == 1) {
				day13_swap(&packets[i], &packets[j]);
			}
		}
	}
}

static void day13_part2(void *p)
{
	day13_packet_pairs *pairs = (day13_packet_pairs*)p;

	size_t packet_count = pairs->count*2 + 2;
	struct day13_packet **packets = malloc(sizeof(struct day13_packet*)*packet_count);

	// insert divider [[2]]
	struct day13_packet divider1;
	day13_parse_packet(&divider1, "[[2]]");
	packets[0] = &divider1;

	// insert divider [[6]]
	struct day13_packet divider2;
	day13_parse_packet(&divider2, "[[6]]");
	packets[1] = &divider2;

	// insert packets from `pairs.pairs`
	for (int i = 0; i < pairs->count; i++) {
		packets[2 + i*2+0] = &pairs->pairs[i].left;
		packets[2 + i*2+1] = &pairs->pairs[i].right;
	}

	day13_sort_packets(packets, packet_count);

	int divider1_idx = day13_find_packet(packets, packet_count, &divider1);
	int divider2_idx = day13_find_packet(packets, packet_count, &divider2);
	int answer = (divider1_idx+1) * (divider2_idx+1);
	printf("Answer: %d\n", answer);
}

ADD_SOLUTION(13, day13_parse, day13_part1, day13_part2);
