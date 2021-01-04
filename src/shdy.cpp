//
// Created by anthonydelciotto on 20/12/20.
//

// TODO:
// - Better shader error output.
// - Better OpenGL debug output.
// - Draw compile status and frame metrics on screen.

#include "shdy.h"
#include <cstdlib>
#include <cassert>
#include <getopt.h>
#include <cctype>
#include <cstring>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#include <glad/glad.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

static const char * s_shared_shader_src =
#include "shdy.frag"
;

static void glfw_error_callback(int error, const char *description) {
    ERRORF("GLFW error %d: %s\n", error, description);
}

static void glfw_window_size_callback(GLFWwindow *win, int w, int h) {
    auto *window = (Window*)glfwGetWindowUserPointer(win);
    window->win_width = w;
    window->win_height = h;
}

static void glfw_framebuffer_size_callback(GLFWwindow *win, int w, int h) {
    auto *window = (Window*)glfwGetWindowUserPointer(win);
    window->fb_width = w;
    window->fb_height = h;
}

static void APIENTRY gl_error_callback(GLenum source,
                                       GLenum type,
                                       unsigned int id,
                                       GLenum severity,
                                       GLsizei length,
                                       const char *message,
                                       const void *userParam) {
    /* Ignore non-significant error/warning codes. */
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    /* TODO: Print more details */
    ERRORF("OpenGL error: %s\n", message);
}

static void init_gl_debug() {
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_error_callback, nullptr);
        glDebugMessageControl(GL_DEBUG_SOURCE_API,
                              GL_DEBUG_TYPE_ERROR,
                              GL_DEBUG_SEVERITY_HIGH,
                              0, nullptr, GL_TRUE);

        INFOF("Initialized OpenGL debug output.\n");
    } else {
        ERRORF("Failed to initialize OpenGL debug output.\n");
    }
}

void window_create(Window *window, const char *title, int width, int height, bool fullscreen, bool hidden) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    if (hidden) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    GLFWwindow *glfw_win = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (glfw_win == nullptr) {
        exit(EXIT_FAILURE);
    }

    if (!hidden && fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        if (monitor == nullptr) {
            exit(EXIT_FAILURE);
        }
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        if (mode == nullptr) {
            exit(EXIT_FAILURE);
        }
        glfwSetWindowMonitor(glfw_win, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    glfwMakeContextCurrent(glfw_win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ERRORF("Failure from call to gladLoadGLLoader().\n");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    init_gl_debug();
#endif

    glfwSwapInterval(1);

    int win_width, win_height;
    int fb_width, fb_height;
    glfwGetWindowSize(glfw_win, &win_width, &win_height);
    glfwGetFramebufferSize(glfw_win, &fb_width, &fb_height);

    window->win_width = win_width;
    window->win_height = win_height;
    window->fb_width = fb_width;
    window->fb_height = fb_height;
    window->fullscreen = fullscreen;
    window->hidden = hidden;
    window->glfw_win = glfw_win;

    glfwSetWindowUserPointer(glfw_win, window);
    glfwSetWindowSizeCallback(glfw_win, glfw_window_size_callback);
    glfwSetFramebufferSizeCallback(glfw_win, glfw_framebuffer_size_callback);

    INFOF("Rendering with OpenGL. Version: %d.%d, vendor: %s, renderer: %s.\n",
          GLVersion.major, GLVersion.minor, glGetString(GL_VENDOR), glGetString(GL_RENDERER));
}

void window_destroy(Window *window) {
    if (window->glfw_win != nullptr) {
        glfwDestroyWindow(window->glfw_win);
    }

    glfwTerminate();
}

bool window_is_open(Window *window) {
    assert(window->glfw_win != nullptr);

    return !glfwWindowShouldClose(window->glfw_win);
}

void window_update(Window *window) {
    assert(window->glfw_win != nullptr);

    glfwPollEvents();
    glfwSwapBuffers(window->glfw_win);
}

float get_elapsed_time() {
    return (float)glfwGetTime();
}

static const char* s_vert_shader_src =
        "#version 330\n"
        "layout (location = 0) in vec2 vPos;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(vPos, 0.0, 1.0);\n"
        "}\n";

static void log_shader_error(const char *msg, unsigned int id) {
    int info_log_length = 0;
    int max_len = info_log_length;

    if (glIsShader(id)) {
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &max_len);
    } else if (glIsProgram(id)) {
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_len);
    } else {
        return;
    }

    char info_log_buffer[max_len];

    if (glIsShader(id)) {
        glGetShaderInfoLog(id, max_len, &info_log_length, info_log_buffer);
    } else {
        glGetProgramInfoLog(id, max_len, &info_log_length, info_log_buffer);
    }

    if (info_log_length > 0) {
        ERRORF("%s:\n%s", msg, info_log_buffer);
    }
}

