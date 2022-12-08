#ifndef VEC_H_
#define VEC_H_

#include <stdlib.h>
#include <assert.h>

typedef struct {
	int count;
	int capacity;
	void **data;
} Vec;

static inline Vec *vec_malloc(size_t capacity)
{
	Vec *vec = (Vec*)malloc(sizeof(Vec));
	vec->count = 0;
	vec->capacity = capacity;
	vec->data = (void**)malloc(capacity * sizeof(void*));
	return vec;
}

static inline void vec_push(Vec *vec, void* value)
{
	if (vec->count >= vec->capacity) {
		vec->capacity = (vec->capacity + 1) * 2;
		vec->data = (void**)realloc(vec->data, vec->capacity * sizeof(void*));
	}
	vec->data[vec->count++] = value;
}

static inline void *vec_pop(Vec* vec)
{
	vec->count--;
	return vec->data[vec->count];
}

static inline void vec_free(Vec *v)
{
	free(v->data);
	free(v);
}


#endif //VEC_H_
