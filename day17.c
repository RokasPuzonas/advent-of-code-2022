#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"
#include "types.h"
#include "vec2.h"

#define DAY17_BOARD_WIDTH 7

typedef enum {
	DIR_LEFT,
	DIR_RIGHT,
} day17_direction;

typedef struct {
	day17_direction *dirs;
	size_t count;
} day17_gusts;

typedef struct {
	vec2 blocks[5];
	size_t count;
	size_t width;
	size_t height;
} day17_piece;

day17_piece g_day17_pieces[] = {
	// ####
	{
		.blocks = {
			{ .x = 0, .y = 0 },
			{ .x = 1, .y = 0 },
			{ .x = 2, .y = 0 },
			{ .x = 3, .y = 0 },
		},
		.count = 4,
		.width = 4, .height = 1,
	},

	// .#.
	// ###
	// .#.
	{
		.blocks = {
			{ .x = 1, .y = 0 },
			{ .x = 0, .y = 1 },
			{ .x = 1, .y = 1 },
			{ .x = 2, .y = 1 },
			{ .x = 1, .y = 2 },
		},
		.count = 5,
		.width = 3, .height = 3,
	},

	// ..#
	// ..#
	// ###
	{
		.blocks = {
			{ .x = 0, .y = 0 },
			{ .x = 1, .y = 0 },
			{ .x = 2, .y = 0 },
			{ .x = 2, .y = 1 },
			{ .x = 2, .y = 2 },
		},
		.count = 5,
		.width = 3, .height = 3,
	},

	// #
	// #
	// #
	// #
	{
		.blocks = {
			{ .x = 0, .y = 0 },
			{ .x = 0, .y = 1 },
			{ .x = 0, .y = 2 },
			{ .x = 0, .y = 3 },
		},
		.count = 4,
		.width = 1, .height = 4,
	},

	// ##
	// ##
	{
		.blocks = {
			{ .x = 0, .y = 0 },
			{ .x = 1, .y = 0 },
			{ .x = 0, .y = 1 },
			{ .x = 1, .y = 1 },
		},
		.count = 4,
		.width = 2, .height = 2,
	},
};

static void* day17_parse(char** lines, int line_count)
{
	day17_gusts *data = malloc(sizeof(day17_gusts));
	data->count = strlen(lines[0]);
	data->dirs = malloc(sizeof(day17_direction) * data->count);
	for (int i = 0; i < data->count; i++) {
		data->dirs[i] = lines[0][i] == '>' ? DIR_RIGHT : DIR_LEFT;
	}

	return data;
}

static void day17_print_board(bool board[][DAY17_BOARD_WIDTH], u32 height, day17_piece *falling_piece, vec2 *falling_pos)
{
	for (int i = height; i >= 0; i--) {
		printf("|");
		for (int j = 0; j < DAY17_BOARD_WIDTH; j++) {
			bool is_falling_piece = false;
			if (falling_piece) {
				for (int k = 0; k < falling_piece->count; k++) {
					vec2 *block = &falling_piece->blocks[k];
					if (block->x + falling_pos->x == j && block->y + falling_pos->y == i) {
						is_falling_piece = true;
						break;
					}
				}
			}

			if (is_falling_piece) {
				printf("@");
			} else {
				printf(board[i][j] ? "#" : ".");
			}
		}
		printf("|\n");
	}
	printf("+-------+\n");
}

static bool day17_is_solid(bool board[][DAY17_BOARD_WIDTH], i32 x, i32 y)
{
	if (y < 0 || x < 0 || x > DAY17_BOARD_WIDTH-1) return true;
	return board[y][x];
}

static bool day17_can_move_down(bool board[][DAY17_BOARD_WIDTH], day17_piece *piece, vec2 *pos)
{
	for (int i = 0; i < piece->count; i++) {
		i32 x = piece->blocks[i].x + pos->x;
		i32 y = piece->blocks[i].y + pos->y;
		if (day17_is_solid(board, x, y-1)) {
			return false;
		}
	}
	return true;
}

static bool day17_can_move_left(bool board[][DAY17_BOARD_WIDTH], day17_piece *piece, vec2 *pos)
{
	if (pos->x == 0) return false;
	for (int i = 0; i < piece->count; i++) {
		i32 x = piece->blocks[i].x + pos->x;
		i32 y = piece->blocks[i].y + pos->y;
		if (day17_is_solid(board, x-1, y)) {
			return false;
		}
	}
	return true;
}

static bool day17_can_move_right(bool board[][DAY17_BOARD_WIDTH], day17_piece *piece, vec2 *pos)
{
	if (pos->x >= DAY17_BOARD_WIDTH - piece->width) return false;

	for (int i = 0; i < piece->count; i++) {
		i32 x = piece->blocks[i].x + pos->x;
		i32 y = piece->blocks[i].y + pos->y;
		if (day17_is_solid(board, x+1, y)) {
			return false;
		}
	}
	return true;
}

