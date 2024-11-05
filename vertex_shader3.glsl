#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 project;

out vec2 TexCoord;

void main()
{
    //gl_Position = project * view * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    TexCoord = aTexCoord;
};