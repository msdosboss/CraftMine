#include "craftmine.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"


void framebufferSizeCallback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window, float deltaTime){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct Camera *cam = dataWrapper->cam;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    vec3 cameraSpeed = {5.0f * deltaTime, 5.0f * deltaTime, 5.0f * deltaTime};

    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        cameraSpeed[0] *= 5;
        cameraSpeed[1] *= 5;
        cameraSpeed[2] *= 5;
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_vec3_mul(cameraSpeed, cam->front, cameraPosOffset);
        glm_vec3_add(cam->position, cameraPosOffset, cam->position);
        //cam->position[1] = oldCamYposition;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_vec3_mul(cameraSpeed, cam->front, cameraPosOffset);
        glm_vec3_sub(cam->position, cameraPosOffset, cam->position);
        //cam->position[1] = oldCamYposition;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_cross(cam->front, cam->up, cameraPosOffset);
        glm_normalize(cameraPosOffset);
        glm_vec3_mul(cameraPosOffset, cameraSpeed, cameraPosOffset);
        glm_vec3_sub(cam->position, cameraPosOffset, cam->position);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        vec3 cameraPosOffset;
        glm_cross(cam->front, cam->up, cameraPosOffset);
        glm_normalize(cameraPosOffset);
        glm_vec3_mul(cameraPosOffset, cameraSpeed, cameraPosOffset);
        glm_vec3_add(cam->position, cameraPosOffset, cam->position);
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        cam->position[1] += cameraSpeed[1];
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cam->position[1] -= cameraSpeed[1];
    }

    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
        cam->selectBlockId = DIRT;
    }
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
        cam->selectBlockId = STONE;
    }
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
        cam->selectBlockId = WOOD;
    }

    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
        setVisableChunks(window);
    }
}


char *readShaderFile(const char *fileName){
    FILE* file = fopen(fileName, "r");
    if(file == NULL){
        printf("uhhhhhhhhhhhhhhhhhhhhh he's right behind isn't he?\n");
    }
    fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
    char *fileText = malloc(sizeof(char) * (size + 1));

    fread(fileText, 1, size,file);
    fileText[size] = '\0';

    fclose(file);
    return fileText;
}


