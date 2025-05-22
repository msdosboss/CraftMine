#include "fileio.h"

void writeChunk(struct Chunk *chunk, int x, int z) {
    char chunkName[50];
    sprintf(chunkName, "chunkBinData/chunkBinx%dz%d.bin", x, z);

    FILE *fp = fopen(chunkName, "wb");
    if (!fp) {
        perror("Failed to open chunk file for writing");
        return;
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 16; k++) {
                struct Block *b = &chunk->blocks[i][j][k];

                fwrite(&b->blockId, sizeof(int), 1, fp);
                fwrite(b->blockPosition, sizeof(float), 3, fp);
                fwrite(b->texPosition, sizeof(float), 2, fp);
                fwrite(b->texPositionTop, sizeof(float), 2, fp);
            }
        }
    }

    fclose(fp);
}

struct Chunk *readChunk(int x, int z) {
    char chunkName[50];
    sprintf(chunkName, "chunkBinData/chunkBinx%dz%d.bin", x, z);

    FILE *fp = fopen(chunkName, "rb");
    if (!fp) {
        return NULL;
    }

    struct Chunk *chunk = malloc(sizeof(struct Chunk));
    if (!chunk) {
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 16; k++) {
                struct Block *b = &chunk->blocks[i][j][k];

                fread(&b->blockId, sizeof(int), 1, fp);
                fread(b->blockPosition, sizeof(float), 3, fp);
                fread(b->texPosition, sizeof(float), 2, fp);
                fread(b->texPositionTop, sizeof(float), 2, fp);
            }
        }
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
