#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <glad/glad.h>

#include "tools.h"

typedef GLuint tex_t;
struct texinfo {
	tex_t tex;
	u32 width, height;
};
struct texinfo texinfo_load(const char *path);
struct texinfo texinfo_init(u32 width, u32 height, bool rgb, bool use_f32, bool mipmap);
void tex_bind(tex_t tex, u32 slot);

enum texbuf_format {
	TEXBUF_U8,
	TEXBUF_U16,
	TEXBUF_I32,
	TEXBUF_U32,
	TEXBUF_F32,
};

struct texbuf {
	tex_t tex;
	GLuint buf;
	usize size;
	enum texbuf_format format;
};

struct texbuf texbuf_init(enum texbuf_format format);
void texbuf_deinit(struct texbuf *self);
void texbuf_buffer(struct texbuf *self, void *data, usize size);
void texbuf_buffer_sub(struct texbuf *self, usize offset, void *data, usize size);
void texbuf_use(struct texbuf *self, u32 slot);

#endif