static u32 get_max_piece_height()
{
	u32 max_piece_height = 0;
	for (int i = 0; i < ARRAY_LEN(g_day17_pieces); i++) {
		max_piece_height = MAX(max_piece_height, g_day17_pieces[i].height);
	}
	return max_piece_height;
}

static void day17_simulate_rock(bool board[][DAY17_BOARD_WIDTH], u32 rock_idx, u32 *tower_height, day17_gusts *gusts, u32 *current_gust)
{
	vec2 piece_pos = { .x = 2, .y = *tower_height+3 };
	day17_piece *piece = &g_day17_pieces[rock_idx % ARRAY_LEN(g_day17_pieces)];

	while (true) {
		if (gusts->dirs[*current_gust] == DIR_LEFT) {
			if (day17_can_move_left(board, piece, &piece_pos)) {
				piece_pos.x -= 1;
			}
		} else if (gusts->dirs[*current_gust] == DIR_RIGHT) {
			if (day17_can_move_right(board, piece, &piece_pos)) {
				piece_pos.x += 1;
			}
		}
		*current_gust = (*current_gust + 1) % gusts->count;

		if (!day17_can_move_down(board, piece, &piece_pos)) {
			break;
		}

		piece_pos.y -= 1;
	}

	for (int i = 0; i < piece->count; i++) {
		u32 x = piece->blocks[i].x + piece_pos.x;
		u32 y = piece->blocks[i].y + piece_pos.y;
		board[y][x] = true;
		*tower_height = MAX(*tower_height, y+1);
	}
}

static void day17_part1(void *p)
{
	day17_gusts *data = (day17_gusts*)p;

	u32 rock_amount = 2022;

	u32 max_piece_height = get_max_piece_height();
	u32 max_board_height = (rock_amount+2) * max_piece_height + 3;
	bool board[max_board_height][DAY17_BOARD_WIDTH];
	memset(board, false, max_board_height*DAY17_BOARD_WIDTH*sizeof(bool));

	u32 tower_height = 0;
	u32 current_gust = 0;
	for (size_t rock_idx = 0; rock_idx < rock_amount; rock_idx++) {
		day17_simulate_rock(board, rock_idx, &tower_height, data, &current_gust);
	}

	printf("%d\n", tower_height);
}

static i32 day17_find_cycle_start(u32 *height_differences, u32 heights_count)
{
	for (u32 cycle_size = heights_count/2; cycle_size > 5; cycle_size--) {
		u32 cycle_start = heights_count - 2*cycle_size;

		bool found_cycle = true;
		for (int i = 0; i < cycle_size; i++) {
			if (height_differences[cycle_start + i] != height_differences[cycle_start + cycle_size + i]) {
				found_cycle = false;
				break;
			}
		}

		if (found_cycle) {
			return cycle_start;
		}
	}
	return -1;
}

static void day17_part2(void *p)
{
	day17_gusts *data = (day17_gusts*)p;

	u64 target_rocks = 1000000000000;

	u32 rock_amount = 4000;
	u32 max_piece_height = get_max_piece_height();
	u32 max_board_height = (rock_amount+2) * max_piece_height + 3;
	bool board[max_board_height][DAY17_BOARD_WIDTH];
	memset(board, false, max_board_height*DAY17_BOARD_WIDTH*sizeof(bool));

	u32 height_differences[rock_amount];
	memset(height_differences, 0, sizeof(u32) * rock_amount);


	i32 cycle_start = -1;
	u32 cycle_length = 0;

	u32 tower_height = 0;
	u32 current_gust = 0;
	for (size_t rock_idx = 0; rock_idx < rock_amount; rock_idx++) {
		u32 old_height = tower_height;
		day17_simulate_rock(board, rock_idx, &tower_height, data, &current_gust);
		height_differences[rock_idx] = tower_height - old_height;

		cycle_start = day17_find_cycle_start(height_differences, rock_idx+1);
		if (cycle_start != -1) {
			cycle_length = (rock_idx-cycle_start)/2 + 1;
			break;
		}
	}

	if (cycle_start == -1) {
		printf("Failed to find cycle, try increasing `rock_amount` for searching longer\n");
		return;
	}

	u64 answer = 0;

	// Add height up to the first cycle
	for (int i = 0; i < cycle_start; i++) {
		answer += height_differences[i];
	}

	u64 cycle_height = 0;
	for (int i = 0; i < cycle_length; i++) {
		cycle_height += height_differences[cycle_start + i];
	}

	// Calculate how many cycles would occur and extrapolate height
	answer += (target_rocks - cycle_start) / cycle_length * cycle_height;

	// Add remaining rocks at the end which have not completed a cycle
	u32 remainder_rocks = (target_rocks - cycle_start) % cycle_length;
	for (int i = 0; i < remainder_rocks; i++) {
		answer += height_differences[cycle_start + i];
	}

	printf("%ld\n", answer);

}

ADD_SOLUTION(17, day17_parse, day17_part1, day17_part2);
