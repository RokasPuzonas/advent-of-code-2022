#ifndef AOC_H_
#define AOC_H_

#include <stdbool.h>

typedef int (*solution_cb)(void*);
typedef struct {
	int day;

	void* (*parse)(char** lines, int count);
	int (*part1)(void* data);
	int (*part2)(void* data);
} Solution;

// Macro magic for easy of use
#define ADD_SOLUTION(_day, parse, part1, part2)                                                    \
		static Solution ptr_##parse;                                                                   \
		static Solution ptr_##part1;                                                                   \
		static Solution ptr_##part2                                                                    \
		__attribute((used, section("g_solutions"))) = {                                                \
			.parse = parse,                                                                              \
			.part1 = part1,                                                                              \
			.part2 = part2,                                                                              \
			.day = _day                                                                                  \
		}

#define SOLUTIONS ({                                                                               \
			extern Solution __start_##g_solutions;                                                       \
			&__start_##g_solutions;                                                                      \
		})

#define SOLUTIONS_END ({                                                                           \
			extern Solution __stop_##g_solutions;                                                        \
			&__stop_##g_solutions;                                                                       \
		})

#define SOLUTIONS_COUNT SOLUTIONS_END - SOLUTIONS

#endif //AOC_H_
