#include "shader_viewer.h"

#include <alloca.h>
#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void raise_error(char * error_string) {
	printf("%s\n", error_string);
	exit(0);
}


void performance_timer(char * string) {
	static long prev_seconds = 0; 
	static long prev_nanoseconds = 0;
	struct timespec current_time;
	clock_gettime(CLOCK_REALTIME, &current_time);
	long diff_seconds = current_time.tv_sec - prev_seconds;
	long diff_nanoseconds = current_time.tv_nsec - prev_nanoseconds;
	prev_seconds = current_time.tv_sec;
	prev_nanoseconds = current_time.tv_nsec;
	printf ("Timer: %s. Seconds: %ld, Nanoseconds: %ld\n", string, diff_seconds, diff_nanoseconds);
}

typedef	struct texture_catalouge {
	unsigned int * gl_texture_ids; 
	unsigned int * texture_indices; 
	int n_items;
	int max_items;
} texture_catalouge;  

texture_catalouge global_textures;

texture_catalouge texture_catalouge_create () {
	texture_catalouge result;
	result.n_items = 0;
	result.max_items = 100;
	result.gl_texture_ids= calloc(result.max_items, sizeof(unsigned int));
	result.texture_indices= calloc(result.max_items, sizeof(unsigned int));
	return result;
}

bool texture_catalouge_search(unsigned int texture_index, unsigned int *result) {
	for (int i = 0; i < global_textures.n_items; i ++) {
		if (global_textures.texture_indices[i] == texture_index) {
			*result = global_textures.gl_texture_ids[i];	
			return true; 
		}
	}
	return false;
}

void texture_catalouge_add(unsigned int gl_texture_id, unsigned int texture_index) {
	if (global_textures.n_items >= global_textures.max_items) {
		global_textures.max_items *= 2;
		global_textures.gl_texture_ids= realloc(global_textures.gl_texture_ids, sizeof(unsigned int) * global_textures.max_items);	
		global_textures.texture_indices= realloc(global_textures.texture_indices, sizeof(unsigned int) * global_textures.max_items);	
	}	
	global_textures.gl_texture_ids[global_textures.n_items] = texture_index;
	global_textures.texture_indices[global_textures.n_items] = gl_texture_id;
	global_textures.n_items ++;
}

void global_texture_catalouge_free() {
	for (int i = 0; i < global_textures.n_items; i ++) {
		const unsigned int delete_texture = global_textures.gl_texture_ids[i];
		glDeleteTextures(1,&delete_texture);
	}
	free(global_textures.texture_indices);
	free(global_textures.gl_texture_ids);
}

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

GLFWwindow * window_create() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(1920, 1080, "Shader display", NULL, NULL);

	if (NULL == window)
		raise_error("Failed to create a window.");	

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

typedef struct model {
	unsigned int mesh;
	float * vertices;
	int n_vertices;
	unsigned int * indices;
	int n_indices;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int texture;
} model;

typedef struct scene {
	const struct aiScene * model_data;
	struct model ** models;
	int n_models;
	int max_models;
	unsigned int shader_program;
	int n_indices;
	mat4 transform;
	mat4 view;
	mat4 project;
	mat4 lookat;
} scene;

void scene_add_model(scene * input_scene, model * new_model) {
	if (input_scene->n_models >= input_scene->max_models) {
		raise_error("Scene max model count exceed.");
		//input_scene->max_models *= 2;	
		//input_scene->models = realloc(input_scene->models, sizeof(model *) * input_scene->max_models);
	}
	input_scene->models[input_scene->n_models] = new_model;
	input_scene->n_models++;
}