unsigned int linkShaders(const char *vertexFileName, const char *fragmentFileName){
    const GLchar *vertexFileText = readShaderFile(vertexFileName);
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexFileText, NULL);
    glCompileShader(vertexShader);

    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",infoLog);
    }

    unsigned int fragmentShader;
    const GLchar *fragmentFileText = readShaderFile(fragmentFileName);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentFileText, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",infoLog);
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if(!success){
        char infoLog[512];
        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::LINKING_FAILED\n%s\n",infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


unsigned int genTextures(const char *firstFileName){

    stbi_set_flip_vertically_on_load(true);

    unsigned int texture;

    int width, height, nChannels;
    unsigned char *data = stbi_load(firstFileName, &width, &height, &nChannels, 0);

    if(!data) {
         printf("ORCA %s\n", stbi_failure_reason());
    }

    glGenTextures(1, &texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
}


void mouseCallback(GLFWwindow *window, double xpos, double ypos){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct Camera *cam = dataWrapper->cam;

    if(cam->firstMouse){
        cam->lastX = xpos;
        cam->lastY = ypos;
        cam->firstMouse = 0;
    }

    float xoffset = xpos - cam->lastX;
    float yoffset = cam->lastY - ypos;
    cam->lastX = xpos;
    cam->lastY = ypos;

    xoffset *= cam->sensitivity;
    yoffset *= cam->sensitivity;

    cam->yaw += xoffset;
    cam->pitch += yoffset;

    if(cam->pitch > 89.0f){
        cam->pitch = 89.0f;
    }
    if(cam->pitch < -89.0f){
        cam->pitch = -89.0f;
    }

    vec3 directions;
    directions[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    directions[1] = sin(glm_rad(cam->pitch));
    directions[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));

    glm_normalize_to(directions, cam->front);
}


void scrollCallback(GLFWwindow *window, double xoffset, double yoffset){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct Camera *cam = dataWrapper->cam;

    cam->fov -= (float)yoffset;
    if(cam->fov < 1.0f){
        cam->fov = 1.0f;
    }
    if(cam->fov > 89.0f){
        cam->fov = 89.0f;
    }
}


struct ChunkPos getChunkPosFromWorld(GLFWwindow *window){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct ChunkPos chunkPos = {
        .x = floor(dataWrapper->cam->position[0] / CHUNK_WIDTH),
        .z = floor(dataWrapper->cam->position[2] / CHUNK_WIDTH)
    };

    return chunkPos;
}


int wrapToChunk(int x){
    return ((x % 16) + 16) % 16;
}


void addCube(GLFWwindow *window, struct ChunkMapEntry *chunkEntry){
    struct Chunk *chunk = chunkEntry->chunk;
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct Camera *cam = dataWrapper->cam;

    ivec3 currentPos;
    currentPos[0] = (int)floor(cam->position[0]);
    currentPos[1] = (int)floor(cam->position[1]);
    currentPos[2] = (int)floor(cam->position[2]);

    vec3 step;
    step[0] = (cam->front[0] > 0) ? 1 : -1;
    step[1] = (cam->front[1] > 0) ? 1 : -1;
    step[2] = (cam->front[2] > 0) ? 1 : -1;

    vec3 deltaDist;
    deltaDist[0] = 1 / cam->front[0];
    deltaDist[1] = 1 / cam->front[1];
    deltaDist[2] = 1 / cam->front[2];

    vec3 sideDist;
    if (step[0] > 0){
        sideDist[0] = (floor(cam->position[0]) + 1.0f - cam->position[0]) * deltaDist[0];
    }
    else{
        sideDist[0] = (cam->position[0] - floor(cam->position[0])) * deltaDist[0];
    }
    if (step[1] > 0){
        sideDist[1] = (floor(cam->position[1]) + 1.0f - cam->position[1]) * deltaDist[1];
    }
    else{
        sideDist[1] = (cam->position[1] - floor(cam->position[1])) * deltaDist[1];
    }
    if (step[2] > 0){
        sideDist[2] = (floor(cam->position[2]) + 1.0f - cam->position[2]) * deltaDist[2];
    }
    else{
        sideDist[2] = (cam->position[2] - floor(cam->position[2])) * deltaDist[2];
    }

    ivec3 lastAirBlock;
    glm_ivec3_copy(currentPos, lastAirBlock);

    for(int i = 0; i < 10; i++){
        if(chunk->blocks[wrapToChunk(currentPos[0])][currentPos[1]][wrapToChunk(currentPos[2])].blockId != AIR){
            printf("currentPos: %d, %d, %d lastAirBlock: %d, %d, %d\n", currentPos[0], currentPos[1], currentPos[2], lastAirBlock[0], lastAirBlock[1], lastAirBlock[2]);
            setBlock(chunk, cam->selectBlockId, wrapToChunk(lastAirBlock[0]), lastAirBlock[1], wrapToChunk(lastAirBlock[2]));
            break;
        }
        glm_ivec3_copy(currentPos, lastAirBlock);
        if(fabs(sideDist[0]) < fabs(sideDist[1]) && fabs(sideDist[0]) < fabs(sideDist[2])){
            sideDist[0] += deltaDist[0];
            currentPos[0] += step[0];
        }
        else if(fabs(sideDist[1]) < fabs(sideDist[2])){
            sideDist[1] += deltaDist[1];
            currentPos[1] += step[1];

        }
        else{
            sideDist[2] += deltaDist[2];
            currentPos[2] += step[2];

        }
    }

}


void breakCube(GLFWwindow *window, struct ChunkMapEntry *chunkEntry){
    struct Chunk *chunk = chunkEntry->chunk;
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct Camera *cam = dataWrapper->cam;

    ivec3 currentPos;
    currentPos[0] = (int)floor(cam->position[0]);
    currentPos[1] = (int)floor(cam->position[1]);
    currentPos[2] = (int)floor(cam->position[2]);

    vec3 step;
    step[0] = (cam->front[0] > 0) ? 1 : -1;
    step[1] = (cam->front[1] > 0) ? 1 : -1;
    step[2] = (cam->front[2] > 0) ? 1 : -1;

    vec3 deltaDist;
    deltaDist[0] = 1 / cam->front[0];
    deltaDist[1] = 1 / cam->front[1];
    deltaDist[2] = 1 / cam->front[2];

    vec3 sideDist;
    if (step[0] > 0){
        sideDist[0] = (floor(cam->position[0]) + 1.0f - cam->position[0]) * deltaDist[0];
    }
    else{
        sideDist[0] = (cam->position[0] - floor(cam->position[0])) * deltaDist[0];
    }
    if (step[1] > 0){
        sideDist[1] = (floor(cam->position[1]) + 1.0f - cam->position[1]) * deltaDist[1];
    }
    else{
        sideDist[1] = (cam->position[1] - floor(cam->position[1])) * deltaDist[1];
    }
    if (step[2] > 0){
        sideDist[2] = (floor(cam->position[2]) + 1.0f - cam->position[2]) * deltaDist[2];
    }
    else{
        sideDist[2] = (cam->position[2] - floor(cam->position[2])) * deltaDist[2];
    }

    for(int i = 0; i < 10; i++){
        if(chunk->blocks[wrapToChunk(currentPos[0])][currentPos[1]][wrapToChunk(currentPos[2])].blockId != AIR){
            chunk->blocks[wrapToChunk(currentPos[0])][currentPos[1]][wrapToChunk(currentPos[2])].blockId = AIR;
            break;
        }
        if(fabs(sideDist[0]) < fabs(sideDist[1]) && fabs(sideDist[0]) < fabs(sideDist[2])){
            sideDist[0] += deltaDist[0];
            currentPos[0] += step[0];
        }
        else if(fabs(sideDist[1]) < fabs(sideDist[2])){
            sideDist[1] += deltaDist[1];
            currentPos[1] += step[1];

        }
        else{
            sideDist[2] += deltaDist[2];
            currentPos[2] += step[2];

        }
    }

}


void setBlock(struct Chunk *chunk, int blockId, int x, int y, int z){
    float textureLookupTable[4][4] = {
        {0, 0, 0, 0},  //Air
        {0, 15, 0, 14},  //Dirt
        {1, 15, 1, 15},  //Stone
        {2, 15, 2, 14}   //Wood
    };
    chunk->blocks[x][y][z].blockId = blockId;
    chunk->blocks[x][y][z].texPosition[0] = textureLookupTable[blockId][0];
    chunk->blocks[x][y][z].texPosition[1] = textureLookupTable[blockId][1];
    chunk->blocks[x][y][z].texPositionTop[0] = textureLookupTable[blockId][2];
    chunk->blocks[x][y][z].texPositionTop[1] = textureLookupTable[blockId][3];

}


int getBlock(int x, int y, int z){

    float scale = 0.05f;
    float amp = 20.0f;
    float freq = scale;
    float noise = 0;
    float totalAmp = 0.0f;

    for(int i = 0; i < 4; i++){
        float octive = stb_perlin_noise3(x * freq, z * freq, 1.0f, 0, 0, 0) * amp;
        noise += octive;

        totalAmp += amp;
        amp /= 2.0f;
        freq *= 2.0f;
    }

    noise = noise / totalAmp;
    noise = (noise + 1.0f) * 0.5f;
    noise *= 30;

    int surfaceY = 100 + noise;

    if(y < surfaceY - 5){
        return STONE;
    }
    if(y < surfaceY){
        return DIRT;
    }
    else{
        return AIR;
    }
}


void createChunk(struct Chunk *chunk, int xStart, int zStart){
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 256; y++){
            for(int z = 0; z < 16; z++){

                int blockId = getBlock(x + xStart * 16, y, z + zStart * 16);
                setBlock(chunk, blockId, x, y, z);

                chunk->blocks[x][y][z].blockPosition[0] = x + xStart * 16;
                chunk->blocks[x][y][z].blockPosition[1] = y;
                chunk->blocks[x][y][z].blockPosition[2] = z + zStart * 16;
            }
        }
    }

}



