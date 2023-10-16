#ifndef INCLUDE_PROGRAM_H
#define INCLUDE_PROGRAM_H

#include <SDL2/SDL_opengl.h>
#include <stdbool.h>
#include <time.h>

struct Program_;
typedef struct Program_* Program;

// Initialize and install a program.
Program create_program(char const* vert_shader_path, char const* frag_shader_path);

// Combine the modified check and validity check.
void update_program_if_modified(Program program);

// Bind the program.
void use_program(Program program);

// Deinitializa all program resources.
void delete_program(Program program);

#endif
