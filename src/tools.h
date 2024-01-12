#ifndef _TOOLS_H
#define _TOOLS_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <cglm/struct.h>

//
// Type definitions
//
typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef size_t usize;
typedef uintptr_t uptr;
typedef intptr_t iptr;

//
// Universal definitions
//
#ifndef thread_local
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define thread_local _Thread_local
# elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __DMC__ || \
       defined __BORLANDC__ )
#  define thread_local __declspec(thread) 
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define thread_local __thread
# else
#  error "Cannot define thread_local"
# endif
#endif

//
// Small helper macros/functions
//
#define arrlen(arr) (sizeof(arr) / sizeof((arr)[0]))
#define torad(deg) ((deg)/180.0f*M_PI)
#define todeg(rad) ((rad)/M_PI*180.0f)
#define min(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a < _b ? _a : _b; \
})
#define max(a, b) ({ \
	typeof(a) _a = (a); \
	typeof(b) _b = (b); \
	_a > _b ? _a : _b; \
})
#define clamp(a, mn, mx) ({ \
	typeof(a) _a = (a); \
	typeof(mn) _mn = (mn); \
	typeof(mx) _mx = (mx); \
	min(max(_a, _mn), _mx); \
})
#define setbit(x, i) ((x) | (1 << (i)))
#define clrbit(x, i) ((x) & ~(1 << (i)))
#define cnt_leading_zeros(x) (_Generic((x), \
	unsigned int		: __builtin_clz((x)), \
	unsigned long		: __builtin_clzl((x)), \
	unsigned long long	: __builtin_clzll((x)) \
))
#define cnt_trailing_zeros(x) (_Generic((x), \
	unsigned int		: __builtin_ctz((x)), \
	unsigned long		: __builtin_ctzl((x)), \
	unsigned long long	: __builtin_ctzll((x)) \
))
// Returns first set + 1
#define find_first_set(x) (_Generic((x), \
	unsigned int		: __builtin_ffs((x)), \
	unsigned long		: __builtin_ffsl((x)), \
	unsigned long long	: __builtin_ffsll((x)) \
))
// Returns last set + 1
#define find_last_set(x) ({ \
	typeof(x) _x = (x); \
	_x ? (8 * sizeof(_x) - cnt_leading_zeros(_x)) : 0; \
})

//
// Common functions
//
void tools_init(void);
void tools_deinit(void);
char *loadfile(const char *path);
usize fsize(FILE *file);
const char *tmp_snprintf(const char *restrict format, ...);

//
// Vector functions
//
struct vector_hdr {
	u32 len, capacity;
	u8 data[];
};

#define vector_len(vector) (((struct vector_hdr *)(vector) - 1)->len)
#define vector_capacity(vector) (((struct vector_hdr *)(vector) - 1)->capacity)
#define vector_init(type) _vector_init(sizeof(type), 4)
void *_vector_init(u32 elemsize, u32 capacity);
#define vector_deinit(vector) _vector_deinit((vector))
void *_vector_deinit(void *self);
#define vector_resize(vector, len) _vector_resize((vector), sizeof(*(vector)), len)
void *_vector_resize(void *self, u32 elemsize, u32 len);
#define vector_reserve(vector, capacity) _vector_reserve((vector), sizeof(*(vector)), capacity)
void *_vector_reserve(void *self, u32 elemsize, u32 capacity);
#define vector_reserve_exact(vector, capacity) _vector_reserve_exact((vector), sizeof(*(vector)), capacity)
void *_vector_reserve_exact(void *self, u32 elemsize, u32 capacity);
#define vector_push(vector, ...) ({ \
	typeof(*(vector)) _val = (__VA_ARGS__); \
	_vector_push((vector), sizeof(*(vector)), &(_val)); \
})
void *_vector_push(void *self, u32 elemsize, const void *val);
#define vector_pop(vector) _vector_pop((vector), sizeof(*(vector)))
void *_vector_pop(void *self, u32 elemsize);

//
// Game specific
//
enum cube_face {
	FACE_PZ,
	FACE_NZ,
	FACE_PY,
	FACE_NY,
	FACE_PX,
	FACE_NX,
};

#endif