model * model_create (scene * input_scene, unsigned int mesh_loc) {
	model * new_model = calloc(1,sizeof(model));
	new_model->mesh = mesh_loc;

	struct aiMesh * model_mesh = input_scene->model_data->mMeshes[mesh_loc];

	new_model->n_vertices = model_mesh->mNumVertices;
	new_model->vertices = calloc(8*new_model->n_vertices,sizeof(float));

	for (int i = 0; i < new_model->n_vertices; i ++) {
		new_model->vertices[8*i] = model_mesh->mVertices[i].x;
		new_model->vertices[8*i+1] = model_mesh->mVertices[i].y;
		new_model->vertices[8*i+2] = model_mesh->mVertices[i].z;
		new_model->vertices[8*i+3] = model_mesh->mNormals[i].x;
		new_model->vertices[8*i+4] = model_mesh->mNormals[i].y;
		new_model->vertices[8*i+5] = model_mesh->mNormals[i].z;
		new_model->vertices[8*i+6] = model_mesh->mTextureCoords[0][i].x; //0 corresponds to first texture
		new_model->vertices[8*i+7] = model_mesh->mTextureCoords[0][i].y;
	}

	new_model->n_indices = model_mesh->mNumFaces * 3; //3 corresponds to three indices per face for a triangle
	new_model->indices = calloc(new_model->n_indices,sizeof(unsigned int));

	//this loop can run at the same time as the previous
	for (int i = 0; i < model_mesh->mNumFaces; i ++) {
		for (int j = 0; j < 3; j++) {
			new_model->indices[3*i+j] = model_mesh->mFaces[i].mIndices[j];
		}
	}

	//texture
	unsigned int texture_index = model_mesh->mMaterialIndex; //this is ok
	unsigned int gl_texture;

	//cation, this condition is quite heavily loaded
	if (!texture_catalouge_search(texture_index, &new_model->texture)) { 
		//texture has to be generated
		struct aiMaterial * material = input_scene->model_data->mMaterials[texture_index]; 
		unsigned int texture_count = aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE);
		struct aiString texture_path;

		if (aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &texture_path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS) {

			int texture_width, texture_height, texture_channels;
			unsigned char * texture_data = stbi_load(texture_path.data, &texture_width, &texture_height, &texture_channels, 0);

			if (!texture_data) {
				raise_error("Failed to load texture.");
			}

			glGenTextures(1, &new_model->texture); 
			glActiveTexture(GL_TEXTURE0); 
			glBindTexture(GL_TEXTURE_2D, new_model->texture); 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(texture_data);
			texture_catalouge_add(new_model->texture, texture_index);


		} else {
			raise_error("No diffuse texture found");
		}
	}

	return new_model;
}

void scene_print_info(scene * new_scene) {
	printf("Scene loaded with %d models.\n", new_scene->n_models);
	for (int i = 0; i < new_scene->n_models; i ++) {
		printf("Model with %d vertices, and %d indices.\n", new_scene->models[i]->n_vertices, new_scene->models[i]->n_indices);
	}
}

void scene_add_meshes(scene * input_scene, const struct aiNode * node) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		model * new_model = model_create(input_scene, node->mMeshes[i]);
		scene_add_model(input_scene, new_model);
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		scene_add_meshes(input_scene, node->mChildren[i]);
	}
} 

scene * scene_create (char * model_name) {
	scene * new_scene = calloc(1, sizeof(scene));
	new_scene->model_data = aiImportFile(model_name, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (NULL == new_scene) {
		char * error_buff = (char *)alloca(sizeof(char)*512);
		sprintf(error_buff, "Failed to load model at %s.", model_name);
		raise_error(error_buff);
	}

	const struct aiNode * root_node = new_scene->model_data->mRootNode;

	if (NULL == root_node) {
		char * error_buff = (char *)alloca(sizeof(char)*512);
		sprintf(error_buff, "Failed to load model root node.");
		raise_error(error_buff);
	}
	new_scene->n_models = 0;
	new_scene->max_models = new_scene->model_data->mNumMeshes; //10;
	new_scene->models = calloc(new_scene->max_models , sizeof(model*));

	performance_timer("Starting to add meshes to scene.");
	scene_add_meshes(new_scene, root_node);	

	glm_mat4_identity(new_scene->transform); //placed here for my sanity
	return new_scene;
}

void model_generate_buffers(model * current_model, unsigned int VAO, unsigned int VBO, unsigned int EBO) {
	glBindVertexArray(VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, current_model->n_vertices*8*sizeof(float),current_model->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, current_model->n_indices*sizeof(unsigned int), current_model->indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	current_model->VAO = VAO;	
	current_model->VBO = VBO;	
	current_model->EBO = EBO;	
}

void scene_generate_buffers(scene * current_scene) {
	int n_models = current_scene->n_models;

	unsigned int *VAOS = calloc(n_models,sizeof(unsigned int));
	unsigned int *VBOS = calloc(n_models,sizeof(unsigned int));
	unsigned int *EBOS = calloc(n_models,sizeof(unsigned int));

	glGenVertexArrays(n_models,VAOS);
	glGenBuffers(n_models,VBOS);
	glGenBuffers(n_models,EBOS);

	for (int i = 0; i < n_models; i ++) {
		model_generate_buffers(current_scene->models[i], VAOS[i], VBOS[i], EBOS[i]);
	} 

	free(VAOS);
	free(VBOS);
	free(EBOS);
}

const char * shader_load_from_file(char * path, char * shader_source) {
	size_t ret;
	FILE * shader_file = fopen(path, "r");	
	if (NULL == shader_file) {
		char * error_buff = (char *)alloca(sizeof(char)*512);
		sprintf(error_buff, "Failed to load shader at %s.", shader_source);
		raise_error(error_buff);
	}
	ret = fread(shader_source, sizeof(char), 1024, shader_file);
	shader_source[ret] = '\n';
	shader_source[ret+1] = '\0';
	fclose(shader_file);
	return shader_source;
}

void check_program_success(unsigned int program) {
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char error_buffer[512];
		glGetProgramInfoLog(program, 512, NULL, error_buffer);
		printf("%s\n", error_buffer);
		raise_error("Bad program.");
	}
}

void check_shader_success(unsigned int shader, char * error_message) {
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char error_buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, error_buffer);
		printf("%s\n", error_buffer);
		raise_error(error_message);
	}
}

