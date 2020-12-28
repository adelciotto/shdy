out vec4 fragColor;

void main() {
    vec2 p = shdyNormCoordLandscape(gl_FragCoord.xy);

    vec3 backgroundCol = vec3(1.0);
    vec3 axesCol = vec3(1.0, 0.0, 0.0);
    vec3 gridCol = vec3(0.5);

    vec3 pixel = backgroundCol;

    const float cellWidth = 0.1;
    for (float i = -2.0; i < 2.0; i += cellWidth) {
        if ((abs(p.x - i) < 0.004) || (abs(p.y - i) < 0.004)) {
            pixel = gridCol;
        }
    }

    if (abs(p.x) < 0.006 || abs(p.y) < 0.006) {
        pixel = axesCol;
    }

    fragColor = vec4(pixel, 1.0);
}
