#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;

void main() {
    FragPos = vec3(transform * vec4(aPos, 1.0));;
    gl_Position = projection * mat4(mat3(view)) * vec4(FragPos.xy, -2, 1.0);
    TexCoord = aTexCoord;
}