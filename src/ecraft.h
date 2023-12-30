#ifndef _ecraft_h
#define _ecraft_h
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <glad/glad.h>
#include <SDL.h>

// Window and context creation
extern SDL_Window *g_window;
extern SDL_GLContext *g_glcontext;
bool window_init(void);
void window_deinit(void);

#endif

