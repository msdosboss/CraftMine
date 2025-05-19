#ifndef FILEIO_H
#define FILEIO_H

#include "cglm/cglm.h"
#include <stdio.h>

struct Block {
  int blockId;
  vec3 blockPosition;
  vec2 texPosition;
  vec2 texPositionTop;
};

struct Chunk {
  struct Block blocks[16][256][16];
};

void writeChunk(struct Chunk *chunk, int x, int z);
struct Chunk *readChunk(int x, int z);

#endif
