#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "aoc.h"
#include "vec.h"

#define CD_CMD "$ cd"
#define LS_CMD "$ ls"

#define TOTAL_FS_SIZE 70000000
#define UPDATE_SIZE 30000000

struct TreeNode {
	char *name;
	struct TreeNode *parent;
	struct TreeNode **children;
	size_t children_count;
	size_t size;
};

static struct TreeNode *append_node(struct TreeNode *parent, char *child_name, size_t size)
{
	struct TreeNode *new_child = calloc(1, sizeof(struct TreeNode));
	new_child->parent = parent;
	new_child->name = child_name;
	new_child->size = size;

	parent->children = realloc(parent->children, sizeof(struct TreeNode*)*(parent->children_count+1));
	parent->children[parent->children_count] = new_child;
	parent->children_count++;
	return new_child;
}

static struct TreeNode *get_or_append_child(struct TreeNode *parent, char *child_name)
{
	for (int i = 0; i < parent->children_count; i++) {
		struct TreeNode *child = parent->children[i];
		if (strcmp(child->name, child_name) == 0) {
			return child;
		}
	}

	return append_node(parent, child_name, -1);
}

static int find_char(char *haystack, char needle) {
	int n = strlen(haystack);
	for (int i = 0; i < n; i++) {
		if (haystack[i] == needle) {
			return i;
		}
	}
	return -1;
}

static void populate_directory_sizes(struct TreeNode *start)
{
	struct TreeNode *current = start;
	while (current != start->parent) {
		bool is_explored = true;
		for (int i = 0; i < current->children_count; i++) {
			struct TreeNode *child = current->children[i];
			if (child->size == -1) {
				current = child;
				is_explored = false;
				break;
			}
		}

		if (is_explored) {
			current->size = 0;
			for (int i = 0; i < current->children_count; i++) {
				current->size += current->children[i]->size;
			}

			current = current->parent;
		}
	}
}

static void *day7_parse(char **lines, int line_count)
{
	struct TreeNode *root = calloc(1, sizeof(struct TreeNode));
	root->name = "/";

	struct TreeNode *current = root;
	for (int i = 0; i < line_count; i++) {
		char *line = lines[i];
		if (strncmp(line, CD_CMD, sizeof(CD_CMD)-1) == 0) {
			char *dir_name = line + sizeof(CD_CMD);
			if (strncmp(dir_name, "/", 1) == 0) {
				current = root;
			} else if (strncmp(dir_name, "..", 2) == 0) {
				current = current->parent;
			} else {
				current = get_or_append_child(current, dir_name);
			}
		} else if (strncmp(line, LS_CMD, sizeof(LS_CMD)) == 0) {
			continue;
		} else {
			if (strncmp(line, "dir", 3) == 0) {
				char *dir_name = line + 4;
				get_or_append_child(current, dir_name);
			} else {
				int sep = find_char(line, ' ');
				char *child_name = line + sep + 1;
				line[sep] = '\0';
				int size = strtol(line, NULL, 10);
				line[sep] = ' ';
				append_node(current, child_name, size);
			}
		}
	}

	return root;
}

static void day7_part1(void *p)
{
	struct TreeNode *root = p;

	populate_directory_sizes(root);

	int result = 0;
	Vec *stack = vec_malloc(10);
	vec_push(stack, root);
	while (stack->count > 0) {
		struct TreeNode *node = vec_pop(stack);
		if (node->size <= 100000 && node->children_count > 0) {
			result += node->size;
		}
		for (int i = 0; i < node->children_count; i++) {
			vec_push(stack, node->children[i]);
		}
	}

	printf("%d\n", result);
}

static void day7_part2(void *p)
{
	struct TreeNode *root = p;

	populate_directory_sizes(root);

	int needed_space = UPDATE_SIZE - (TOTAL_FS_SIZE - root->size);

	int result = INT_MAX;
	Vec *stack = vec_malloc(10);
	vec_push(stack, root);
	while (stack->count > 0) {
		struct TreeNode *node = vec_pop(stack);
		if (node->size >= needed_space && node->children_count > 0) {
			result = MIN(result, node->size);
		}
		for (int i = 0; i < node->children_count; i++) {
			vec_push(stack, node->children[i]);
		}
	}

	printf("%d\n", result);
}

ADD_SOLUTION(7, day7_parse, day7_part1, day7_part2);