bool isFaceVisible(struct Chunk *chunk, int x, int y, int z, int tileDir){
    if(x <= 0 || x >= 15 || y <= 0 || y >= 256 || z <= 0 || z >= 15){
        return true;
    }
    if(chunk->blocks[x + 1][y][z].blockId == AIR && tileDir == FACE_RIGHT){
        return true;
    }
    if(chunk->blocks[x - 1][y][z].blockId == AIR && tileDir == FACE_LEFT){
        return true;
    }
    if(chunk->blocks[x][y + 1][z].blockId == AIR && tileDir == FACE_TOP){
        return true;
    }
    if(chunk->blocks[x][y - 1][z].blockId == AIR && tileDir == FACE_BOTTOM){
        return true;
    }
    if(chunk->blocks[x][y][z + 1].blockId == AIR && tileDir == FACE_FRONT){
        return true;
    }
    if(chunk->blocks[x][y][z - 1].blockId == AIR && tileDir == FACE_BACK){
        return true;
    }
    return false;
}


void addFaceToBuffer(struct Vertex *mesh, int *meshIndex, int tileDir, int x, int y, int z, vec2 texUV){
    float tileSize = 1.0f / 16.0f;
    float u = texUV[0] * tileSize;
    float v = texUV[1] * tileSize;
    if(tileDir == FACE_TOP){
        float topPos[] = {
            -0.5f,  0.5f, -0.5f, u, v + tileSize,  //Top Left
             0.5f,  0.5f, -0.5f, u + tileSize, v + tileSize, //Top Right
             0.5f,  0.5f,  0.5f, u + tileSize, v, //Bottom Right
             0.5f,  0.5f,  0.5f, u + tileSize, v, //Bottom Right
            -0.5f,  0.5f,  0.5f, u, v, //Bottom Left
            -0.5f,  0.5f, -0.5f, u, v + tileSize //Top Left
        };
        for(int i = 0; i < 6; i++){
            mesh[*meshIndex].pos[0] = topPos[0 + i * 5] + x;
            mesh[*meshIndex].pos[1] = topPos[1 + i * 5] + y;
            mesh[*meshIndex].pos[2] = topPos[2 + i * 5] + z;
            mesh[*meshIndex].uv[0] =  topPos[3 + i * 5];
            mesh[*meshIndex].uv[1] =  topPos[4 + i * 5];
            *meshIndex += 1;
        }

    }
    else if(tileDir == FACE_BOTTOM){
        float bottomPos[] = {
            -0.5f,  -0.5f, -0.5f, u, v,  //Bottom Left
             0.5f,  -0.5f, -0.5f, u + tileSize, v, //Bottom Right
             0.5f,  -0.5f,  0.5f, u + tileSize, v + tileSize, //Top Right
             0.5f,  -0.5f,  0.5f, u + tileSize, v + tileSize, //Top Right
            -0.5f,  -0.5f,  0.5f, u, v + tileSize, //Top Left
            -0.5f,  -0.5f, -0.5f, u, v //Bottom Left
        };
        for(int i = 0; i < 6; i++){
            mesh[*meshIndex].pos[0] = bottomPos[0 + i * 5] + x;
            mesh[*meshIndex].pos[1] = bottomPos[1 + i * 5] + y;
            mesh[*meshIndex].pos[2] = bottomPos[2 + i * 5] + z;
            mesh[*meshIndex].uv[0] =  bottomPos[3 + i * 5];
            mesh[*meshIndex].uv[1] =  bottomPos[4 + i * 5];
            *meshIndex += 1;
        }
    }
    else if(tileDir == FACE_RIGHT){
        float rightPos[] = {
             0.5f,  0.5f,  0.5f, u, v + tileSize,  //Top Left
             0.5f,  0.5f, -0.5f, u + tileSize, v + tileSize, //Top Right
             0.5f, -0.5f, -0.5f, u + tileSize, v,  //Bottom Right
             0.5f, -0.5f, -0.5f, u + tileSize, v, //Bottom Right
             0.5f, -0.5f,  0.5f, u, v, //Bottom Left
             0.5f,  0.5f,  0.5f, u, v + tileSize //Top Left
        };
        for(int i = 0; i < 6; i++){
            mesh[*meshIndex].pos[0] = rightPos[0 + i * 5] + x;
            mesh[*meshIndex].pos[1] = rightPos[1 + i * 5] + y;
            mesh[*meshIndex].pos[2] = rightPos[2 + i * 5] + z;
            mesh[*meshIndex].uv[0] =  rightPos[3 + i * 5];
            mesh[*meshIndex].uv[1] =  rightPos[4 + i * 5];
            *meshIndex += 1;
        }
    }
    else if(tileDir == FACE_LEFT){
        float leftPos[] = {
            -0.5f,  0.5f,  0.5f, u + tileSize, v + tileSize,  //Top Right
            -0.5f,  0.5f, -0.5f, u, v + tileSize, //Top Left
            -0.5f, -0.5f, -0.5f, u, v,  //Bottom Left
            -0.5f, -0.5f, -0.5f, u, v,  //Bottom Left
            -0.5f, -0.5f,  0.5f, u + tileSize, v,  //Bottom Right
            -0.5f,  0.5f,  0.5f, u + tileSize, v + tileSize  //Top Right
        };
        for(int i = 0; i < 6; i++){
            mesh[*meshIndex].pos[0] = leftPos[0 + i * 5] + x;
            mesh[*meshIndex].pos[1] = leftPos[1 + i * 5] + y;
            mesh[*meshIndex].pos[2] = leftPos[2 + i * 5] + z;
            mesh[*meshIndex].uv[0] =  leftPos[3 + i * 5];
            mesh[*meshIndex].uv[1] =  leftPos[4 + i * 5];
            *meshIndex += 1;
        }
    }
    else if(tileDir == FACE_FRONT){
        float frontPos[] = {
            -0.5f, -0.5f,  0.5f, u, v,  //Bottom Left
             0.5f, -0.5f,  0.5f, u + tileSize, v,  //Bottom Right
             0.5f,  0.5f,  0.5f, u + tileSize, v + tileSize,  //Top Right
             0.5f,  0.5f,  0.5f, u + tileSize, v + tileSize, //Top Right
            -0.5f,  0.5f,  0.5f, u, v + tileSize, //Top Left
            -0.5f, -0.5f,  0.5f, u, v //Bottom Left
        };
        for(int i = 0; i < 6; i++){
            mesh[*meshIndex].pos[0] = frontPos[0 + i * 5] + x;
            mesh[*meshIndex].pos[1] = frontPos[1 + i * 5] + y;
            mesh[*meshIndex].pos[2] = frontPos[2 + i * 5] + z;
            mesh[*meshIndex].uv[0] =  frontPos[3 + i * 5];
            mesh[*meshIndex].uv[1] =  frontPos[4 + i * 5];
            *meshIndex += 1;
        }
    }
    else if(tileDir == FACE_BACK){
        float backPos[] = {
            -0.5f, -0.5f, -0.5f, u + tileSize, v, //Bottom Right
             0.5f, -0.5f, -0.5f, u, v, //Bottom Left
             0.5f,  0.5f, -0.5f, u, v + tileSize,  //Top Left
             0.5f,  0.5f, -0.5f, u, v + tileSize, //Top Left
            -0.5f,  0.5f, -0.5f, u + tileSize, v + tileSize, //Top Right
            -0.5f, -0.5f, -0.5f, u + tileSize, v  //Bottom Right
        };
        for(int i = 0; i < 6; i++){
            mesh[*meshIndex].pos[0] = backPos[0 + i * 5] + x;
            mesh[*meshIndex].pos[1] = backPos[1 + i * 5] + y;
            mesh[*meshIndex].pos[2] = backPos[2 + i * 5] + z;
            mesh[*meshIndex].uv[0] =  backPos[3 + i * 5];
            mesh[*meshIndex].uv[1] =  backPos[4 + i * 5];
            *meshIndex += 1;
        }
    }
}


