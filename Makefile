program: main.c
	cc ./main.c -o test -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -lGLEW 

