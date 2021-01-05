#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D sprite;

void main() {
    vec4 texColor = texture(sprite, TexCoord);
    FragColor = texColor;
}