struct Mesh createChunkMesh(GLFWwindow *window, struct Chunk *chunk){
    struct Mesh mesh;
    mesh.vertices = malloc(sizeof(struct Vertex) * 16 * 256 * 16 * 6);    //this would be to small if I added every face in a chunk to it but at most only like 10% of the faces should be visable at a time
    mesh.meshIndex = 0;
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 256; y++){
            for(int z = 0; z < 16; z++){
                if(chunk->blocks[x][y][z].blockId == AIR){
                    continue;
                }
                int blockXPos = chunk->blocks[x][y][z].blockPosition[0];
                int blockYPos = chunk->blocks[x][y][z].blockPosition[1];
                int blockZPos = chunk->blocks[x][y][z].blockPosition[2];

                if(y == 255 || isFaceVisible(chunk, x, y, z, FACE_TOP)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_TOP, blockXPos, blockYPos, blockZPos, chunk->blocks[x][y][z].texPositionTop);
                }
                if(y == 0 || isFaceVisible(chunk, x, y, z, FACE_BOTTOM)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_BOTTOM, blockXPos, blockYPos, blockZPos, chunk->blocks[x][y][z].texPosition);
                }
                if(x == 15 || isFaceVisible(chunk, x, y, z, FACE_RIGHT)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_RIGHT, blockXPos, blockYPos, blockZPos, chunk->blocks[x][y][z].texPosition);
                }
                if(x == 0 || isFaceVisible(chunk, x, y, z, FACE_LEFT)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_LEFT, blockXPos, blockYPos, blockZPos, chunk->blocks[x][y][z].texPosition);
                }
                if(z == 15 || isFaceVisible(chunk, x, y, z, FACE_FRONT)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_FRONT, blockXPos, blockYPos, blockZPos, chunk->blocks[x][y][z].texPosition);
                }
                if(z == 0 || isFaceVisible(chunk, x, y, z, FACE_BACK)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_BACK, blockXPos, blockYPos, blockZPos, chunk->blocks[x][y][z].texPosition);
                }
            }
        }
    }

    return mesh;

}


