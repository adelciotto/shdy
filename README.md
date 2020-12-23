# shdy

My personal CLI tool for live editing GLSL shaders, and saving high resolution screenshots for printed artwork.
Not really suitable for use by others as it is very much personalised for my setup. But, some of the code may be useful
to others.

## Usage

To live edit the shader and see changes as you save:

```shell
shdy --shader [FILEPATH]
```

Where `[FILEPATH]` is the path to your shader (e.g `~/shaders/shader.frag`).

To save a high resolution screenshot for printing:

```shell
shdy --shader [FILEPATH] --print-size [PRINTSIZE] --output [FILEPATH]
```

So for example:

```shell
shdy --shader ~/shaders/shader.frag --print-size A3-300dpi --output ~/Pictures/art/shader_A3_300DPI.png
```

To print usage information:

```shell
shdy --help
```

Here are all the CLI options available.

| Option           | Argument      | Description                                                                                                                      | Required | Default          |
|------------------|---------------|----------------------------------------------------------------------------------------------------------------------------------|----------|------------------|
| -s, --shader     | string        | Sets the fragment shader to run.                                                                                                 | YES      | N/A              |
| -w, --width      | unsigned int  | Sets the width of the window.                                                                                                    | NO       | 1280             |
| -h, --height     | unsigned int  | Sets the height of the window.                                                                                                   | NO       | 720              |
| -f, --fullscreen | NONE          | Sets the window to fullscreen.                                                                                                   | NO       | Disabled         |
| -p, --print-size | string        | Sets the size for output image used for printing. Can be one of the following values: 720p, 1080p, 4k, 5k, A3-150dpi, A3-300dpi  | NO       | Disabled         |
| -o, --output     | string        | Sets the output path for the image used for printing.                                                                            | NO       | "shdy_print.png" |

## Shader uniforms

| Name        | Type  | Description                                         |
|-------------|-------|-----------------------------------------------------|
| uResolution | ivec2 | Width and height of the framebuffer in pixels.      |
| uTime       | float | The elapsed time since the application was started. |
