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

	size_t size = fsize(file);
	char *str = malloc(size + 1);
	fread(str, size, 1, file);
	str[size] = '\0';
	
	fclose(file);
	return str;
}
size_t loadfileb(const char *path, uint8_t **bufptr) {
	FILE *file = fopen(path, "rb");
	if (!file) return 0;

	size_t size = fsize(file);
	*bufptr = malloc(size);
	fread(*bufptr, size, 1, file);
	
	fclose(file);
	return size;
}
size_t fsize(FILE *file) {
	long pos = ftell(file);
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
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

void *_vector_init(uint32_t elemsize, uint32_t capacity) {
	vector_hdr_t *header = malloc(sizeof(*header) + capacity * elemsize);
	*header = (vector_hdr_t){ .len = 0, .capacity = capacity, };
	return header + 1;
}
void *_vector_deinit(void *self) {
	vector_hdr_t *header = (vector_hdr_t *)self-1;
	free(header);
	return NULL;
}
void *_vector_resize(void *self, uint32_t elemsize, uint32_t len) {
	vector_hdr_t *header = (vector_hdr_t *)self-1;
	header->len = len;

	if (header->len > header->capacity) {
		header->capacity = 1 << find_last_set(header->len);
		header = realloc(header, sizeof(*header) + elemsize * header->capacity);
	}

	return header + 1;
}
void *_vector_reserve(void *self, uint32_t elemsize, uint32_t capacity) {
	vector_hdr_t *header = (vector_hdr_t *)self-1;

	if (capacity > header->capacity) {
		header->capacity = 1 << find_last_set(capacity);
		header = realloc(header, sizeof(*header) + elemsize * header->capacity);
	}
	
	return header + 1;
}
void *_vector_reserve_exact(void *self, uint32_t elemsize, uint32_t capacity) {
	vector_hdr_t *header = (vector_hdr_t *)self-1;
	if (header->capacity == capacity) return self;

	header->capacity = !capacity ? 1 : capacity;
	if (header->len > header->capacity) header->len = header->capacity;

	header = realloc(header, sizeof(*header) + elemsize * header->capacity);
	return header + 1;
}
void *_vector_push(void *self, uint32_t elemsize, const void *val) {
	self = _vector_resize(self, elemsize, vector_len(self) + 1);
	memcpy((uint8_t *)self + elemsize * (vector_len(self) - 1), val, elemsize);
	return self;
}
void *_vector_pop(void *self, uint32_t elemsize) {
	if (vector_len(self)) return _vector_resize(self, elemsize, vector_len(self) - 1);
	else return self;
}

