#include "ecraft.h"

int main(int argc, char **argv) {
	if (!window_init()) {
		window_deinit();
		return -1;
	}

	bool runloop = true;
	while (runloop) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_WINDOWEVENT:
				if (event.window.type == SDL_WINDOWEVENT_RESIZED) {
					glViewport(0, 0, event.window.data1, event.window.data2);
				}
				break;
			case SDL_QUIT:
				runloop = false;
				break;
			}
		}

		glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(g_window);
	}

	window_deinit();
	return 0;
}

