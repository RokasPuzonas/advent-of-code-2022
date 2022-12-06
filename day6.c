#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "aoc.h"

#define MESSAGE_LENGTH 14

static void *day6_parse(char **lines, int line_count)
{
	return lines[0];
}

static void day6_part1(void *p)
{
	char *msg = p;
	int n = strlen(msg);
	for (int i = 3; i < n; i++) {
		char c1 = msg[i-3];
		char c2 = msg[i-2];
		char c3 = msg[i-1];
		char c4 = msg[i-0];
		if (c1 != c2 && c1 != c3 && c1 != c4 &&
				c2 != c3 && c2 != c4 &&
				c3 != c4) {
			printf("%d\n", i+1);
			break;
		}
	}
}

static bool is_start_of_message(char *str)
{
	for (int i=0; i < MESSAGE_LENGTH - 1; i++) {
		for (int j=i+1; j < MESSAGE_LENGTH; j++) {
			if (str[i] == str[j]) {
				return false;
			}
		}
	}
	return true;
}

static void day6_part2(void *p)
{
	char *msg = p;
	int n = strlen(msg);
	for (int i = 0; i < n-13; i++) {
		if (is_start_of_message(msg+i)) {
			printf("%d\n", i+14);
			break;
		}
	}
}

ADD_SOLUTION(6, day6_parse, day6_part1, day6_part2);
