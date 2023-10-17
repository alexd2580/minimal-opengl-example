// See https://antongerdelan.net/opengl/compute.html for reference.
#include <stdbool.h>
#include <stdio.h>

#include <sys/types.h>

#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include "error.h"
#include "file.h"
#include "globals.h"
#include "log.h"
#include "program.h"

GLuint invalid = (GLuint)-1;

void eprint_info_log(
    void (*get_log)(GLuint, GLsizei, GLsizei*, GLchar*),
    void (*get_log_length)(GLuint, GLenum, GLint*),
    GLuint resource,
    char const* msg
) {
    GLint length;
    (*get_log_length)(resource, GL_INFO_LOG_LENGTH, &length);
    if(length == 0) {
        return;
    }

    GLchar* log = (GLchar*)malloc((size_t)(length + 1) * sizeof(GLchar));
    (*get_log)(resource, length + 1, nullptr, log);
    fprintf(stderr, "%s\n%.*s", msg, length, log);
    free(log);
}

void eprint_shader_info_log(GLuint shader, char const* msg) {
    eprint_info_log(&glGetShaderInfoLog, &glGetShaderiv, shader, msg);
}

void eprint_program_info_log(GLuint program, char const* msg) {
    eprint_info_log(&glGetProgramInfoLog, &glGetProgramiv, program, msg);
}

GLint get_program_i(GLuint program, GLuint what) {
    GLint result;
    glGetProgramiv(program, what, &result);
    return result;
}

typedef struct Shader_* Shader;
struct Shader_ {
    GLenum type;
    char const* path;
    time_t mtime;
    GLuint shader;
};

Shader create_shader(GLenum type, char const* path) {
    Shader shader = ALLOCATE(1, struct Shader_);
    shader->type = type;
    shader->path = path;
    shader->mtime = 0;
    shader->shader = invalid;
    return shader;
}

bool shader_is_valid(Shader shader) { return shader->shader != invalid; }
bool shader_was_modified(Shader shader) { return mtime_of(shader->path) > shader->mtime; }

GLuint compile_shader(Shader shader) {
    info_log("Compiling %s...\n", shader->path);
    shader->mtime = mtime_of(shader->path);

    GLuint new_shader = glCreateShader(shader->type);

    int source_len;
    char* source = read_file(shader->path, &source_len);
    char const* const_source = source;

    glShaderSource(new_shader, 1, (GLchar const**)&const_source, &source_len);
    glCompileShader(new_shader);

    free(source);

    GLint status;
    glGetShaderiv(new_shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        eprint_shader_info_log(new_shader, "Shader compilation failed:");
        glDeleteShader(new_shader);
        return invalid;
    }

    return new_shader;
}

void uninstall_shader(GLuint shader) {
    if(shader != invalid) {
        glDeleteShader(shader);
    }
}

void delete_shader(Shader shader) {
    uninstall_shader(shader->shader);
    free(shader);
}

struct Program_ {
    Shader vertex;
    Shader fragment;
    GLuint program;
};

Program create_program(char const* vertex_shader_path, char const* fragment_shader_path) {
    Program program = ALLOCATE(1, struct Program_);
    program->vertex = create_shader(GL_VERTEX_SHADER, vertex_shader_path);
    program->fragment = create_shader(GL_FRAGMENT_SHADER, fragment_shader_path);
    program->program = invalid;
    return program;
}

bool program_is_valid(Program program) { return program->program != invalid; }

GLuint link_program(GLuint vertex, GLuint fragment) {
    info_log("Linking shader program...\n");
    GLuint program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    glLinkProgram(program);
    if(get_program_i(program, GL_LINK_STATUS) == GL_FALSE) {
        eprint_program_info_log(program, "Failed to link program");
        glDeleteProgram(program);
        return invalid;
    }

    return program;
}

void uninstall_program(GLuint program) {
    if(program != invalid) {
        glDeleteProgram(program);
    }
}

void try_update_program(Program program) {
    Shader vertex = program->vertex;
    Shader fragment = program->fragment;

    if(!shader_was_modified(vertex) && !shader_was_modified(fragment)) {
        return;
    }

    GLuint new_vertex = compile_shader(vertex);
    vertex->mtime = mtime_of(vertex->path);
    GLuint new_fragment = compile_shader(fragment);
    fragment->mtime = mtime_of(fragment->path);

    if(new_vertex == invalid || new_fragment == invalid) {
        uninstall_shader(new_vertex);
        uninstall_shader(new_fragment);
        return;
    }

    GLuint new_program = link_program(new_vertex, new_fragment);
    if(new_program == invalid) {
        uninstall_shader(new_vertex);
        uninstall_shader(new_fragment);
        return;
    }

    glValidateProgram(new_program);
    if(get_program_i(new_program, GL_VALIDATE_STATUS) != GL_TRUE) {
        eprint_program_info_log(new_program, "Failed to validate program");
        glDeleteProgram(new_program);
        uninstall_shader(new_vertex);
        uninstall_shader(new_fragment);
        return;
    }

    uninstall_program(program->program);
    uninstall_shader(vertex->shader);
    uninstall_shader(fragment->shader);

    program->program = new_program;
    vertex->shader = new_vertex;
    fragment->shader = new_fragment;
}

void use_program(Program program) { glUseProgram(program->program); }

void delete_program(Program program) {
    uninstall_program(program->program);
    delete_shader(program->vertex);
    delete_shader(program->fragment);
    free(program);
}