void putDataOntoGPU(GLFWwindow *window, struct Mesh mesh, unsigned int VAO, unsigned VBO){
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, MAX_SIZE * sizeof(struct Vertex), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(ptr, mesh.vertices, mesh.meshIndex * sizeof(struct Vertex));
    glUnmapBuffer(GL_ARRAY_BUFFER);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindVertexArray(0);

}


void updateMesh(GLFWwindow *window, struct ChunkMapEntry *chunkMapEntry){
        printf("running putDataOntoGPU\n");
        struct Mesh *mesh = malloc(sizeof(struct Mesh));
        *mesh = createChunkMesh(window, chunkMapEntry->chunk);
        putDataOntoGPU(window, *mesh, chunkMapEntry->VAO, chunkMapEntry->VBO);
        free(chunkMapEntry->mesh->vertices);
        free(chunkMapEntry->mesh);
        chunkMapEntry->mesh = mesh;

}


void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods){
    struct ChunkPos chunkPos = getChunkPosFromWorld(window);
    printf("chunkPos.x = %d chunkPos.z = %d\n", chunkPos.x, chunkPos.z);
    struct ChunkMapEntry *chunkMapEntry = getChunk(window, chunkPos.x, chunkPos.z);
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        addCube(window, chunkMapEntry);
        updateMesh(window, chunkMapEntry);
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        breakCube(window, chunkMapEntry);
        updateMesh(window, chunkMapEntry);
    }
}


