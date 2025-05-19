#include "fileio.h"

void writeChunk(struct Chunk *chunk, int x, int z){
    char chunkName[50];
    sprintf(chunkName, "chunkBinData/chunkBinx%dz%d.bin", x, z);

    FILE *fp = fopen(chunkName, "wb");

    fwrite(chunk, sizeof(struct Chunk), 1, fp);

    fclose(fp);
}

struct Chunk *readChunk(int x, int z){
    char chunkName[50];
    sprintf(chunkName, "chunkBinData/chunkBinx%dz%d.bin", x, z);

    FILE *fp = fopen(chunkName, "rb");

    if(fp == NULL){ //This will happen a lot because I will have to check if a chunk exists in files before trying to make a new one
        return NULL;
    }

    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    size_t success = fread(chunk, sizeof(struct Chunk), 1, fp);

    if(!success){
        fprintf(stderr, "failed to read %s", chunkName);
    }

    fclose(fp);

    return chunk;
}

/*int main(){
>>>>>>> c0ac3a3 (Had problems removing large files, almost lost all my work luckily my vim .swp file saved me)
    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    chunk->blocks[0][0][0].blockId = 3;
    writeChunk(chunk, 0, 0);
    free(chunk);
    struct Chunk *chunkRead = readChunk(0,0);
    printf("%d\n",chunkRead->blocks[0][0][0].blockId);

    return 0;
}*/
