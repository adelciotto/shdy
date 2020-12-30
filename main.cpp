//
// Created by anthonydelciotto on 20/12/20.
//

#include "shdy.h"
#include <cstdlib>

static Window s_window;
static FileWatcher s_file_watcher;

void exit_callback() {
    window_destroy(&s_window);
    file_watcher_destroy(&s_file_watcher);
}

int main(int argc, char **argv) {
    atexit(exit_callback);

    CliOpts cli_opts;
    cli_opts_parse(&cli_opts, argc, argv);

    bool print_mode = cli_opts.print_size != PRINTING_DISABLED;

    char *abs_path = realpath(cli_opts.frag_shader_path, nullptr);
    const char *title_fmt = "shdy: %s";
    int buf_size = snprintf(nullptr, 0, title_fmt, abs_path);
    char title[buf_size + 1];
    snprintf(title, buf_size + 1, title_fmt, abs_path);
    free(abs_path);

    window_create(&s_window, title, cli_opts.win_width, cli_opts.win_height, cli_opts.fullscreen, print_mode);

    ShaderRenderer shader_renderer;
    shader_renderer_create(&shader_renderer, cli_opts.frag_shader_path);

    if (print_mode) {
        int print_w, print_h;
        print_size_get_dimensions(cli_opts.print_size, &print_w, &print_h);

        shader_renderer.width = print_w;
        shader_renderer.height = print_h;

        shader_renderer_draw_to_print(&shader_renderer, cli_opts.output_image_path);
    } else {
        file_watcher_create(&s_file_watcher, cli_opts.frag_shader_path);

        while (window_is_open(&s_window)) {
            shader_renderer.width = s_window.fb_width;
            shader_renderer.height = s_window.fb_height;

            shader_renderer_draw(&shader_renderer, get_elapsed_time());

            window_update(&s_window);

            file_watcher_poll(&s_file_watcher);
            if (s_file_watcher.modified) {
                shader_renderer_reload(&shader_renderer);
            }
        }
    }

    return EXIT_SUCCESS;
}
