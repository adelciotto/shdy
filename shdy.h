/*
 * Created by anthonydelciotto on 20/12/20.
*/

#ifndef SHDY_H
#define SHDY_H

#include <stdio.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define VERSION "1.0.0"

typedef enum { false, true } bool;

#define ERRORF(fmt, ...) \
    fprintf(stderr, "ERROR - %s() in %s, line %i: " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#define INFOF(fmt, ...) \
    printf("INFO - %s() in %s, line %i: " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#define ARRAY_LEN(arr) (int)((sizeof(arr)) / (sizeof(arr[0])))

typedef struct {
    int win_width;
    int win_height;
    int fb_width;
    int fb_height;
    bool fullscreen;
    bool hidden;
    GLFWwindow *glfw_win;
} Window;

void window_create(Window *window, int width, int height, bool fullscreen, bool hidden);
void window_destroy(Window *window);
bool window_is_open(Window *window);
void window_update(Window *window);

float get_elapsed_time(void);

typedef struct {
    const char *frag_shader_path;
    unsigned int program;
    unsigned int vert_shader;
    bool compiled;
    int uniform_resolution_loc;
    int uniform_elapsed_time_loc;
} Shader;

void shader_create(Shader *shader, const char *frag_shader_path);
void shader_compile(Shader *shader);
void shader_set_uniform_resolution(Shader *shader, int width, int height);
void shader_set_uniform_elapsed_time(Shader *shader, float elapsed_time);

typedef enum {
    PRINTING_DISABLED = 0,
    PRINT_SIZE_720p,
    PRINT_SIZE_1080P,
    PRINT_SIZE_4K,
    PRINT_SIZE_5K,
    PRINT_SIZE_A3_150DPI,
    PRINT_SIZE_A3_300DPI
} PrintSize;

void print_size_get_dimensions(PrintSize print_size, int *out_width, int *out_height);

typedef struct {
    int width;
    int height;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    Shader shader;
} ShaderRenderer;

void shader_renderer_create(ShaderRenderer *shader_renderer, const char *frag_shader_path);
void shader_renderer_draw(ShaderRenderer *shader_renderer, float elapsed_time);
void shader_renderer_draw_to_print(ShaderRenderer *shader_renderer, const char *output_path);
void shader_renderer_reload(ShaderRenderer *shader_renderer);

typedef struct {
    const char *filepath;
    int fd;
    int wd;
    bool modified;
} FileWatcher;

void file_watcher_create(FileWatcher *file_watcher, const char *filepath);
void file_watcher_destroy(FileWatcher *file_watcher);
void file_watcher_poll(FileWatcher *file_watcher);

#define CLI_OPTS_DEFAULT_WIDTH 1280
#define CLI_OPTS_DEFAULT_HEIGHT 720
#define CLI_OPTS_DEFAULT_FULLSCREEN false
#define CLI_OPTS_DEFAULT_PRINT_SIZE PRINTING_DISABLED
#define CLI_OPTS_DEFAULT_OUTPUT_IMAGE_PATH "shdy_print.png"

typedef struct {
    const char *frag_shader_path;  /* required */
    int win_width;                 /* optional */
    int win_height;                /* optional */
    bool fullscreen;               /* optional */

    PrintSize print_size;          /* optional */
    const char *output_image_path; /* optional */
} CliOpts;

void cli_opts_parse(CliOpts *cli_opts, int argc, char **argv);

#endif /*SHDY_H*/