void scene_generate_shaders(scene * current_scene, char * vertex_path, char * fragment_path) {
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

	current_scene->shader_program = shader_program;
}

void model_draw(model * draw_model, unsigned int shader_program) {
	glBindVertexArray(draw_model->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_model->EBO);

	//belive that this is needed on evey draw	
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, draw_model->texture); 
	unsigned int uniform_location = glGetUniformLocation(shader_program, "input_texture");	
	glUniform1i(uniform_location, 0); 
	
	glDrawElements(GL_TRIANGLES, draw_model->n_indices, GL_UNSIGNED_INT, 0);
	
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		printf("Error detected: %x\n", error);
	}
}

void shader_program_set_uniform_mat4(unsigned int shader_program, char * uniform_name, mat4 uniform_value) {
	unsigned int uniform_location = glGetUniformLocation(shader_program, uniform_name);
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (float *)uniform_value);
}

void print_mat4(mat4 matrix) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%f ", matrix[i][j]);
		} 	
		printf("\n");
	} 	
	printf("\n");
}

void scene_draw_objects(scene * draw_scene) {
	
	glUseProgram(draw_scene->shader_program);
	int n_models = draw_scene->n_models;

	camera_get_look_at(draw_scene->lookat);
	camera_get_projection(draw_scene->project);

	//these vertices are messed up!!!!
	
	//print_mat4(draw_scene->transform);
	//print_mat4(draw_scene->lookat);
	//print_mat4(draw_scene->project);
	shader_program_set_uniform_mat4(draw_scene->shader_program, "transform", draw_scene->transform);	
	shader_program_set_uniform_mat4(draw_scene->shader_program, "view", draw_scene->lookat);	
	shader_program_set_uniform_mat4(draw_scene->shader_program, "project", draw_scene->project);	

	for (int i = 0; i < n_models; i ++) {
		model_draw(draw_scene->models[i], draw_scene->shader_program);
	} 
}

void model_free(model * old_model) {
	free(old_model->indices);
	free(old_model->vertices);
	free(old_model);
}

void scene_free(scene * old_scene) {
	for (int i = 0; i < old_scene->n_models; i ++) {
		model_free(old_scene->models[i]);
	}
	glDeleteProgram(old_scene->shader_program);
	free(old_scene->models);	
	aiReleaseImport(old_scene->model_data);
	free(old_scene);
} 

int main () {
	performance_timer("Startup");
	global_textures = texture_catalouge_create();
	frame_time timer = frame_time_create();
	GLFWwindow * window = window_create();	
	performance_timer("Window created");
	scene * new_scene = scene_create("./backpack.obj");
	performance_timer("Scene created");

	//these two could run in parallel...
	scene_generate_buffers(new_scene);
	performance_timer("Buffers created");
	scene_generate_shaders(new_scene, "./vertex_shader3.glsl", "./fragment_shader3.glsl");
	performance_timer("Shaders created");

	//these transformations are just for fun
	vec4 axis = {0.0, 0.0, 1.0};
	glm_rotate(new_scene->transform, 0, axis);
	glm_translate_z(new_scene->transform, -1.0);
	
	glEnable(GL_DEPTH_TEST);

	while(!glfwWindowShouldClose(window)) {
		frame_time_update(&timer);
		process_input(window, timer);

		glClearColor(0.9, 0.2, 0.2, 0.8);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene_draw_objects(new_scene);
	
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	glfwTerminate();
	global_texture_catalouge_free();
	scene_free(new_scene);
}
