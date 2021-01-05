#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aOffset;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;

// uniform int tile_size;
// uniform int chunk_size;

// vec3 tPos(int instance) {
// 	int columns = chunk_size / tile_size;
// 	int column = columns % instance;
// 	int row = instance / columns + int(instance % columns != 0);

// 	return vec3(aPos.x + column * tile_size, aPos.y + row * tile_size, aPos.z);
// }

void main() {
    FragPos = vec3(transform * vec4(aPos + vec3(aOffset, 0.0), 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoord = aTexCoord;
}