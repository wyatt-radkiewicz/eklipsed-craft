#include <stb_image.h>

#include "texture.h"

struct texinfo texinfo_load(const char *path) {
	struct texinfo texinfo = texinfo_init(0, 0, false, false, true);
    stbi_set_flip_vertically_on_load(true);

	int width, height, num_channels;
    unsigned char *data = stbi_load(path, &width, &height, &num_channels, 4);
    if (!data) {
		glDeleteTextures(1, &texinfo.tex);
		return (struct texinfo){0};
	}

	texinfo.width = width;
	texinfo.height = height;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texinfo.width, texinfo.height, 0, num_channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

	return texinfo;
}
struct texinfo texinfo_init(u32 width, u32 height, bool rgb, bool use_f32, bool mipmap) {
	struct texinfo texinfo = (struct texinfo){ .width = width, .height = height };

	glGenTextures(1, &texinfo.tex);
    glBindTexture(GL_TEXTURE_2D, texinfo.tex); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	if (width && height) {
		glTexImage2D(GL_TEXTURE_2D, 0, rgb ? GL_RGB : GL_RGBA, texinfo.width, texinfo.height, 0, GL_RGBA, use_f32 ? GL_FLOAT : GL_UNSIGNED_BYTE, NULL);
    	if (mipmap) glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	return texinfo;
}
void tex_bind(tex_t tex, u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, tex);
}

static GLenum _texbuf_get_internal_format(const struct texbuf *self) {
	switch (self->format) {
	case TEXBUF_U8: return GL_RGBA8UI;
	case TEXBUF_U16: return GL_RGBA16UI;
	case TEXBUF_I32: return GL_RGBA32I;
	case TEXBUF_U32: return GL_RGBA32F;
	case TEXBUF_F32: return GL_RGBA32F;
	default: return -1;
	}
}
struct texbuf texbuf_init(enum texbuf_format format) {
	struct texbuf self = (struct texbuf){ .format = format, };

	glGenBuffers(1, &self.buf);
	glGenTextures(1, &self.tex);
	return self;
}
void texbuf_deinit(struct texbuf *self) {
	glDeleteBuffers(1, &self->buf);
	glDeleteTextures(1, &self->tex);
}
void texbuf_buffer(struct texbuf *self, void *data, usize size) {
	glBindBuffer(GL_TEXTURE_BUFFER, self->buf);
	glBufferData(GL_TEXTURE_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}
void texbuf_buffer_sub(struct texbuf *self, usize offset, void *data, usize size) {
	glBindBuffer(GL_TEXTURE_BUFFER, self->buf);
	glBufferSubData(GL_TEXTURE_BUFFER, offset, size, data);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}
void texbuf_use(struct texbuf *self, u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_BUFFER, self->tex);
	glTexBuffer(GL_TEXTURE_BUFFER, _texbuf_get_internal_format(self), self->buf);
}

