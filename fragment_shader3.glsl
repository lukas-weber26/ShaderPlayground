#version 330 core

in vec2 TexCoord;
out vec4 FragColor;
//uniform sampler2D input_texture;

void main()
{
    FragColor = vec4(0.0, 0.25, 1.0, 1.0); //texture(input_texture, TexCoord);
};
