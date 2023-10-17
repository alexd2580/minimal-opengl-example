#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_opengl.h>

#include "error.h"
#include "program.h"

struct Size {
    int w;
    int h;
};

void init_sdl() {
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
}

GLuint create_sampler() {
    GLuint sampler;
    glCreateSamplers(1, &sampler);

    // These parameters MUST be set (may be set with different values).
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return sampler;
}

GLuint create_texture(struct Size size, GLint num_of_components_enum) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // The values we put here (the unsized ones!) are the same for both internalFormat and format.
    GLenum format_of_data_which_doesnt_exist = (GLenum)num_of_components_enum;
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        num_of_components_enum,
        size.w,
        size.h,
        0,
        format_of_data_which_doesnt_exist,
        GL_FLOAT,
        NULL
    );

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLuint create_framebuffer(GLuint color_texture, GLuint depth_texture) {
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depth_texture, 0);
    const GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    init_sdl();

    // Create window.
    struct Size size = {.w = 800, .h = 800};
    int pos = SDL_WINDOWPOS_CENTERED;
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    SDL_Window* window = SDL_CreateWindow("", pos, pos, size.w, size.h, flags);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    glViewport(0, 0, size.w, size.h);
    glClearColor(0.5f, 0.0, 0.5f, 1);

    GLuint linearFilteringSampler = create_sampler();

    GLuint color_texture = create_texture(size, GL_RGBA);
    GLuint depth_texture = create_texture(size, GL_RED);
    GLuint framebuffer = create_framebuffer(color_texture, depth_texture);

    // What the fuck?! Why does this work without a vao on pyopengl?!
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    Program shader_a = create_program("shaders/quad.vert", "shaders/data.frag");
    Program shader_b = create_program("shaders/quad.vert", "shaders/color.frag");

    SDL_Event event;
    bool run = true;
    while(run) {
        try_update_program(shader_a);
        try_update_program(shader_b);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(program_is_valid(shader_a)) {
            use_program(shader_a);
            check_gl_error("Use program 'data'");
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            check_gl_error("Draw arrays 'data'");
            glUseProgram(0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // For reference see:
        // https://paroj.github.io/gltut/Texturing/SamplerBindingDiagram.svg
        // https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Binding_points

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, color_texture);
        glBindSampler(0, linearFilteringSampler);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glBindSampler(1, linearFilteringSampler);

        if(program_is_valid(shader_b)) {
            use_program(shader_b);
            check_gl_error("Use program 'color'");
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            check_gl_error("Draw arrays 'color'");
            glUseProgram(0);
        }

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, 0);

        SDL_GL_SwapWindow(window);

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN: {
                SDL_Keycode key = event.key.keysym.sym;
                if(key == SDLK_ESCAPE || key == SDLK_q) {
                    run = false;
                }
                break;
            }
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    size.w = event.window.data1;
                    size.h = event.window.data2;
                    glViewport(0, 0, size.w, size.h);

                    // Recreate framebuffer.
                    glDeleteFramebuffers(1, &framebuffer);
                    glDeleteTextures(1, &color_texture);

                    color_texture = create_texture(size, GL_RGBA);
                    depth_texture = create_texture(size, GL_RED);
                    framebuffer = create_framebuffer(color_texture, depth_texture);
                }
            default:
                break;
            }
        }
    }

    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &color_texture);
    glDeleteTextures(1, &depth_texture);
    glDeleteSamplers(1, &linearFilteringSampler);

    delete_program(shader_a);
    delete_program(shader_b);

    glDeleteVertexArrays(1, &vao);

    /* delete_textures(textures); */

    // Close window.
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    // Quit SDL.
    SDL_Quit();

    return 0;
}
