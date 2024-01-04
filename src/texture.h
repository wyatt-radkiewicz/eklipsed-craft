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

#endif

