all: craftmine.c glad.c
	gcc -o main craftmine.c glad.c -lglfw -lm
