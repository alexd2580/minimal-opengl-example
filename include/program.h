#ifndef INCLUDE_PROGRAM_H
#define INCLUDE_PROGRAM_H

#include <stdbool.h>

struct Program_;
typedef struct Program_* Program;

// Initialize and install a program.
Program create_program(char const* vert_shader_path, char const* frag_shader_path);

// Can i use this program to render stuff.
bool program_is_valid(Program program);

// Combine the modified check and validity check.
void try_update_program(Program program);

// Bind the program.
void use_program(Program program);

// Deinitializa all program resources.
void delete_program(Program program);

#endif
