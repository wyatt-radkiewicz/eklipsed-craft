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

