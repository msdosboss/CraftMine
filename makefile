all: craftmine.c glad.c
	gcc -g -Wall -o main craftmine.c glad.c -lglfw -lm
