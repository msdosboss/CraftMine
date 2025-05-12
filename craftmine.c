#include "craftmine.h"
#include "cglm/cglm.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window, float deltaTime){
    struct CamAndChunk *camAndChunk = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndChunk->cam;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    vec3 cameraSpeed = {2.5f * deltaTime, 2.5f * deltaTime, 2.5f * deltaTime};

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
    struct CamAndChunk *camAndChunk = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndChunk->cam;

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
    struct CamAndChunk *camAndChunk = glfwGetWindowUserPointer(window);
    struct Camera *cam = camAndChunk->cam;

    cam->fov -= (float)yoffset;
    if(cam->fov < 1.0f){
        cam->fov = 1.0f;
    }
    if(cam->fov > 89.0f){
        cam->fov = 89.0f;
    }
}


void addCube(GLFWwindow *window){
    struct CamAndChunk *camAndChunk = glfwGetWindowUserPointer(window);
    struct Chunk *chunk = camAndChunk->chunk;
    struct Camera *cam = camAndChunk->cam;

    ivec3 currentPos;
    currentPos[0] = (int)floor(cam->position[0]);
    currentPos[1] = (int)floor(cam->position[1]) + 1;
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
        if(chunk->blocks[currentPos[0]][currentPos[1]][currentPos[2]].blockId != AIR){
            printf("currentPos: %d, %d, %d lastAirBlock: %d, %d, %d\n", currentPos[0], currentPos[1], currentPos[2], lastAirBlock[0], lastAirBlock[1], lastAirBlock[2]);
            chunk->blocks[lastAirBlock[0]][lastAirBlock[1]][lastAirBlock[2]].blockId = DIRT;
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


void breakCube(GLFWwindow *window){
    struct CamAndChunk *camAndChunk = glfwGetWindowUserPointer(window);
    struct Chunk *chunk = camAndChunk->chunk;
    struct Camera *cam = camAndChunk->cam;

    ivec3 currentPos;
    currentPos[0] = (int)floor(cam->position[0]);
    currentPos[1] = (int)floor(cam->position[1]) + 1;
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
        if(chunk->blocks[currentPos[0]][currentPos[1]][currentPos[2]].blockId != AIR){
            chunk->blocks[currentPos[0]][currentPos[1]][currentPos[2]].blockId = AIR;
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


void createChunk(struct Chunk *chunk){
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 256; y++){
            for(int z = 0; z < 16; z++){
                if(y <= 20){
                    chunk->blocks[x][y][z].blockId = DIRT;
                    chunk->blocks[x][y][z].texPosition[0] = 0;
                    chunk->blocks[x][y][z].texPosition[1] = 15;
                }
                else{
                    chunk->blocks[x][y][z].blockId = AIR;
                }
                chunk->blocks[x][y][z].blockPosition[0] = x;
                chunk->blocks[x][y][z].blockPosition[1] = y;
                chunk->blocks[x][y][z].blockPosition[2] = z;
            }
        }
    }

}



bool isFaceVisible(struct Chunk *chunk, int x, int y, int z, int tileDir){
    if(x == 15 || x == 0 || y == 0 || y == 255 || z == 0 || z == 15){
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
            -0.5f,  0.5f, -0.5f, u, v //Bottom Left
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
                if(y == 255 || isFaceVisible(chunk, x, y, z, FACE_TOP)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_TOP, x, y, z, chunk->blocks[x][y][z].texPosition);
                }
                if(y == 0 || isFaceVisible(chunk, x, y, z, FACE_BOTTOM)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_BOTTOM, x, y, z, chunk->blocks[x][y][z].texPosition);
                }
                if(x == 15 || isFaceVisible(chunk, x, y, z, FACE_RIGHT)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_RIGHT, x, y, z, chunk->blocks[x][y][z].texPosition);
                }
                if(x == 0 || isFaceVisible(chunk, x, y, z, FACE_LEFT)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_LEFT, x, y, z, chunk->blocks[x][y][z].texPosition);
                }
                if(z == 15 || isFaceVisible(chunk, x, y, z, FACE_FRONT)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_FRONT, x, y, z, chunk->blocks[x][y][z].texPosition);
                }
                if(z == 0 || isFaceVisible(chunk, x, y, z, FACE_BACK)){
                    addFaceToBuffer(mesh.vertices, &mesh.meshIndex, FACE_BACK, x, y, z, chunk->blocks[x][y][z].texPosition);
                }
            }
        }
    }

    return mesh;

}


void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        addCube(window);
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        breakCube(window);
    }
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

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    struct Camera camera ={
        .position = {0.0f, 21.0f, 3.0f},
        .front = {0.0f, 0.0f, -1.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .yaw = -90.0f,
        .pitch = 0.0f,
        .sensitivity = 0.1f,
        .lastX = 400.0f,
        .lastY = 300.0f,
        .firstMouse = 1,
        .fov = 45.0f
    };
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    //struct Chunk chunk;
    struct Chunk *chunk = malloc(sizeof(struct Chunk));  //A chunk is to big to fit on the stack
    createChunk(chunk);

    struct CamAndChunk camAndChunk = {
        .cam = &camera,
        .chunk = chunk
    };

    struct Mesh mesh = createChunkMesh(window, chunk);

    printf("meshIndex = %d\n", mesh.meshIndex);

    for(int i = 0; i < 100; i++){
        printf("%f, %f, %f, %f, %f\n", mesh.vertices[i].pos[0], mesh.vertices[i].pos[1], mesh.vertices[i].pos[2],mesh.vertices[i].uv[0], mesh.vertices[i].uv[1]);
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int VBO;
    glGenBuffers(1, &VBO);  //Creates buffer for array data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //binds buffer to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * mesh.meshIndex, mesh.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, pos));  //telling the openGL state box how to read the inputVector
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, uv));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int shaderProgram = linkShaders("shaders/3dVertex.glsl", "shaders/3dFragments.glsl");
    glUseProgram(shaderProgram);

    float mixValue = 0.2;

    glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

    unsigned int texture = genTextures("textures/craftmineTextures.png");
    glBindVertexArray(VAO);

    int textureUniformLocation = glGetUniformLocation(shaderProgram, "ourTexture");
    glUniform1i(textureUniformLocation, 0);

    glEnable(GL_DEPTH_TEST);
    glfwSetWindowUserPointer(window, &camAndChunk);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

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

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, mesh.meshIndex);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    free(mesh.vertices);
    free(chunk);
    glfwTerminate();
    return 0;
}
