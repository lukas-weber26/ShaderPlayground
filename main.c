//next make structs and functions to do all of the above things automatically and easily. In the end it should be so easy that loading models does not add significantly more code or require changes. This is the equivialent of creating a shader class, camera class etc.
//then load models 
//finally, use the above to experiment with lighting, use it as a basis to complete the rest of learnopnegl. This may end up being a bigger project than expected..
#include "./shader_viewer.h"
#include <stdio.h>

camera global_camera = {{0.0, 0.0, 3.0}, {0.0,0.0, -1.0}, {0.0, 1.0, 0.0}, 1.2};

void camera_get_look_at(mat4 destination) {
	vec3 center;
	glm_vec3_add(global_camera.position, global_camera.front, center);
	glm_lookat(global_camera.position, center, global_camera.up, destination);
}

void camera_get_projection(mat4 destination) {
	glm_perspective(glm_rad(45.0f), global_camera.aspect_ratio, 0.1f, 100.0f, destination);
}

frame_time frame_time_create() {
	frame_time result = {0.0, 0.0};
	return result;
}

void frame_time_update(frame_time *timer) {
	float current_frame = glfwGetTime();
	timer->delta_time = current_frame - timer->last_frame;
	timer->last_frame = current_frame;
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos) {
	static int first_mouse = true;
	static float lastX = 400;
	static float lastY = 400;
	static float pitch = 0;
	static float yaw = 0;
	float xoffset = (xpos - lastX) * 0.01;
	float yoffset = (ypos - lastY) * 0.01;

	if (first_mouse) {
		lastX = xpos;
		lastY = ypos;
		first_mouse = false;
	}

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0)
		pitch = 89.0;
	if (pitch < -89.0)
		pitch = -89.0;

	vec3 direction;
	direction[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
	direction[1] = -glm_rad(pitch);
	direction[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
	glm_normalize_to(direction, global_camera.front);

	lastX = xpos;
	lastY = ypos;
}

void process_input(GLFWwindow * window, frame_time timer) {
	const float camera_speed = 2.0f * timer.delta_time;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		vec3 scaled_camera_front;
		glm_vec3_scale(global_camera.front, 1.5*camera_speed, scaled_camera_front);
		glm_vec3_add(global_camera.position, scaled_camera_front, global_camera.position);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		vec3 scaled_camera_front;
		glm_vec3_scale(global_camera.front, camera_speed, scaled_camera_front);
		glm_vec3_sub(global_camera.position, scaled_camera_front, global_camera.position);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		vec3 cross;
		glm_cross(global_camera.front, global_camera.up, cross);	
		glm_vec3_normalize(cross);
		glm_vec3_scale(cross, camera_speed, cross);
		glm_vec3_sub(global_camera.position, cross, global_camera.position);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		vec3 cross;
		glm_cross(global_camera.front, global_camera.up, cross);	
		glm_vec3_normalize(cross);
		glm_vec3_scale(cross, camera_speed, cross);
		glm_vec3_add(global_camera.position, cross, global_camera.position);
	}

}

void window_resize_callback(GLFWwindow * window, int width, int height) {
	glViewport(0,0,width, height);	
	global_camera.aspect_ratio = ((float)width)/((float)height);
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

const char * shader_load_from_file(char * path, char * shader_source) {
	size_t ret;
	FILE * shader_file = fopen(path, "r");	
	if (NULL == shader_file)
		print_error("Could not open shader file.");	
	ret = fread(shader_source, sizeof(char), 1024, shader_file);
	shader_source[ret] = '\n';
	shader_source[ret+1] = '\0';
	fclose(shader_file);
	return shader_source;
}

unsigned int shader_program_create(char * vertex_path, char * fragment_path) {

	char shader_buffer[1024]; 

	const char * vertex_shader_source_submit = shader_load_from_file(vertex_path, shader_buffer); 
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source_submit, NULL);
	glCompileShader(vertex_shader);
	check_shader_success(vertex_shader, "Bad vertex shader.");

	const char * fragment_shader_source_submit = shader_load_from_file(fragment_path, shader_buffer); 
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source_submit, NULL);
	glCompileShader(fragment_shader);
	check_shader_success(fragment_shader, "Bad fragment shader.");

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);	
	check_program_success(shader_program);	
	
	glDeleteShader(vertex_shader);	
	glDeleteShader(fragment_shader);	

	return shader_program;
}

void shader_program_activate(unsigned int shader_program) {
	glUseProgram(shader_program);
}

void shader_program_set_uniform_mat4(unsigned int shader_program, char * uniform_name, mat4 uniform_value) {
	unsigned int uniform_location = glGetUniformLocation(shader_program, uniform_name);
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (float *)uniform_value);
}

void shader_program_set_texture(unsigned int shader_program, char * uniform_name) {
	unsigned int uniform_location = glGetUniformLocation(shader_program, "input_texture");	
	glUniform1i(uniform_location, 0); //this zero coresonponds to GL_TEXTURE0
}

unsigned int texture_create(char * texture_path) {
	unsigned int texture;
	int texture_width, texture_height, texture_channels;
	unsigned char * texture_data = stbi_load(texture_path, &texture_width, &texture_height, &texture_channels, 0);

	if (!texture_data)
		print_error("Failed to load texture.");

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0); //indicated you are working with texture 0
	glBindTexture(GL_TEXTURE_2D, texture); //binds your texture to the active texture unit, in this case texture 0
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(texture_data);
	return texture;
}

GLFWwindow * window_create() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(1920, 1080, "Shader display", NULL, NULL);

	if (NULL == window)
		print_error("Failed to create a window.");	

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, window_resize_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwPollEvents();
	glViewport(0,0, 1920, 1080);	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	glewInit();
	stbi_set_flip_vertically_on_load(true);

	return window;
} 

int main() {
	
	GLFWwindow * window = window_create();
	frame_time timer = frame_time_create();

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

	unsigned int shader_program = shader_program_create("./vertex_shader.glsl", "./fragment_shader.glsl");
	unsigned int texture = texture_create("./wall.jpg");

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);

	mat4 look_at; 
	mat4 projection;
	camera_get_look_at(look_at);
	camera_get_projection(projection);

	mat4 trans;
	vec4 axis = {0.0, 0.0, 1.0};
	glm_mat4_identity(trans);
	glm_rotate(trans, 0, axis);
	glm_translate_z(trans, -1.0);

	shader_program_activate(shader_program);	
	shader_program_set_texture(shader_program, "input_texture");
	shader_program_set_uniform_mat4(shader_program, "transform", trans);	
	shader_program_set_uniform_mat4(shader_program, "view", look_at);	
	shader_program_set_uniform_mat4(shader_program, "project", projection);	

	glEnable(GL_DEPTH_TEST);

	while(!glfwWindowShouldClose(window)) {

		frame_time_update(&timer);
		process_input(window, timer);

		glClearColor(0.9, 0.2, 0.2, 0.8);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_program_activate(shader_program);	
		shader_program_set_texture(shader_program, "input_texture");

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		//some sort of camera class?
		camera_get_look_at(look_at);
		camera_get_projection(projection);
	
		shader_program_set_uniform_mat4(shader_program, "transform", trans);	
		shader_program_set_uniform_mat4(shader_program, "view", look_at);	
		shader_program_set_uniform_mat4(shader_program, "project", projection);	

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	glfwTerminate();

}
