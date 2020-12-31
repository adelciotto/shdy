R"(
#version 330

out vec4 fragColor;

uniform vec2 uResolution;
uniform float uTime;

const float PI = 3.14159265359;
const float TWOPI = 6.28318530718;

// Transforms the given fragCoord from pixels into a normalized form for a landscape orientation.
// The normalized form is in the range Y = [-1.0..+1.0] and X will differ based on the width.
vec2 shdyNormCoordLandscape(in vec2 fragCoord) {
    return 2.0*(fragCoord - 0.5*uResolution) / uResolution.y;
}

// Transforms the given fragCoord from pixels into a normalized form for a portrait orientation.
// The normalized form is in the range X = [-1.0..+1.0] and Y will differ based on the height.
vec2 shdyNormCoordPortrait(in vec2 fragCoord) {
    return 2.0*(fragCoord - 0.5*uResolution) / uResolution.x;
}

// Returns a pseudorandom float from a given 2d point.
highp float shdyRand2d(in vec2 p) {
    const highp float a = 12.9898;
    const highp float b = 78.233;
    const highp float ampl = 43758.5453;
    highp float freq = dot(p, vec2(a, b));
    return fract(sin(freq)*ampl);
}

// 2d transformations.

vec2 shdyTranslate2d(in vec2 p, in vec2 t) {
    return p - t;
}

vec2 shdyRotate2d(in vec2 p, in float angle) {
    mat2 rot = mat2(cos(angle), sin(angle),
                    -sin(angle), cos(angle));
    return p*rot;
}

vec2 shdyScale2d(in vec2 p, in vec2 scale) {
    mat2 scl = mat2(scale.x, 0.0,
                    0.0, scale.y);
    return p*scl;
}
)"