#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shinyness;
};
in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
out vec4 FragColor;
uniform sampler2D input_texture;
uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 view_pos;
uniform float highlight;
uniform Material material;

float linear_depth(float d) {
    float far = 100.0;
    float depth = d / far;
    float near = 0.1;
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    float distance = length(light_position - Position);
    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * (distance * distance));

    float ambient_strength = 0.4;
    vec3 ambient_light = ambient_strength * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(light_position - Position);

    float diffuse_strength = 0.4;
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse_light = diffuse_strength * diff * material.diffuse;

    float specular_strength = 0.4;
    vec3 view_dir = normalize(view_pos - Position);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shinyness);
    vec3 specular_light = specular_strength * spec * material.specular;

    //FragColor = vec4(vec3(linear_depth(gl_FragCoord.z)), 1.0);
    vec4 total_light = vec4((ambient_light + diffuse_light + specular_light) * attenuation, 1.0);
    FragColor = (highlight * total_light * texture(input_texture, TexCoord)) + (1 - highlight) * vec4(0.9, 0.1, 0.1, 1.0);
};