static bool compile_shader(GLenum type, const char *src[], unsigned int count, unsigned int *out) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, count, src, nullptr);
    glCompileShader(shader);

    *out = shader;

    int compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        return false;
    }

    return true;
}

static char *read_file(const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (fp == nullptr) {
        ERRORF("Failed to fopen() file %s.\n", filepath);
        exit(EXIT_FAILURE);
    }

    if (fseek(fp, 0, SEEK_END) < 0) {
        ERRORF("Failed to fseek() to end of file %s.\n", filepath);
        exit(EXIT_FAILURE);
    }
    long file_size = ftell(fp);
    if (file_size == -1L) {
        ERRORF("Failed to ftell() file %s.\n", filepath);
        exit(EXIT_FAILURE);
    }
    rewind(fp);

    char *buffer = (char *)calloc(file_size + 1, sizeof(char));
    if (buffer == nullptr) {
        ERRORF("Failed to malloc() file contents for %s.\n", filepath);
        exit(EXIT_FAILURE);
    }

    size_t result = fread(buffer, 1, file_size, fp);
    if ((long)result != file_size) {
        ERRORF("Failed to fread() file %s.\n", filepath);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
    return buffer;
}

void shader_create(Shader *shader, const char *user_frag_shader_path) {
    unsigned int vert_shader;
    if (!compile_shader(GL_VERTEX_SHADER, (const GLchar **)&s_vert_shader_src, 1, &vert_shader)) {
        log_shader_error("Failed to compile vertex shader.", vert_shader);
        exit(EXIT_FAILURE);
    }

    shader->user_frag_shader_path = user_frag_shader_path;
    shader->vert_shader = vert_shader;
    shader->compiled = false;

    shader_compile(shader);
}

static void link_program(unsigned int program, unsigned int vert_shader, unsigned int frag_shader) {
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);

    glLinkProgram(program);
    int linked = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        log_shader_error("Failed to link shader program.", program);
        exit(EXIT_FAILURE);
    }
}

void shader_compile(Shader *shader) {
    char *user_frag_shader_src = read_file(shader->user_frag_shader_path);
    const char *frag_shader_srcs[] = {
        s_shared_shader_src,
        user_frag_shader_src
    };
    unsigned int frag_shader;
    if (!compile_shader(GL_FRAGMENT_SHADER, frag_shader_srcs, 2, &frag_shader)) {
        log_shader_error("Failed to compile fragment shader.", frag_shader);
        free(user_frag_shader_src);
        return;
    }
    free(user_frag_shader_src);

    INFOF("Shader %s compiled successfully.\n", shader->user_frag_shader_path);

    if (shader->compiled) {
        glDeleteProgram(shader->program);
        shader->compiled = false;
    }

    unsigned int program = glCreateProgram();
    link_program(program, shader->vert_shader, frag_shader);

    glDeleteShader(frag_shader);

    shader->uniform_resolution_loc = glGetUniformLocation(program, "uResolution");
    shader->uniform_elapsed_time_loc = glGetUniformLocation(program, "uTime");
    shader->program = program;
    shader->compiled = true;
}

