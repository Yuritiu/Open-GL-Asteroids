#version 400 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

layout(std140) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};

out vec2 fragTexcoord;
out vec3 fragNormal;
out vec3 fragPosition;

void main() {
    vec4 worldPosition = model * vec4(position, 1.0);
    gl_Position = projection * view * worldPosition;
    fragTexcoord = texcoord;
    fragNormal = normalize(mat3(transpose(inverse(model))) * normal);
    fragPosition = worldPosition.xyz;
}
