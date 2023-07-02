#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "aoc.h"

#include "day1.c"
#include "day2.c"
#include "day3.c"
#include "day4.c"
#include "day5.c"
#include "day6.c"
#include "day7.c"
#include "day8.c"
#include "day9.c"
#include "day10.c"
#include "day11.c"
#include "day12.c"
#include "day13.c"
#include "day14.c"
#include "day15.c"
#include "day16.c"
#include "day17.c"
#include "day18.c"

Solution *find_solution(int day)
{
	for (int i = 0; i < SOLUTIONS_COUNT; i++) {
		Solution *s = &SOLUTIONS[i];
		if (s->day == day) {
			return s;
		}
	}
	return NULL;
}

char **read_lines(int *line_count, FILE *f)
{
	*line_count = 0;

	rewind(f);
	while(!feof(f)){
		char ch = fgetc(f);
		if(ch == '\n') {
			(*line_count)++;
		}
	}

	char **lines = malloc(*line_count * sizeof(char*));

	char *line;
	size_t len = 0;
	int i = 0;
	size_t read;
	rewind(f);
	while ((read = getline(&line, &len, f)) != -1) {
		line[read-1] = '\0';
		lines[i++] = strdup(line);
	}

	return lines;
}

int download_input(int day, char *filename, char *session)
{
	int rc = 0;

	curl_global_init(CURL_GLOBAL_ALL);
	FILE *f = fopen(filename, "w");
	CURL *curl = curl_easy_init();

	CURLcode curl_code;
	if (curl) {
		char *cookie_header = calloc(42 + strlen(session), sizeof(char));
		sprintf(cookie_header, "Cookie: session=%s", session);

		char url[81] = { 0 };
		sprintf(url, "https://adventofcode.com/2022/day/%d/input", day);

		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, cookie_header);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(curl, CURLOPT_URL, &url);
		curl_code = curl_easy_perform(curl);
		if (curl_code == CURLE_OK) {
			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (http_code != 200) {
				fprintf(stderr, "Request failed: HTTP code %ld\n", http_code);
				rc = -1;
			}
		} else {
			fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(curl_code));
			rc = -1;
		}

		curl_easy_cleanup(curl);
		curl_slist_free_all(chunk);
	}
end:
	curl_global_cleanup();
	fclose(f);
	if (rc) {
		remove(filename);
	}
	return rc;
}

void print_solutions()
{
	fprintf(stderr, "Available solutions: ");
	for (int i = 0; i < SOLUTIONS_COUNT; i++) {
		Solution *s = &SOLUTIONS[i];
		fprintf(stderr, "%d ", s->day);
	}
	fprintf(stderr, "\n");
}

uint64_t get_current_time_us()
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);
	return time.tv_sec * 1000000 + time.tv_nsec / 1000;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <day> [input.txt]\n", argv[0]);
		exit(1);
	}

	int day = atoi(argv[1]);
	if (day == 0) {
		fprintf(stderr, "Day number is invalid\n");
		print_solutions();
		exit(1);
	}

	Solution *solution = find_solution(day);
	if (solution == NULL) {
		fprintf(stderr, "Failed to find solution to %d\n", day);
		print_solutions();
		exit(1);
	}

	char *input_file = NULL;
	if (argc >= 3) {
		input_file = argv[2];
	} else if ( access( "input.txt", F_OK ) != -1  ) {
		input_file = "input.txt";
	} else {
		char* session = getenv("AOC_SESSION");
		if (session && !download_input(day, "input.txt", session)) {
			input_file = "input.txt";
		} else {
			fprintf(stderr, "Missing input file");
			exit(1);
		}
	}

	FILE *f = fopen(input_file, "r");
	if (f == NULL) {
		fprintf(stderr, "Failed to open file solution to day '%s': %s\n", input_file, strerror(errno));
		return 1;
	}

	int line_count;
	char **lines = read_lines(&line_count, f);

	fclose(f);

	uint64_t start_time = get_current_time_us();
	void* parsed = solution->parse(lines, line_count);
	printf("Parsing took %ldus\n", get_current_time_us() - start_time);

	printf("part1:\n");
	start_time = get_current_time_us();
	solution->part1(parsed);
	printf("Part 1 took %ldus\n\n", get_current_time_us() - start_time);

	printf("part2:\n");
	start_time = get_current_time_us();
	solution->part2(parsed);
	printf("Part 2 took %ldus\n", get_current_time_us() - start_time);

	return 0;
}
