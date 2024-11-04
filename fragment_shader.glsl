#version 330 core

in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D input_texture;

void main()
{
    FragColor = texture(input_texture, TexCoord);
};
