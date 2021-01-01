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
| uResolution | vec2 | Width and height of the framebuffer in pixels.      |
| uTime       | float | The elapsed time since the application was started. |


## Shader constants and functions

shdy pre-defines some useful constants and functions that can be used in the target shader.

```glsl
// Constants that are available:
const float PI = 3.14159265359;
const float TWOPI = 6.28318530718;

// Functions that are available:

// Transforms the given fragCoord from pixels into a normalized form for a landscape orientation.
// The normalized form is in the range Y = [-1.0..+1.0] and X will differ based on the width.
vec2 shdyNormCoordLandscape(in vec2 fragCoord);

// Transforms the given fragCoord from pixels into a normalized form for a portrait orientation.
// The normalized form is in the range X = [-1.0..+1.0] and Y will differ based on the height.
vec2 shdyNormCoordPortrait(in vec2 fragCoord);

// Translates the point to the given position.
vec2 shdyTranslate2d(in vec2 p, in vec2 t);

// Returns a 2d rotation matrix.
mat2 shdyRotMat2d(in float angle);

// Rotates the point around z-axis (2D rotation) by the given angle.
vec2 shdyRotate2d(in vec2 p, in float angle);

// Returns a 2d scale matrix.
mat2 shdyScaleMat2d(in vec2 scale);

// Scales the point by the given scale.
vec2 shdyScale2d(in vec2 p, in vec2 scale);

// Returns a pseudorandom float from a given 2d point.
highp float shdyRand2d(in vec2 p);

// Returns 2d value noise.
float shdyNoise2d(in vec2 p);

// Returns 2d fractal value noise.
float shdyFracNoise2d(in vec2 p, in int octaves);
```