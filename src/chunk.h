#ifndef _CHUNK_H
#define _CHUNK_H

#include "mesh.h"
#include "texture.h"
#include "tools.h"

// Used when loading minecraft chunks
#define MC_CHUNK_SIZE 16
// Actual chunk size in game
#define CHUNK_SIZE 32

struct chunk_basic_vtx {
	u16 draw_id;
	// 1  | 4    | 4    | 7*3 = 21+8+1 = 30 used bits
	// ao | uv y | uv x | z y x
	u32 packed_pos_uv_ao;
	// 4      | 4  | 4  | 4
	// normal | b? | g? | light
	u16 light_normal;
};

struct chunk_complex_vtx {
	vec3s pos;
	// 1  | 4    | 4 = 8+1 = 9 used bits
	// ao | uv y | uv x 
	u16 uv_ao;
	u16 light_normal;
};

struct mesh_heap_chunk {
	bool free;
	u32 order;
	struct mesh_heap_chunk *last_free;
	struct mesh_heap_chunk *next_free;
};

struct mesh_heap {
	usize size;
	u32 order;
	struct mesh_heap_chunk **free;
	struct mesh_heap_chunk *chunks;
};

union chunk_mesh_pos {
	vec3s pos;
	union chunk_mesh_pos *next;
};

struct chunk_mesh_poses {
	usize size;
	union chunk_mesh_pos *poses;
	union chunk_mesh_pos *free;
};

struct chunk_mesh_alloc {
	usize max_chunks;
	struct mesh basic_mesh, complex_mesh, water_mesh;
	struct mesh_heap basic_heap, complex_heap, water_heap;
	struct chunk_mesh_poses poses;
	struct texbuf poses_buf;
};

u32 mesh_heap_chunk_get_vtxid(struct mesh_heap_chunk *self);
u32 chunk_mesh_pos_getid(union chunk_mesh_pos *self);

struct chunk_mesh_alloc chunk_mesh_alloc_init(usize num_vtxs, usize max_chunks);
void chunk_mesh_alloc_deinit(struct chunk_mesh_alloc *self);
struct mesh_heap_chunk *chunk_mesh_alloc_alloc_basic(struct chunk_mesh_alloc *self, const struct chunk_basic_vtx *vtxs, usize num_vtxs, vec3s base_pos);
struct mesh_heap_chunk *chunk_mesh_alloc_complex(struct chunk_mesh_alloc *self, const struct chunk_complex_vtx *vtxs, usize num_vtxs);
struct mesh_heap_chunk *chunk_mesh_alloc_water(struct chunk_mesh_alloc *self, const struct chunk_complex_vtx *vtxs, usize num_vtxs);
struct mesh_heap_chunk *chunk_mesh_realloc_basic(struct chunk_mesh_alloc *self, struct mesh_heap_chunk *chunk, const struct chunk_basic_vtx *vtxs, usize num_vtxs);
struct mesh_heap_chunk *chunk_mesh_realloc_complex(struct chunk_mesh_alloc *self, struct mesh_heap_chunk *chunk, const struct chunk_complex_vtx *vtxs, usize num_vtxs);
struct mesh_heap_chunk *chunk_mesh_realloc_water(struct chunk_mesh_alloc *self, struct mesh_heap_chunk *chunk, const struct chunk_complex_vtx *vtxs, usize num_vtxs);
void chunk_mesh_free_basic(struct chunk_mesh_alloc *self, struct mesh_heap_chunk *chunk);
void chunk_mesh_free_complex(struct chunk_mesh_alloc *self, struct mesh_heap_chunk *chunk);
void chunk_mesh_free_water(struct chunk_mesh_alloc *self, struct mesh_heap_chunk *chunk);

#endif

