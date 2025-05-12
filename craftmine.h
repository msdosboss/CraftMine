#ifndef CRAFTMINE_H
#define CRAFTMINE_H

#include "cglm/cglm.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define AIR 0
#define DIRT 1
#define STONE 2

#define FACE_TOP 0
#define FACE_BOTTOM 1
#define FACE_LEFT 2
#define FACE_RIGHT 3
#define FACE_FRONT 4
#define FACE_BACK 5

struct Camera {
  vec3 position;
  vec3 front;
  vec3 up;
  float yaw;
  float pitch;
  float sensitivity;
  float lastX, lastY;
  int firstMouse;
  float fov;
};

struct Vertex {
  vec3 pos;
  vec2 uv;
};

struct Mesh {
  struct Vertex *vertices;
  int meshIndex;
};

struct CamAndChunk {
  struct Camera *cam;
  struct Chunk *chunk;
};

struct Block {
  int blockId;
  vec3 blockPosition;
  vec2 texPosition;
};

struct Chunk {
  struct Block blocks[16][256][16];
};

#endif
