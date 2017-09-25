
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
    
    
    GLuint starTexture = LoadTexture(RESOURCE_FOLDER"goldStar.png");
    GLuint moonTexture = LoadTexture(RESOURCE_FOLDER"moon.png");
    GLuint carTexture = LoadTexture(RESOURCE_FOLDER"car.png");
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix modelViewMatrix;
    Matrix modelViewMatrix2;
    Matrix modelViewMatrix3;

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float lastFrameTicks = 0;
    float xpos = 0;
    
    SDL_Event event;
    bool done = false;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        
        
        glUseProgram(program.programID);
        
        program.SetModelviewMatrix(modelViewMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        
        glClear(GL_COLOR_BUFFER_BIT);

        
        
        //Gold star coordinates
        
        glBindTexture(GL_TEXTURE_2D, starTexture);
        
        
        
        float starVertices[] = {-2.5f, 0.5f, -1.5f, 0.5f, -1.5f, 1.5f, -2.5f, 0.5f, -1.5f, 1.5f, -2.5f, 1.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, starVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float starTexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, starTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Moon coordinates
        
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        program.SetModelviewMatrix(modelViewMatrix2);
        
        float moonVertices[] = {2.5f, 0.5f, 1.5f, 0.5f, 1.5f, 1.5f, 2.5f, 0.5f, 1.5f, 1.5f, 2.5f, 1.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, moonVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float moonTexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, moonTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Car coordinates
        
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        float distance = elapsed * 1.0f;
        xpos += distance;
        
        if (xpos >= 7.1) {
            
            modelViewMatrix3.Translate(-xpos, 0.0, 0.0);
            xpos = 0;
        }

        modelViewMatrix3.Translate(distance, 0.0, 0.0);


        glBindTexture(GL_TEXTURE_2D, carTexture);
        
        
        program.SetModelviewMatrix(modelViewMatrix3);
        
        
        float carVertices[] = {-3.5, -1.5, -2.5, -1.5, -2.5, -0.5, -3.5, -1.5, -2.5, -0.5, -3.5, -0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, carVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float carTexCoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, carTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}