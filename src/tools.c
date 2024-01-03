#include <stdarg.h>

#include "tools.h"

thread_local char *_vectmp_snprintf;

//
// Common functions
//
void tools_init(void) {
	_vectmp_snprintf = vector_init(*_vectmp_snprintf);
}
void tools_deinit(void) {
	_vectmp_snprintf = vector_deinit(_vectmp_snprintf);
}
char *loadfile(const char *path) {
	FILE *file = fopen(path, "rb");
	if (!file) return NULL;

	usize size = fsize(file);
	char *str = malloc(size + 1);
	fread(str, size, 1, file);
	str[size] = '\0';
	
	fclose(file);
	return str;
}
size_t fsize(FILE *file) {
	long pos = ftell(file);
	fseek(file, 0, SEEK_END);
	usize size = ftell(file);
	fseek(file, pos, SEEK_SET);
	return size;
}
const char *tmp_snprintf(const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	const int space_needed = vsnprintf(_vectmp_snprintf, vector_len(_vectmp_snprintf), format, args) + 1;
	if (space_needed > vector_len(_vectmp_snprintf)) {
		_vectmp_snprintf = vector_reserve(_vectmp_snprintf, space_needed);
		vsnprintf(_vectmp_snprintf, vector_len(_vectmp_snprintf), format, args);
	}
	va_end(args);
	return _vectmp_snprintf;
}

void *_vector_init(u32 elemsize, u32 capacity) {
	struct vector_hdr *header = malloc(sizeof(*header) + capacity * elemsize);
	*header = (struct vector_hdr){ .len = 0, .capacity = capacity, };
	return header + 1;
}
void *_vector_deinit(void *self) {
	struct vector_hdr *header = (struct vector_hdr *)self-1;
	free(header);
	return NULL;
}
void *_vector_resize(void *self, u32 elemsize, u32 len) {
	struct vector_hdr *header = (struct vector_hdr *)self-1;
	header->len = len;

	if (header->len > header->capacity) {
		header->capacity = 1 << find_last_set(header->len);
		header = realloc(header, sizeof(*header) + elemsize * header->capacity);
	}

	return header + 1;
}
void *_vector_reserve(void *self, u32 elemsize, u32 capacity) {
	struct vector_hdr *header = (struct vector_hdr *)self-1;

	if (capacity > header->capacity) {
		header->capacity = 1 << find_last_set(capacity);
		header = realloc(header, sizeof(*header) + elemsize * header->capacity);
	}
	
	return header + 1;
}
void *_vector_reserve_exact(void *self, u32 elemsize, u32 capacity) {
	struct vector_hdr *header = (struct vector_hdr *)self-1;
	if (header->capacity == capacity) return self;

	header->capacity = !capacity ? 1 : capacity;
	if (header->len > header->capacity) header->len = header->capacity;

	header = realloc(header, sizeof(*header) + elemsize * header->capacity);
	return header + 1;
}
void *_vector_push(void *self, u32 elemsize, const void *val) {
	self = _vector_resize(self, elemsize, vector_len(self) + 1);
	memcpy((u8 *)self + elemsize * vector_len(self) - 1, val, elemsize);
	return self;
}
void *_vector_pop(void *self, u32 elemsize) {
	if (vector_len(self)) return _vector_resize(self, elemsize, vector_len(self) - 1);
	else return self;
}

