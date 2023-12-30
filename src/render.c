#include "ecraft.h"

// Window and context creation
static bool _sdl2_inited;
SDL_Window *g_window;
SDL_GLContext *g_glcontext;

bool window_init(void) {
	_sdl2_inited = true;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("%s", SDL_GetError());
		_sdl2_inited = false;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	g_window = SDL_CreateWindow(
		"Eklipsed Craft",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	if (!g_window) {
		printf("%s", SDL_GetError());
		return false;
	}

	g_glcontext = SDL_GL_CreateContext(g_window);
	if (!g_glcontext) {
		printf("%s", SDL_GetError());
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		printf("glad: cannot load opengl functions!\n");
		return false;
	}

	glViewport(0, 0, 800, 600);

	return true;
}

void window_deinit(void) {
	if (g_glcontext) SDL_GL_DeleteContext(g_glcontext);
	if (g_window) SDL_DestroyWindow(g_window);
	if (_sdl2_inited) SDL_Quit();
}

