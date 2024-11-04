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