void setChunk(GLFWwindow *window, struct ChunkMapEntry chunkEntry){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    if(dataWrapper->world->count >= dataWrapper->world->max){
        return;
    }

    struct ChunkMapEntry *entryCopy = malloc(sizeof(struct ChunkMapEntry));
    *entryCopy = chunkEntry;

    hmput(dataWrapper->world->chunkMap, entryCopy->key, entryCopy);
    dataWrapper->world->count += 1;
}


struct ChunkMapEntry *getChunk(GLFWwindow *window, int x, int z){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    return hmget(dataWrapper->world->chunkMap, ((struct ChunkPos){x, z}));
}


struct ChunkMapEntry createChunkEntry(GLFWwindow *window, int x, int z){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);  //Creates buffer for array data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //binds buffer to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, MAX_SIZE * sizeof(struct Vertex), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, pos));  //telling the openGL state box how to read the inputVector
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, uv));
    glEnableVertexAttribArray(1);

    //struct Chunk chunk;
    struct Chunk *chunk = malloc(sizeof(struct Chunk));  //A chunk is to big to fit on the stack
    if(!chunk){
        fprintf(stderr, "Failed to malloc chunk\n");
        exit(1);
    }
    memset(chunk->blocks, 0, sizeof(chunk->blocks));
    createChunk(chunk, x, z);

    struct Mesh *mesh = malloc(sizeof(struct Mesh));
    *mesh = createChunkMesh(window, chunk);

    struct ChunkMapEntry entry = {
        .key = (struct ChunkPos){x, z},
        .mesh = mesh,
        .chunk = chunk,
        .VAO = VAO,
        .VBO = VBO
    };

    return entry;
}


