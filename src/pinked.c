#include <assert.h>
#include <stdbool.h>

#include <cglm/cglm.h>

#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#undef main

#include "bitmap.h"
#include "layer.h"

static void check_gl_errors_helper(const char* file, int line) {
	while(1) {
		GLenum err = glGetError();
		if(err == GL_NO_ERROR)
			break;
		const char* error_str[] = {
			[GL_NO_ERROR] = "GL_NO_ERROR",
			[GL_INVALID_ENUM] = "GL_INVALID_ENUM",
			[GL_INVALID_VALUE] = "GL_INVALID_VALUE",
			[GL_INVALID_OPERATION] = "GL_INVALID_OPERATION",
			[GL_INVALID_FRAMEBUFFER_OPERATION]
			= "GL_INVALID_FRAMEBUFFER_OPERATION",
			[GL_OUT_OF_MEMORY] = "GL_OUT_OF_MEMORY",
		};
		printf("%s:L%u glGetError(): %s\n", file, line, error_str[err]);
	}
}

#define CHECK_GL_ERRORS(func) func, check_gl_errors_helper(__FILE__, __LINE__)

int main(int argc, char** argv) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	SDL_Window* window = SDL_CreateWindow(
		"PinkEd", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1120, 600,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GLContext ctx = SDL_GL_CreateContext(window);

	// try adaptive sync first, then fall back to vsync
	if(!SDL_GL_SetSwapInterval(-1))
		SDL_GL_SetSwapInterval(1);

	// SDL_SetRelativeMouseMode(SDL_TRUE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

	char* v_src = "#version 100\n"
				  "uniform mat4 mvp;\n"
				  "attribute vec3 v_pos;\n"
				  "void main() {\n"
				  "	gl_Position = mvp * vec4(v_pos, 1.0);\n"
				  "}";
	char* f_src = "#version 100\n"
				  "void main() {\n"
				  "	gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);"
				  "}";

	GLuint compile_status;
	GLuint shader_v = glCreateShader(GL_VERTEX_SHADER);
	GLuint shader_f = glCreateShader(GL_FRAGMENT_SHADER);
	CHECK_GL_ERRORS(glShaderSource(shader_v, 1, (const GLchar**)&v_src,
								   (GLuint[]) {strlen(v_src)}));
	CHECK_GL_ERRORS(glShaderSource(shader_f, 1, (const GLchar**)&f_src,
								   (GLuint[]) {strlen(f_src)}));

	CHECK_GL_ERRORS(glCompileShader(shader_v));
	glGetShaderiv(shader_v, GL_COMPILE_STATUS, &compile_status);
	assert(compile_status == GL_TRUE);

	CHECK_GL_ERRORS(glCompileShader(shader_f));
	glGetShaderiv(shader_v, GL_COMPILE_STATUS, &compile_status);
	assert(compile_status == GL_TRUE);

	GLuint prog = glCreateProgram();
	CHECK_GL_ERRORS(glAttachShader(prog, shader_v));
	CHECK_GL_ERRORS(glAttachShader(prog, shader_f));
	CHECK_GL_ERRORS(glLinkProgram(prog));

	CHECK_GL_ERRORS(glUseProgram(prog));

	glBindAttribLocation(prog, 0, "v_pos");

	bool quit = false;

	struct layer test;
	layer_create(&test, 0, 0, 0);

	for(int x = -256; x < 256; x++) {
		for(int y = -256; y < 256; y++) {
			for(int z = 0; z < 8; z++) {
				layer_set_solid(&test, x, y, z, (struct color) {255, 0, 255});
			}
		}
	}

	while(!quit) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_WINDOWEVENT:
					switch(event.window.event) {
						case SDL_WINDOWEVENT_CLOSE: quit = true; break;
						case SDL_WINDOWEVENT_RESIZED:
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							glViewport(0, 0, event.window.data1,
									   event.window.data2);
							break;
					}
					break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int8_t vertices[] = {0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0};

		glUniformMatrix4fv(glGetUniformLocation(prog, "mvp"), 1, GL_FALSE,
						   (float[]) {0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0.5, 0,
									  -0.25, -0.25, 0, 1});

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 0, vertices);
		glDrawArrays(GL_LINES, 0, 4);
		glDisableVertexAttribArray(0);

		layer_render(&test);

		SDL_GL_SwapWindow(window);
	}

	layer_destroy(&test);

	SDL_GL_DeleteContext(ctx);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
