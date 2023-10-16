#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include "program.h"

void check_error(char const *loc) {
  GLenum e;
  if ((e = glGetError()) != GL_NO_ERROR) {
    printf("%s: OpenGL error \"%s\"\n", loc, gluErrorString(e));
    exit(1);
  }
}

struct Size {
  int w;
  int h;
};

bool handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      return true;
    default:
      break;
    }
  }

  return false;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // Initialize SDL.
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // Create window.
  struct Size size = {.w = 800, .h = 800};
  int pos = SDL_WINDOWPOS_CENTERED;
  Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
  SDL_Window *window = SDL_CreateWindow("", pos, pos, size.w, size.h, flags);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  glViewport(0, 0, size.w, size.h);

  glClearColor(0.2f, 0, 0.2f, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  check_error("Blend setup");

  /* glDisable(GL_DEPTH_TEST); */
  /* // glClearColor(0.0, 0.0, 0.0, 0.0); */

  /* glGenFramebuffers(1, &window->framebuffer); */
  /* glBindFramebuffer(GL_READ_FRAMEBUFFER, window->framebuffer); */

  /* Textures textures = create_textures(size); */

  // What the fuck?! Why does this work without a vao on pyopengl?!
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  Program shader_a = create_program("shaders/quad.vert", "shaders/data.frag");
  check_error("Compile program");
  /* Program shader_b = create_program("assets/shaders/basic_present.comp"); */

  while (true) {
    update_program_if_modified(shader_a);
    check_error("Update program");
    /* reinstall_program_if_modified(shader_b); */

    /*     swap_and_bind_textures(textures); */
    glClear(GL_COLOR_BUFFER_BIT);
    check_error("Clear color");

    use_program(shader_a);
    check_error("Use program");

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    check_error("Draw arrays");

    glUseProgram(0);
    check_error("Unuse program");

    SDL_GL_SwapWindow(window);

    if (handle_events()) {
      break;
    }
  }

  delete_program(shader_a);
  /* delete_program(shader_b); */
  /* delete_textures(textures); */

  // Close window.
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);

  // Quit SDL.
  SDL_Quit();

  return 0;
}