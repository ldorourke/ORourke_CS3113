
/*
 Lucas O'Rourke
 lor215
 3 textures added
 */

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif


SDL_Window* displayWindow;


GLuint LoadTexture(const char *img_path){
    int width, height, item;
    unsigned char* img = stbi_load(img_path, &width, &height, &item, STBI_rgb_alpha);
    
    if (img == NULL){
        std::cout << "Cannot show image. Please make sure the directory is the correct one!\n";
    }
    
    GLuint imgTexture;
    glGenTextures(1, &imgTexture);
    glBindTexture(GL_TEXTURE_2D, imgTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(img);
    return imgTexture;
}



int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("O'Rourke_HW1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    
    //Preloop Setup
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    

    GLuint middleTexture = LoadTexture(RESOURCE_FOLDER"middle.png");
    GLuint topBotTexture = LoadTexture(RESOURCE_FOLDER"tb.png");
    GLuint paddleTexture = LoadTexture(RESOURCE_FOLDER"paddle.png");
    GLuint ballTexture = LoadTexture(RESOURCE_FOLDER"ball.png");
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix modelViewMatrix;
    Matrix lmodelViewMatrix;
    Matrix rmodelViewMatrix;
    Matrix bmodelViewMatrix;

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    //Position Variables
    float lastFrameTicks = 0;
    float ylPos = 0;
    float yrPos = 0;
    float ballx = 0;
    float bally= 0;
    float distToMove = 0.1;
    float elapsed = 0;
    float ballDistY = 0;
    float ballDistX = 0;
    bool up = true;
    bool right = true;
    
    SDL_Event event;
    bool done = false;
    
    bmodelViewMatrix.Translate(ballDistX, ballDistY, 0);
    
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            const Uint8 *keys = SDL_GetKeyboardState(NULL);

                //Right Paddle
            
            
                if (keys[SDL_SCANCODE_DOWN]) {
                    if (yrPos >= -1.4) {
                        rmodelViewMatrix.Translate(0.0, -distToMove, 0.0);
                        yrPos -= distToMove;
                    }
                }
                if (keys[SDL_SCANCODE_UP]) {
                    if (yrPos <= 1.4) {
                        rmodelViewMatrix.Translate(0.0, distToMove, 0.0);
                        yrPos += distToMove;
                    }
                }
                
                //Left Paddle
                if (keys[SDL_SCANCODE_S]) {
                    if (ylPos >= -1.4) {
                        lmodelViewMatrix.Translate(0.0, -distToMove, 0.0);
                        ylPos -= distToMove;
                    }
                }
                if (keys[SDL_SCANCODE_W]) {
                    if (ylPos <= 1.4) {
                        lmodelViewMatrix.Translate(0.0, distToMove, 0.0);
                        ylPos += distToMove;
                    }
                }
            
        }
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
       
        glUseProgram(program.programID);
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(modelViewMatrix);

        
        glClear(GL_COLOR_BUFFER_BIT);

        
        //Middle Texture
        glBindTexture(GL_TEXTURE_2D, middleTexture);
        float midVertices[] = {-0.15f, -1.85f, 0.15f, -1.85f, 0.15f, 1.85f, -0.15f, -1.85f, 0.15f, 1.85f, -0.15f, 1.85f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, midVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float midTexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, midTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Top Texture
        glBindTexture(GL_TEXTURE_2D, topBotTexture);
        float topVertices[] = {-3.0f, 1.85f, 3.0f, 1.85f, 3.0f, 2.0f, -3.0f, 1.85f, 3.0f, 2.0f, -3.0f, 2.0f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, topVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float topTextCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, topTextCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Bottom Texture
        glBindTexture(GL_TEXTURE_2D, topBotTexture);
        float botVertices[] = {-3.0f, -1.85f, 3.0f, -1.85f, 3.0f, -2.0f, -3.0f, -1.85f, 3.0f, -2.0f, -3.0f, -2.0f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, botVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float botTextCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, botTextCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        
        //Paddle Left Texture
        program.SetModelviewMatrix(lmodelViewMatrix);
        glBindTexture(GL_TEXTURE_2D, paddleTexture);
        float plVertices[] = {-3.0f, -0.4f, -2.65f, -0.4f, -2.65f, 0.4f, -3.0f, -0.4f, -2.65f, 0.4f, -3.0f, 0.4f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, plVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float plTextCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, plTextCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Paddle Right Texture
        program.SetModelviewMatrix(rmodelViewMatrix);
        glBindTexture(GL_TEXTURE_2D, paddleTexture);
        float prVertices[] = {3.0f, -0.4f, 2.65f, -0.4f, 2.65f, 0.4f, 3.0f, -0.4f, 2.65f, 0.4f, 3.0f, 0.4f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, prVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float prTextCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, prTextCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        
        //Ball Texture
        glBindTexture(GL_TEXTURE_2D, ballTexture);
        program.SetModelviewMatrix(bmodelViewMatrix);
        float ballVertices[] = {-0.15f, -0.15f, 0.15f, -0.15f, 0.15f, 0.15f, -0.15f, -0.15f, 0.15f, 0.15f, -0.15f, 0.15f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float ballTextCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, ballTextCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        ballDistY = elapsed*2.0;
        ballDistX = elapsed*2.0;
        
        
        //Y Checks
        if (up == false) {
            ballDistY *= -1;
        }
    
        if (right == false) {
            ballDistX *= -1;
        }
        
        if (bally >= 1.7) {
            ballDistY *= -1;
            up = false;
        }
        
        else if (bally <= -1.7) {
            ballDistY *= -1;
            up = true;
        }
        
        
        //X Checks
        float topR = yrPos + 0.45;
        float botR = yrPos - 0.45;
        float topL = ylPos + 0.45;
        float botL = ylPos - 0.45;
        
        if (ballx >= 2.6) {
            if (bally <= topR && bally >= botR) {
                ballDistX *= -1;
                right = false;
            }
        }
        
        else if (ballx <= -2.6) {
            if (bally <= topL && bally >= botL) {
                ballDistX *= -1;
                right = true;
            }
        }
        
        if (ballx >= 3.5) {
            std::cout << "Left Player wins!\n";
            done = true;
        }
        if (ballx <= -3.5) {
            std::cout << "Right Player wins!\n";
            done = true;
        }
        
        bmodelViewMatrix.Translate(ballDistX, ballDistY, 0.0);
        ballx += ballDistX;
        bally += ballDistY;
        

        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}