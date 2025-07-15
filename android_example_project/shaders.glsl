@vs vs
@uniform mat4 mvp ModelViewProjection
@attribute vec4 position Position
@attribute vec4 color0 Color0
@varying vec4 color
void main() {
    gl_Position = mvp * position;
    color = color0;
}
@end

@fs fs
@varying vec4 color
void main() {
    gl_FragColor = color;
}
@end

@program Cube vs fs