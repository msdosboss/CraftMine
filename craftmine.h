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

#define CHUNK_WIDTH 16

#define MAX_SIZE 256 * 16 * 16 * 6 * 5

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
  int selectBlockId;
};

struct Vertex {
  vec3 pos;
  vec2 uv;
};

struct Mesh {
  struct Vertex *vertices;
  int meshIndex;
};

struct DataWrapper {
  struct Camera *cam;
  struct World *world;
};

struct Block {
  int blockId;
  vec3 blockPosition;
  vec2 texPosition;
  vec2 texPositionTop;
};

struct Chunk {
  struct Block blocks[16][256][16];
};

struct ChunkPos {
  int x, z;
};

struct ChunkMapEntry {
  struct ChunkPos key;
  struct Mesh *mesh;
  struct Chunk *chunk;
  unsigned int VBO;
  unsigned int VAO;
};

struct ChunkMapEntryPtrPair {
  struct ChunkPos key;
  struct ChunkMapEntry *value;
};

struct World {
  struct ChunkMapEntryPtrPair *chunkMap;
  struct ChunkMapEntry **entries;
  int count;
  int max;
};

void setChunk(GLFWwindow *window, struct ChunkMapEntry chunkEntry);
struct ChunkMapEntry *getChunk(GLFWwindow *window, int x, int z);
#endif
