//start with a triangle - done!
//then make it use an EBO - done!
//then add a texture - done!
//then add movement to the scene - done!
//then a full camera with controlls 
//next make structs and functions to do all of the above things automatically and easily. In the end it should be so easy that loading models does not add significantly more code or require changes. This is the equivialent of creating a shader class, camera class etc.
//then load models
//finally, use the above to experiment with lighting, use it as a basis to complete the rest of learnopnegl. This may end up being a bigger project than expected..
#include <cglm/mat4.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "./cglm/cam.h"
#include "./cglm/vec3.h"
#include "./cglm/cglm.h"
#include "./cglm/affine-pre.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void process_input(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void window_resize_callback(GLFWwindow * window, int width, int height) {
	glViewport(0,0,width, height);	
}

void print_error(char * error) {
	printf("%s\n", error);
	exit(0);
}

void check_program_success(unsigned int program) {
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char error_buffer[512];
		glGetProgramInfoLog(program, 512, NULL, error_buffer);
		printf("%s\n", error_buffer);
		print_error("Bad program.");
	}
}

void check_shader_success(unsigned int shader, char * error_message) {
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char error_buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, error_buffer);
		printf("%s\n", error_buffer);
		print_error(error_message);
	}
}

int main() {
	int texture_width, texture_height, texture_channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char * texture_data = stbi_load("./wall.jpg", &texture_width, &texture_height, &texture_channels, 0);

	if (!texture_data)
		print_error("Failed to load texture.");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(1920, 1080, "Shader display", NULL, NULL);

	if (NULL == window)
		print_error("Failed to create a window.");	

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, window_resize_callback);
	glfwPollEvents();
	glViewport(0,0, 1920, 1080);	

	glewExperimental = GL_TRUE;
	glewInit();

	float vertices[] = {
	     0.5f,  0.5f, -1.0f,  1.0, 1.0,// top right
	     0.5f, -0.5f, -1.0f,  1.0, 0.0,// bottom right
	    -0.5f, -0.5f, -1.0f,  0.0, 0.0,// bottom left
	    -0.5f,  0.5f, -1.0f,   0.0, 1.0// top left 
	};

	unsigned int indices [] = {
	    0, 1, 3,   // first triangle
	    1, 2, 3    // second triangle
	} ;
	
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const char * vertex_shader_source = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec2 aTexCoord;\n"
	"uniform mat4 transform;\n"
	"uniform mat4 view;\n"
	"uniform mat4 project;\n"
	"out vec2 TexCoord;\n"
	"void main()\n"
	"{\n"
	"gl_Position = project*transform*vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" //project*view*
	"TexCoord = aTexCoord;\n"
	"}\0";

	const char * fragment_shader_source = "#version 330 core\n"
	"in vec2 TexCoord;\n"
	"out vec4 FragColor;\n"
	"uniform sampler2D input_texture;\n"
	"void main()\n"
	"{\n"
	"FragColor = texture(input_texture,TexCoord);\n"
	"}\0";

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	check_shader_success(vertex_shader, "Bad vertex shader.");

	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	check_shader_success(fragment_shader, "Bad fragment shader.");

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);	
	check_program_success(shader_program);	
	
	glDeleteShader(vertex_shader);	
	glDeleteShader(fragment_shader);	

	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(texture_data);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	vec3 camera_pos = {0.0, 0.0, 3.0};
	vec3 camera_target = {0.0, 0.0, 0.0};
	vec3 camera_direction;
	glm_vec3_sub(camera_direction, camera_pos, camera_target);
	glm_vec3_normalize(camera_direction);
	vec3 up = {0.0, 1.0, 0.0};
	vec3 camera_right;
	glm_vec3_cross(up, camera_direction, camera_right);
	glm_vec3_normalize(camera_right);
	mat4 look_at; 
	glm_lookat(camera_pos, camera_target, up, look_at);
		
	mat4 projection;
	glm_perspective(glm_rad(45.0f), 1.2f, 0.1f, 100.0f, projection);

	mat4 trans;
	vec4 axis = {0.0, 0.0, 1.0};
	glm_mat4_identity(trans);
	glm_rotate(trans, 0, axis);
	glm_translate_z(trans, -1.0);

	mat4 unit;
	glm_mat4_identity(unit);

	glUseProgram(shader_program);
	unsigned int transform_location = glGetUniformLocation(shader_program, "transform");
	unsigned int view_location = glGetUniformLocation(shader_program, "view");
	unsigned int project_location = glGetUniformLocation(shader_program, "project");
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, (float *)trans);
	glUniformMatrix4fv(view_location, 1, GL_FALSE, (float *)look_at);
	glUniformMatrix4fv(project_location, 1, GL_FALSE, (float *)projection);

	glEnable(GL_DEPTH_TEST);

	while(!glfwWindowShouldClose(window)) {
		process_input(window);
		glClearColor(0.9, 0.2, 0.2, 0.8);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_program);
		glUniform1i(glGetUniformLocation(shader_program, "input_texture"), 0);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,texture);
		
		float time = ((float)glfwGetTime())/10;
		glm_rotate(trans, ((float)glfwGetTime())/10000, axis);
		glUniformMatrix4fv(transform_location, 1, GL_FALSE, (float *)trans);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	glfwTerminate();

}