void shader_set_uniform_resolution(Shader *shader, int width, int height) {
    assert(shader->compiled);

    glUniform2f(shader->uniform_resolution_loc, (float)width, (float)height);
}

void shader_set_uniform_elapsed_time(Shader *shader, float elapsed_time) {
    assert(shader->compiled);

    glUniform1f(shader->uniform_elapsed_time_loc, elapsed_time);
}

static const char *print_size_str_tbl[] = {
        "",             // PRINTING_DISABLED
        "720p",         // PRINT_SIZE_720P
        "1080p",        // PRINT_SIZE_1080P
        "4k",           // PRINT_SIZE_4K
        "5k",           // PRINT_SIZE_5K
        "A3-150dpi",    // PRINT_SIZE_A3_150DPI
        "A3-300dpi"     // PRINT_SIZE_A3_300DPI
};

static PrintSize print_size_from_str(const char *str) {
    for (int i = 0; i < ARRAY_LEN(print_size_str_tbl); i++) {
        if (strcmp(str, print_size_str_tbl[i]) == 0) {
            return (PrintSize)i;
        }
    }

    ERRORF("Print size %s is not valid.\n", str);

    return PRINTING_DISABLED;
}

void print_size_get_dimensions(PrintSize print_size, int *out_width, int *out_height) {
    switch (print_size) {
        case PRINT_SIZE_720p:
            *out_width = 1280;
            *out_height = 720;
            break;
        case PRINT_SIZE_1080P:
            *out_width = 1920;
            *out_height = 1080;
            break;
        case PRINT_SIZE_4K:
            *out_width = 3840;
            *out_height = 2160;
            break;
        case PRINT_SIZE_5K:
            *out_width = 5120;
            *out_height = 2160;
            break;
        case PRINT_SIZE_A3_150DPI:
            *out_width = 2480;
            *out_height = 1754;
            break;
        case PRINT_SIZE_A3_300DPI:
            *out_width = 4960;
            *out_height = 3508;
            break;
        case PRINTING_DISABLED:
        default:
            *out_width = 0;
            *out_height = 0;
            break;
    }
}

static const float s_quad_verts[] = {
    1.0f, 1.0f,   // top-right
    1.0f, -1.0f,  // bottom-right
    -1.0f, -1.0f, // bottom-left
    -1.0f, 1.0f   // top-left
};

static const unsigned int s_quad_elements[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

void shader_renderer_create(ShaderRenderer *shader_renderer, const char *frag_shader_path) {
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_quad_verts), s_quad_verts, GL_STATIC_DRAW);

    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_quad_elements), s_quad_elements, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);

    shader_renderer->vao = vao;
    shader_renderer->vbo = vbo;
    shader_renderer->ebo = ebo;
    shader_create(&shader_renderer->shader, frag_shader_path);
}

static void shader_renderer_print_begin(ShaderRenderer *shader_renderer) {
    INFOF("Rendering shader for print with dimensions %dx%d...\n", shader_renderer->width, shader_renderer->height);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int tbo;
    glGenTextures(1, &tbo);
    glBindTexture(GL_TEXTURE_2D, tbo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, shader_renderer->width, shader_renderer->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ERRORF("Failure in call to glCheckFrameBufferStatus() returned framebuffer not complete\n");
        exit(EXIT_FAILURE);
    }
}

