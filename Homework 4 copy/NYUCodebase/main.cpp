/*
 Lucas O'Rourke
 lor215
 Hw4
 
 */


#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <vector>

#include "ShaderProgram.h"
#include "Matrix.h"
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;
SDL_Window* displayWindow;
#define FIXED_TIMESTEP 0.0166666f



class SheetSprite {
public:
    SheetSprite() {}
    SheetSprite(unsigned int texID, float u1, float v1, float width1, float height1, float size1) : textureID(texID), u(u1), v(v1), width(width1), height(height1), size(size1) {}
    
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    
    
    void Draw(ShaderProgram* program) {
        
        Matrix modelviewMatrix;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLfloat texCoords[] = {
            u, v + height,
            u + width, v,
            u, v,
            u + width, v,
            u, v + height,
            u + width, v + height
        };
        
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size
        };
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    }
};

class Vector3 {
public:
    Vector3() {}
    Vector3(float x, float y, float z) : xVal(x), yVal(y), zVal(z) {}
    float xVal;
    float yVal;
    float zVal;
};

//Entity Class
enum EntityType {ENTITY_PLAYER, ENTITY_PLAT, ENTITY_CHERRY};
class Entity {
public:
    Entity() {}
    
    Entity(float x, float y, float z, float xValition, float yValition, float zValition, float xVal2, float yVal2, float zVal2, bool stat, SheetSprite sprite) : position(x,y,z), velocity(xValition,yValition,zValition), acceleration(xVal2,yVal2,zVal2), isStatic(stat), sprite(sprite) {}
    
    SheetSprite sprite;
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    bool isStatic;
    EntityType entityType;
};


//globals
vector<Entity> entities;
Entity player;
Entity cherry;
float g = 0.45f;
SheetSprite sprite;
SheetSprite cherrySpr;
SheetSprite gameSprite;
int cherryCounter = 0;


GLuint LoadTexture(const char *filePath) {
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        cout << "Unable to load image. Make sure to path is correct\n";
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return retTexture;
}

void DrawText(ShaderProgram* program, int fontTexture, string text, float size, float spacing) {
    Matrix modelviewMatrix;
    float texture_size = 1.0 / 16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    
    for (size_t i = 0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
        
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    
    glUseProgram(program->programID);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

//Renders Gameplay and displays font
void Render(Matrix& modelviewMatrix, ShaderProgram& program) {
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(player.position.xVal, player.position.yVal, player.position.zVal);
    program.SetModelviewMatrix(modelviewMatrix);
    player.sprite.Draw(&program);
    
    for (size_t i = 0; i < entities.size(); i++) {
        modelviewMatrix.Identity();
        modelviewMatrix.Translate(entities[i].position.xVal, entities[i].position.yVal, entities[i].position.zVal);
        program.SetModelviewMatrix(modelviewMatrix);
        entities[i].sprite.Draw(&program);
    }
}


void CollidesWith(Entity& entity) {
    entity.collidedTop = false;
    entity.collidedBottom = false;
    entity.collidedLeft = false;
    entity.collidedRight = false;
}

void Update(float elapsed) {
    CollidesWith(player);
    for (size_t i = 0; i < entities.size(); i++) {
        if (player.position.xVal - sprite.width * 4 < entities[i].position.xVal + gameSprite.width * 2 &&
            player.position.xVal + sprite.width * 4 > entities[i].position.xVal - gameSprite.width * 2 &&
            player.position.yVal - sprite.height * 2 < entities[i].position.yVal + gameSprite.height * 2 &&
            player.position.yVal + sprite.height * 2 > entities[i].position.yVal - gameSprite.height * 2)
        {
            if (entities[i].isStatic) {
                if (player.position.yVal > entities[i].position.yVal){
                    player.collidedBottom = true;
                }
                else if (player.position.yVal < entities[i].position.yVal){
                    player.collidedTop = true;
                }
                if (player.position.xVal > entities[i].position.xVal + gameSprite.width * 2){
                    player.collidedRight = true;
                }
                else if (player.position.xVal < entities[i].position.xVal - gameSprite.width * 2){
                    player.collidedLeft = true;
                }
            }
            else if ((entities[i].entityType = ENTITY_CHERRY)) {
                swap(entities[i], entities[entities.size()-1]);
                entities.pop_back();
                cherryCounter++;
            }
        }
    }
    
    
    if ((player.collidedBottom && player.velocity.yVal < 0.0f) || (player.collidedTop && player.velocity.yVal > 0.0f))
        player.velocity.yVal = 0.0f;
    else {
        for (size_t i = 0; i < entities.size(); i++)
            entities[i].position.yVal -= player.velocity.yVal * elapsed;
        player.velocity.yVal -= g * elapsed;
    }
    
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_LEFT]) {
        if (!player.collidedRight) {
            for (size_t i = 0; i < entities.size(); i++) {
                entities[i].position.xVal += player.velocity.xVal * elapsed;
            }
        }
    }
    
    else if (keys[SDL_SCANCODE_RIGHT]) {
        if (!player.collidedLeft) {
            for (size_t i = 0; i < entities.size(); i++) {
                entities[i].position.xVal -= player.velocity.xVal * elapsed;
            }
        }
    }
    if (keys[SDL_SCANCODE_UP] && player.collidedBottom == true)
        player.velocity.yVal = 1.0f;
    
    else {
        if (keys[SDL_SCANCODE_DOWN] && player.collidedBottom == false)
            player.velocity.yVal = -1.0f;
    }
    
}



