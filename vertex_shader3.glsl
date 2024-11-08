#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 project;
uniform mat4 normal_correction;

out vec2 TexCoord;
out vec3 Normal;
out vec3 Position;

void main()
{
    gl_Position = project * view * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    TexCoord = aTexCoord;
    Normal = mat3(normal_correction) * aNormal;
    Position = vec3(gl_Position.x, gl_Position.x, gl_Position.z);
};
