program: main.c
	cc -O2 ./main.c -o test -fopenmp -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -lm -ldl -lGLEW -lassimp