static void shader_renderer_print_end(ShaderRenderer *shader_renderer, const char *output_path) {
    glFinish();

    int width = shader_renderer->width;
    int height = shader_renderer->height;
    int num_comp = 3;
    int stride = num_comp * width;

    char *buffer = (char *)calloc(num_comp, width * height);
    if (buffer == nullptr) {
        ERRORF("Failed to calloc() for image buffer.\n");
        exit(EXIT_FAILURE);
    }

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
      ERRORF("glReadPixels() failed with code: %d\n", err);
      exit(EXIT_FAILURE);
    }

    INFOF("Print rendering complete, writing pixel data to %s...\n", output_path);
    stbi_flip_vertically_on_write(true);
    int ret = stbi_write_png(output_path, width, height, num_comp, buffer, stride);
    if (ret == 0) {
        ERRORF("stbi_write_png() failed to write image to: %s\n", output_path);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    free(buffer);
    INFOF("Print written to %s successfully!\n", output_path);
}

void shader_renderer_draw(ShaderRenderer *shader_renderer, float elapsed_time) {
    glViewport(0, 0, shader_renderer->width, shader_renderer->height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_renderer->shader.program);
    shader_set_uniform_resolution(&shader_renderer->shader, shader_renderer->width, shader_renderer->height);
    shader_set_uniform_elapsed_time(&shader_renderer->shader, elapsed_time);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void shader_renderer_draw_to_print(ShaderRenderer *shader_renderer, const char *output_path) {
    assert(output_path != nullptr);

    shader_renderer_print_begin(shader_renderer);
    shader_renderer_draw(shader_renderer, 1.0f);
    shader_renderer_print_end(shader_renderer, output_path);
}

void shader_renderer_reload(ShaderRenderer *shader_renderer) {
    shader_compile(&shader_renderer->shader);
}

void file_watcher_create(FileWatcher *file_watcher, const char *filepath) {
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        ERRORF("Failure in call to inotify_init(): %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(fd, filepath, IN_CLOSE_WRITE);
    if (wd < 0) {
        ERRORF("Failure in call to inotify_add_watch(): %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    file_watcher->filepath = filepath;
    file_watcher->fd = fd;
    file_watcher->wd = wd;
    file_watcher->modified = false;

    INFOF("Watching file %s for changes...\n", filepath);
}

void file_watcher_destroy(FileWatcher *file_watcher) {
    inotify_rm_watch(file_watcher->fd, file_watcher->wd);
    close(file_watcher->fd);
}

void file_watcher_poll(FileWatcher *file_watcher) {
    struct pollfd pfd = {file_watcher->fd, POLLIN, 0};

    if (poll(&pfd, 1, 0) == 1) {
        if (pfd.revents & POLLIN) {
            int avail;
            ioctl(file_watcher->fd, FIONREAD, &avail);

            char buffer[avail];
            read(file_watcher->fd, buffer, avail);

            int offset = 0;
            while (offset < avail) {
                auto *event = (struct inotify_event*)(buffer + offset);

                if (event->mask & IN_CLOSE_WRITE) {
                    file_watcher->modified = true;
                    INFOF("File %s change detected!\n", file_watcher->filepath);
                }

                offset += sizeof(struct inotify_event) + event->len;
            }
        }
    } else {
        file_watcher->modified = false;
    }
}

static struct option long_options[] = {
        {"shader", required_argument, nullptr, 's'},
        {"width", required_argument, nullptr, 'w'},
        {"height", required_argument, nullptr, 'h'},
        {"fullscreen", no_argument, nullptr, 'f'},
        {"print-size", required_argument, nullptr, 'p'},
        {"output", required_argument, nullptr, 'o'},
        { "help", no_argument, nullptr, 'H'},
        { nullptr }
};

static int str_is_empty(const char *str) {
    return strlen(str) == 0;
}

static void print_help() {
    printf("shdy help\n");
    printf("Live edit a GLSL shader, or save a screenshot to a high resolution image for printing.\n");
    printf("\n");

    printf("Usage: shdy --shader [FILEPATH] [OPTION]...\n");
    printf("With no FILEPATH the application will quit with exit status -1.\n");
    printf("\n");

    printf("Options:\n");
    printf("--width [INTEGER]\t\tSets the width of the window.\n");
    printf("\t\t\t\tDefaults to 1280.\n");
    printf("--height [INTEGER]\t\tSets the height of the window.\n");
    printf("\t\t\t\tDefaults to 720.\n");
    printf("--fullscreen\t\t\tEnables fullscreen mode.\n");
    printf("\t\t\t\tDefaults to false.\n");
    printf("--print-size [PRINTSIZE]\tDraws shader for print with given PRINTSIZE.\n");
    printf("\t\t\t\tDefaults to printing disabled.\n");
    printf("\t\t\t\tValid values are 720p,1080p,4k,5k,A3-150dpi,A3-300dpi.\n");
    printf("--output [FILEPATH]\t\tSets the output image filepath for print.\n");
    printf("\t\t\t\tDefaults to shdy_print.png.\n");
}

static int opt_requires_arg(int opt) {
    return (opt == 's' || opt == 'w' || opt == 'h' || opt == 'o' || opt == 'p');
}

void cli_opts_parse(CliOpts *cli_opts, int argc, char **argv) {
    CliOpts opts = {
            nullptr,
            CLI_OPTS_DEFAULT_WIDTH,
            CLI_OPTS_DEFAULT_HEIGHT,
            CLI_OPTS_DEFAULT_FULLSCREEN,
            CLI_OPTS_DEFAULT_PRINT_SIZE,
            CLI_OPTS_DEFAULT_OUTPUT_IMAGE_PATH
    };

    opterr = 0;
    bool has_error = false;

    while (true) {
        char ch = getopt_long(argc, argv, "s:w:h:fp:o:H", long_options, nullptr);

        if (ch == -1) {
            break;
        }

        switch (ch) {
            case 's':
                if (str_is_empty(optarg)) {
                    ERRORF("Arg for shader filename is an empty string.\n");
                    has_error = true;
                }
                // TODO: Maybe validate '.frag' extension is used.
                opts.frag_shader_path = optarg;
                break;
            case 'w': {
                int width = atoi(optarg);
                if (width <= 0) {
                    ERRORF("Invalid arg for width: %s, must be a positive integer.\n", optarg);
                    has_error = true;
                    break;
                }
                opts.win_width = width;
                break;
            }
            case 'h': {
                int height = atoi(optarg);
                if (height <= 0) {
                    ERRORF("Invalid arg for height: %s, must be a positive integer.\n", optarg);
                    has_error = true;
                    break;
                }
                opts.win_height = height;
                break;
            }
            case 'f':
                opts.fullscreen = true;
                break;
            case 'p':
                if (str_is_empty(optarg)) {
                    ERRORF("Arg for print size is an empty string.\n");
                    has_error = true;
                }
                opts.print_size = print_size_from_str(optarg);
                break;
            case 'o':
                if (str_is_empty(optarg)) {
                    ERRORF("Arg for output image path is an empty string.\n");
                    has_error = true;
                }
                // TODO: Maybe validate '.png' extension is used.
                opts.output_image_path = optarg;
                break;
            case 'H':
                print_help();
                exit(EXIT_SUCCESS);
            case '?':
                if (opt_requires_arg(optopt)) {
                    ERRORF("Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    ERRORF("Unknown option -%c.\n", optopt);
                } else {
                    ERRORF("Unknown option character `\\x%x'.\n", optopt);
                }
                has_error = true;
                break;
        }

        if (has_error) {
            break;
        }
    }

    if (opts.frag_shader_path == nullptr) {
        ERRORF("Shader filepath option must be provided, e.g --shader [FILEPATH].\n");
        exit(EXIT_FAILURE);
    }

    cli_opts->frag_shader_path = opts.frag_shader_path;
    cli_opts->win_width = opts.win_width;
    cli_opts->win_height = opts.win_height;
    cli_opts->fullscreen = opts.fullscreen;
    cli_opts->print_size = opts.print_size;
    cli_opts->output_image_path = opts.output_image_path;
}
