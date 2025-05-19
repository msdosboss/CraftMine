all: craftmine.c glad.c
	gcc -o main craftmine.c glad.c fileio.c -lglfw -lm