void removeChunkEntry(struct ChunkMapEntry *chunkEntry){
    glDeleteBuffers(1, &(chunkEntry->VBO));
    glDeleteVertexArrays(1, &(chunkEntry->VAO));

    writeChunk(chunkEntry->chunk, chunkEntry->key.x, chunkEntry->key.z);

    free(chunkEntry->chunk);
    free(chunkEntry->mesh);
    free(chunkEntry);
}


void removeNoneVisableChunks(GLFWwindow *window){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct ChunkMapEntryPtrPair *chunkMap = dataWrapper->world->chunkMap;

    struct ChunkPos playerPos = getChunkPosFromWorld(window);
    int xLowLim = playerPos.x - RENDER_DISTANCE / 2;
    int xUpLim = playerPos.x + RENDER_DISTANCE / 2;
    int zLowLim = playerPos.z - RENDER_DISTANCE / 2;
    int zUpLim = playerPos.z + RENDER_DISTANCE / 2;

    int *toRemove = NULL;

    for(int i = 0; i < hmlen(chunkMap); i++){
        if(chunkMap[i].value->key.x > xUpLim || chunkMap[i].value->key.x < xLowLim || chunkMap[i].value->key.z > zUpLim || chunkMap[i].value->key.z < zLowLim){
            arrput(toRemove, i);
        }
    }
    for(int i = 0; i < arrlen(toRemove); i++){
            removeChunkEntry(chunkMap[toRemove[i]].value);
    }

}


