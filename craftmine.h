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
