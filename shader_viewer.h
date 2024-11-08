#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "./cglm/cam.h"
#include "./cglm/mat4.h"
#include "./cglm/vec3.h"
#include "./cglm/cglm.h"
#include "./cglm/affine-pre.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

typedef struct frame_time {
	float delta_time; 
	float last_frame;
} frame_time;

typedef struct camera {
	vec3 position;
	vec3 front;
	vec3 up;
	float aspect_ratio;
} camera;

typedef	struct texture_catalouge {
	unsigned int * gl_texture_ids; 
	unsigned int * texture_indices; 
	int n_items;
	int max_items;
} texture_catalouge;  

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
	vec3 light_position;
	float highlight;	
} scene;