void setVisableChunks(GLFWwindow *window){
    struct DataWrapper *dataWrapper = glfwGetWindowUserPointer(window);
    struct ChunkMapEntry **visableChunks = dataWrapper->visableChunks;
    struct World *world = dataWrapper->world;

    removeNoneVisableChunks(window);

    struct ChunkPos pos = getChunkPosFromWorld(window);

    printf("hello world\n");
    fflush(stdout);

    int i = 0;
    for(int x = pos.x - RENDER_DISTANCE / 2; x < pos.x + RENDER_DISTANCE / 2; x++){
        for(int z = pos.z - RENDER_DISTANCE / 2; z < pos.z + RENDER_DISTANCE / 2; z++){
            struct Chunk *chunkFromDrive = readChunk(x, z);
            struct ChunkMapEntry *entry = getChunk(window, x, z);
            if(entry != NULL){
                visableChunks[i++] = entry;
            }
            else if(chunkFromDrive != NULL){
                visableChunks[i] = malloc(sizeof(struct ChunkMapEntry));
                visableChunks[i]->chunk = chunkFromDrive;
                visableChunks[i]->mesh = malloc(sizeof(struct Mesh));
                *visableChunks[i]->mesh = createChunkMesh(window, visableChunks[i]->chunk);
                visableChunks[i]->key.x = x;
                visableChunks[i]->key.z = z;
                setChunk(window, *(visableChunks[i]));
                putDataOntoGPU(window, *visableChunks[i]->mesh, visableChunks[i]->VAO, visableChunks[i]->VBO);
                i++;

            }
            else{
                visableChunks[i] = malloc(sizeof(struct ChunkMapEntry));
                *visableChunks[i] = createChunkEntry(window, x, z);
                setChunk(window, *(visableChunks[i]));
                putDataOntoGPU(window, *visableChunks[i]->mesh, visableChunks[i]->VAO, visableChunks[i]->VBO);
                i++;
            }
        }
    }

    printf("goodbye world\n");
    fflush(stdout);
}


int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "CraftMine", NULL, NULL);
    if(window == NULL){
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("GLAD failed\n");
        return -1;
    }
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    //glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    struct Camera camera ={
        .position = {0.0f, 120.0f, 3.0f},
        .front = {0.0f, 0.0f, -1.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .yaw = -90.0f,
        .pitch = 0.0f,
        .sensitivity = 0.1f,
        .lastX = 400.0f,
        .lastY = 300.0f,
        .firstMouse = 1,
        .fov = 45.0f,
        .selectBlockId = DIRT
    };
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    struct World world;
    world.count = 0;
    world.max = 1000;
    world.chunkMap = NULL;

    struct DataWrapper dataWrapper = {
        .cam = &camera,
        .world = &world,
        .visableChunks = malloc(sizeof(struct ChunkMapEntry) * RENDER_DISTANCE * RENDER_DISTANCE),
    };

    glfwSetWindowUserPointer(window, &dataWrapper);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    setVisableChunks(window);

    unsigned int shaderProgram = linkShaders("shaders/3dVertex.glsl", "shaders/3dFragments.glsl");
    glUseProgram(shaderProgram);

    float mixValue = 0.2;

    glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

    unsigned int texture = genTextures("textures/craftmineTextures.png");

    int textureUniformLocation = glGetUniformLocation(shaderProgram, "ourTexture");
    glUniform1i(textureUniformLocation, 0);

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, deltaTime);

        glUseProgram(shaderProgram);

        mat4 view;

        vec3 cameraCenter;
        glm_vec3_add(camera.position, camera.front, cameraCenter);
        glm_lookat(camera.position, cameraCenter, camera.up, view);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, (const float *)view);

        mat4 projection;
        glm_perspective(glm_rad(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, (const float *)projection);

        for(int i = 0; i < RENDER_DISTANCE * RENDER_DISTANCE; i++){
            unsigned int VAO = dataWrapper.visableChunks[i]->VAO;
            unsigned int VBO = dataWrapper.visableChunks[i]->VBO;
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);  //binds buffer to the GL_ARRAY_BUFFER
            glDrawArrays(GL_TRIANGLES, 0, dataWrapper.visableChunks[i]->mesh->meshIndex);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for(int i = 0; i < world.count; i++){
        free(world.chunkMap[i].value->mesh->vertices);
        free(world.chunkMap[i].value->mesh);
        free(world.chunkMap[i].value->chunk);
        free(world.chunkMap[i].value);
        //free(dataWrapper.visableChunks);
    }
    glfwTerminate();
    return 0;
}