void createMap(SheetSprite gameSpr) {
    
    entities.push_back(Entity(-3.0f, -2.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(0.0f, -2.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-1.5f, -1.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(1.5f, -1.7f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(3.0f, -1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    
    entities.push_back(Entity(3.8f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-3.0f, -0.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(0.0f, -0.45f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-1.5f, 0.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(1.5f, 0.6f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    
  //  entities.push_back(Entity(3.0f, 1.9f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(1.5f, 2.6f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-3.0f, 1.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(0.0f, 1.6f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-1.5f, 2.4f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    
    entities.push_back(Entity(1.9f, 4.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-1.5f, 4.3f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(3.5f, 3.4f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-3.0f, 3.3f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(0.0f, 3.3f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
   
    entities.push_back(Entity(-1.5f, 5.9f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(1.5f, 6.3f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(0.0f, 5.4f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(3.5f, 5.4f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-3.6f, 5.1f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
  
    entities.push_back(Entity(0.2f, 9.2f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-1.5f, 7.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(1.5f, 8.1f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(3.5f, 7.2f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    entities.push_back(Entity(-3.5f, 6.9f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, true, gameSpr));
    
}


void addEntity(SheetSprite spr) {
    cherry = Entity(3.8f, 0.8f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, false, spr);
    entities.push_back(cherry);
    cherry = Entity(-3.0f, 3.7f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, false, spr);
    entities.push_back(cherry);
    cherry = Entity(1.9f,  5.5f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, false, spr);
    entities.push_back(cherry);
    cherry = Entity(-3.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, false, spr);
    entities.push_back(cherry);
    cherry = Entity(0.3f, 10.6f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, false, spr);
    entities.push_back(cherry);
    cherry.entityType = ENTITY_CHERRY;
}

enum GameState {TITLE_SCREEN, GAME_LEVEL, GAME_OVER};
int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("O'Rourke_HW", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix modelviewMatrix;
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float lastFrameTicks = 0.0f;
    
    GLuint gameFont = LoadTexture(RESOURCE_FOLDER"font1.png");
    GLuint playerTexture = LoadTexture(RESOURCE_FOLDER"george_0.png");
    GLuint tileSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
    
    sprite = SheetSprite(playerTexture, 10.0f / 192.0f, 9.0f / 187.0f, 32.0f / 192.0f, 32.0f / 187.0f, 0.3f);
    player = Entity(0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, false, sprite);
    player.entityType = ENTITY_PLAYER;
    cherrySpr = SheetSprite(tileSheet, 885.0f / 1024.0f, 15.0f / 1024.0f, 75.0f / 1024.0f, 120.0f / 1024.0f, 0.25f);
    addEntity(cherrySpr);
    gameSprite = SheetSprite(tileSheet, 702.0f / 1024.0f, 10.0f / 1024.0f, 134.0f / 1024.0f, 36.0f / 1024.0f, 0.3f);
    createMap(gameSprite);
    
    SDL_Event event;
    bool done = false;
    GameState state = TITLE_SCREEN;
    bool died = false;
    
    while (!done) {
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        float gameElapse = elapsed;
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            
            if (state == TITLE_SCREEN && keys[SDL_SCANCODE_SPACE]){
                state = GAME_LEVEL;
            }
            
            if (state == GAME_OVER && keys[SDL_SCANCODE_Q]) {
                done = true;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(modelviewMatrix);
        
        
        if (state == TITLE_SCREEN) {
            modelviewMatrix.Identity();
            modelviewMatrix.Scale(1.0f, 1.0f, 0.0f);
            modelviewMatrix.Translate(-2.0f, 0.5f, 0.0f);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(&program, gameFont, "HW4 PLATFORMER", 0.3f, 0.0f);
            modelviewMatrix.Identity();
            modelviewMatrix.Scale(1.0f, 1.0f, 0.0f);
            modelviewMatrix.Translate(-2.90f, 0.0f, 0.0f);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(&program, gameFont, "Press Space to Start", 0.3f, 0.0f);
        }
        
        else if (state == GAME_OVER) {
            if (died == true) {
                modelviewMatrix.Identity();
                modelviewMatrix.Scale(1.0f, 1.0f, 0.0f);
                modelviewMatrix.Translate(-1.7f, 0.5f, 0.0f);
                program.SetModelviewMatrix(modelviewMatrix);
                DrawText(&program, gameFont, "GAME OVER!!!", 0.3f, 0.0f);
            }
            else {
                modelviewMatrix.Identity();
                modelviewMatrix.Scale(1.0f, 1.0f, 0.0f);
                modelviewMatrix.Translate(-1.7f, 0.5f, 0.0f);
                program.SetModelviewMatrix(modelviewMatrix);
                DrawText(&program, gameFont, "YAY YOU WON!!", 0.3f, 0.0f);
            }
            modelviewMatrix.Identity();
            modelviewMatrix.Scale(1.0f, 1.0f, 0.0f);
            modelviewMatrix.Translate(-2.1f, 0.0f, 0.0f);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(&program, gameFont, "Press q to quit", 0.3f, 0.0f);
        }
        
        else {
            if (gameElapse > FIXED_TIMESTEP) {
                gameElapse = FIXED_TIMESTEP;
            }
            
            while (gameElapse >= FIXED_TIMESTEP) {
                gameElapse -= FIXED_TIMESTEP;
                Update(FIXED_TIMESTEP);
            }
            
            Update(gameElapse);
            glClear(GL_COLOR_BUFFER_BIT);
            program.SetModelviewMatrix(modelviewMatrix);
            program.SetProjectionMatrix(projectionMatrix);
            Update(elapsed);
            Render(modelviewMatrix, program);
            
            modelviewMatrix.Identity();
            modelviewMatrix.Scale(1.0f, 1.0f, 0.0f);
            modelviewMatrix.Translate(-3.30f, 1.7f, 0.0f);
            program.SetModelviewMatrix(modelviewMatrix);
            DrawText(&program, gameFont, "CHERRIES FOUND: " + to_string(cherryCounter) + "/5", 0.20f, 0.0f);
            
            if (cherryCounter == 5) {
                state = GAME_OVER;
            }
            
            if (player.velocity.yVal < -4.5) {
                state = GAME_OVER;
                died = true;
            }
        }
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}