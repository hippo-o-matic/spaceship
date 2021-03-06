#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 aOffset;

out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;
uniform int layer;

void main() {
    FragPos = vec3(transform * vec4(aPos + aOffset, layer, 1.0));
    gl_Position = projection * mat4(mat3(view)) * vec4(FragPos, 1.0);
    TexCoord = aTexCoord;
}