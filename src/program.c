// See https://antongerdelan.net/opengl/compute.html for reference.
#include <stdbool.h>
#include <stdio.h>
#include <time.h> // TODO

#include <sys/stat.h>
#include <sys/types.h>

#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include "program.h"

struct Program_ {
  char const *vert_shader_path;
  time_t vert_shader_mtime;
  GLuint vert_shader;
  char const *frag_shader_path;
  time_t frag_shader_mtime;
  GLuint frag_shader;
  GLuint program;
};

char *read_file(char const *path, int *size) {
  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open file %s\n", path);
    exit(1);
  }

  if (fseek(fp, 0L, SEEK_END) != 0) {
    fprintf(stderr, "Failed to fseek\n");
    exit(1);
  }

  int bufsize = (int)ftell(fp);
  if (bufsize == -1) {
    fprintf(stderr, "Failed to ftell\n");
    exit(1);
  }

  if (size != NULL) {
    *size = bufsize;
  }

  char *data = malloc(sizeof(char) * (size_t)(bufsize + 1));

  if (fseek(fp, 0L, SEEK_SET) != 0) {
    fprintf(stderr, "Failed to fseek\n");
    exit(1);
  }

  fread(data, sizeof(char), (size_t)bufsize, fp);
  if (ferror(fp) != 0) {
    fprintf(stderr, "Failed to fread\n");
    exit(1);
  }

  data[bufsize] = '\0';
  fclose(fp);

  return data;
}

time_t get_mtime(char const *path) {
  struct stat attr;
  stat(path, &attr);
  return attr.st_mtime;
}

GLuint compile_shader(GLenum type, char const *source_path) {
  GLuint shader = glCreateShader(type);

  int source_len;
  char *source = read_file(source_path, &source_len);
  char const *const_source = source;

  glShaderSource(shader, 1, (GLchar const **)&const_source, &source_len);
  glCompileShader(shader);

  free(source);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    int max_size = 10000;
    GLchar *log = (GLchar *)malloc((size_t)max_size * sizeof(GLchar));
    GLsizei length;
    glGetShaderInfoLog(shader, max_size, &length, log);
    fprintf(stderr, "%s: shader compilation failed\n%.*s", source_path, length,
            log);
    free(log);
    glDeleteShader(shader);
    return (GLuint)-1;
  }

  return shader;
}

void uninstall_program(Program program) {
  glDeleteProgram(program->program);
  glDeleteShader(program->vert_shader);
  glDeleteShader(program->frag_shader);
}

void print_program_info_log(GLuint program, char const *msg) {
  int max_size = 10000;
  GLchar *log = (GLchar *)malloc((size_t)max_size * sizeof(GLchar));
  GLsizei length;
  glGetProgramInfoLog(program, max_size, &length, log);
  fprintf(stderr, "%s\n%.*s", msg, length, log);
  free(log);
}

GLint get_program_i(GLuint program, GLuint what) {
  GLint result;
  glGetProgramiv(program, what, &result);
  return result;
}

void update_program_if_valid(Program program) {
  time_t t = time(NULL);
  struct tm *localized_time = localtime(&t);
  char s[1000];
  strftime(s, 1000, "%F %T", localized_time);
  printf("[%s] Compiling %s and %s...\n", s, program->vert_shader_path,
         program->frag_shader_path);

  program->vert_shader_mtime = get_mtime(program->vert_shader_path);
  program->frag_shader_mtime = get_mtime(program->frag_shader_path);

  GLuint vert_shader =
      compile_shader(GL_VERTEX_SHADER, program->vert_shader_path);
  if (vert_shader == (GLuint)-1) {
    return;
  }

  GLuint frag_shader =
      compile_shader(GL_FRAGMENT_SHADER, program->frag_shader_path);
  if (frag_shader == (GLuint)-1) {
    return;
  }

  GLuint prgm = glCreateProgram();
  glAttachShader(prgm, vert_shader);
  glAttachShader(prgm, frag_shader);

  glLinkProgram(prgm);
  if (get_program_i(prgm, GL_LINK_STATUS) == GL_FALSE) {
    print_program_info_log(prgm, "Failed to link program");

    glDeleteProgram(prgm);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return;
  }

  if (program->program != (GLuint)-1) {
    uninstall_program(program);
  }

  program->vert_shader = vert_shader;
  program->frag_shader = frag_shader;
  program->program = prgm;

  glValidateProgram(program->program);
  if (get_program_i(program->program, GL_VALIDATE_STATUS) != GL_TRUE) {
    print_program_info_log(prgm, "Failed to validate program");
  }
}

Program create_program(char const *vert_shader_path,
                       char const *frag_shader_path) {
  Program program = (struct Program_ *)malloc(sizeof(struct Program_));

  program->vert_shader_path = vert_shader_path;
  program->vert_shader = (GLuint)-1;
  program->frag_shader_path = frag_shader_path;
  program->frag_shader = (GLuint)-1;
  program->program = (GLuint)-1;

  update_program_if_valid(program);

  return program;
}

void update_program_if_modified(Program program) {
  bool vert_updated =
      get_mtime(program->vert_shader_path) > program->vert_shader_mtime;
  bool frag_updated =
      get_mtime(program->frag_shader_path) > program->frag_shader_mtime;

  if (vert_updated || frag_updated) {
    update_program_if_valid(program);
  }
}

void use_program(Program program) { glUseProgram(program->program); }

void delete_program(Program program) {
  uninstall_program(program);
  free(program);
}
