#ifndef AOC_H_
#define AOC_H_

#include <stdbool.h>

typedef void (*solution_cb)(void*);
typedef void* (*parse_cb)(char** lines, int count);
typedef struct {
	int day;

	parse_cb parse;
	solution_cb part1;
	solution_cb part2;
} Solution;

// Macro magic for easy of use
#define ADD_SOLUTION(_day, _parse, _part1, _part2)                                                           \
	static parse_cb ptr_##_parse;                                                                            \
	static solution_cb ptr_##_part1;                                                                         \
	static Solution ptr_##_part2                                                                             \
	__attribute((used, section("g_solutions"))) = {                                                          \
		.parse = _parse,                                                                                     \
		.part1 = _part1,                                                                                     \
		.part2 = _part2,                                                                                     \
		.day = _day                                                                                          \
	}

#define SOLUTIONS ({                                                                                         \
			extern Solution __start_##g_solutions;                                                           \
			&__start_##g_solutions;                                                                          \
		})

#define SOLUTIONS_END ({                                                                                     \
			extern Solution __stop_##g_solutions;                                                            \
			&__stop_##g_solutions;                                                                           \
		})

#define SOLUTIONS_COUNT SOLUTIONS_END - SOLUTIONS

#endif //AOC_H_
