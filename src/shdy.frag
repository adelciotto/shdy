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

// 2d transformations.

vec2 shdyTranslate2d(in vec2 p, in vec2 t) {
    return p - t;
}

mat2 shdyRotMat2d(in float angle) {
    return mat2(cos(angle), sin(angle),
                -sin(angle), cos(angle));
}

vec2 shdyRotate2d(in vec2 p, in float angle) {
    return p*shdyRotMat2d(angle);
}

mat2 shdyScaleMat2d(in vec2 scale) {
    return mat2(scale.x, 0.0,
                0.0, scale.y);
}

vec2 shdyScale2d(in vec2 p, in vec2 scale) {
    return p*shdyScaleMat2d(scale);
}

// RNG and noise functions.

// Returns a pseudorandom float from a given 2d point.
highp float shdyRand2d(in vec2 p) {
    const highp float a = 12.9898;
    const highp float b = 78.233;
    const highp float ampl = 43758.5453;
    highp float freq = dot(p, vec2(a, b));
    return fract(sin(freq)*ampl);
}

// Returns 2d value noise.
float shdyNoise2d(in vec2 p) {
    vec2 i = floor(p);
    vec2 f = smoothstep(0.0, 1.0, fract(p));

    float bl = shdyRand2d(i);
    float br = shdyRand2d(i + vec2(1.0, 0.0));
    float b = mix(bl, br, f.x);

    float tl = shdyRand2d(i + vec2(0.0, 1.0));
    float tr = shdyRand2d(i + vec2(1.0, 1.0));
    float t = mix(tl, tr, f.x);

    return mix(b, t, f.y);
}

// Returns 2d fractal value noise.
float shdyFracNoise2d(in vec2 p, in int octaves) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100);

    mat2 rot = shdyRotMat2d(0.5);

    for (int i = 0; i < octaves; i++) {
        v += a*shdyNoise2d(p);
        p = rot*p*2.0 + shift;
        a *= 0.5;
    }

    return v;
}

#line 0
)"
