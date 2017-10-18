
/*
 Lucas O'Rourke
 lor215
 
 HW3 Space Invaders
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
#include <vector>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.141592


SDL_Window* displayWindow;


//Implemented Struct -- Vector3 and coordsText
struct Vector3 {
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x;
    float y;
    float z;
};


//Coordinates -- Texture
struct coordsText {
    coordsText (float top, float down, float left, float right) : top(top), down(down), left(left), right(right) {}
    float top;
    float down;
    float left;
    float right;
};


//Sprite Sheet
class SheetSprite {
public:
    SheetSprite() {}
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) :
    textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}
    
    float size;
    float u;
    float v;
    float width;
    float height;
    int textureID;
    void Draw(ShaderProgram* program) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLfloat texCoords[] = {
            u, v + height,
            u + width, v,
            u, v,
            u + width, v,
            u, v + height,
            u + width, v + height
        };
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size
        };
        
        glUseProgram(program->programID);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
};


//Entity Class
class Entity {
public:
    
    Entity(GLuint textureID = 0.0f, float size = 1.0f, float ratio = 1.0f, Vector3 pos = Vector3(0.0f,0.0f,0.0f),coordsText font = coordsText(0.0f, 0.0f, 0.0f, 0.0f)): textureID(textureID), size(size), ratio(ratio), pos(pos), coordTex(font), velocity(0), direction(0) {
        width = (size*ratio)/2;
        height = size/2;
    }
    
    void Draw(ShaderProgram& program, SheetSprite sprite) {
        if (!alive) return;
        Matrix modelviewMatrix;
        modelviewMatrix.Identity();
        modelviewMatrix.Translate(pos.x, pos.y, 0);
        program.SetModelviewMatrix(modelviewMatrix);
        sprite.Draw(&program);
    }
    
    void Update(float elapsed) {
        pos.x += velocity * cos(direction*PI/180) * elapsed;
        pos.y += velocity * sin(direction*PI/180) * elapsed;
    }
    
    bool Collision(const Entity& object) {
        if (!object.alive || !alive)
            return false;
        else if (!(pos.x + width <= object.pos.x - object.width || pos.x - width >= object.pos.x + object.width || pos.y + height <= object.pos.y - object.height || pos.y - height >= object.pos.y + object.height)) {
            return true;
        }
        else
            return false;
    }
    
    Vector3 pos;
    GLuint textureID;
    float velocity;
    float direction;
    coordsText coordTex;
    bool alive = true;
    float size;
    float ratio;
    float width;
    float height;
    
};


//DONE - draw text
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing, float x, float y) {
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for (int i = 0; i < text.size(); i++) {
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
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    Matrix modelviewMatrix;
    
    modelviewMatrix.Identity();
    modelviewMatrix.Translate(x, y, 0);
    
    program->SetModelviewMatrix(modelviewMatrix);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, text.size()*6);
    
}


//DONE - load texture
GLuint LoadTexture(const char *img_path){
    int width, height, item;
    unsigned char* img = stbi_load(img_path, &width, &height, &item, STBI_rgb_alpha);
    
    if (img == NULL){
        std::cout << "Cannot show image. Please check directory!\n";
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


enum GameState {TITLE_SCREEN, GAME_LEVEL};
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
    
    GLuint sprite = LoadTexture(RESOURCE_FOLDER"invaders.png");
    GLuint sprFont = LoadTexture(RESOURCE_FOLDER"font1.png");
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix modelviewMatrix;
    
    SheetSprite player(sprite, 224.0f / 1024.0f, 832.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.40f);
    SheetSprite bullet(sprite, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.30f);
    SheetSprite enemy(sprite, 120.0f / 1024.0f, 604.0f / 1024.0f, 104.0f / 1024.0f, 84.0f / 1024.0f, 0.40f);
    
    Entity Shooter(sprite, 0.4, 1.2, Vector3(0.0f, -1.5f, 0.0f),  coordsText(0.0f,0.8679f,0.0f, 1.025f));
    
    Entity Enemies[3][6];
    for (int a = 0; a < 3; a++){
        for (int b = 0; b < 6; b++){
            Enemies[a][b] = Entity(sprite, 0.4f, 1.1084, Vector3(-2.5f+b, 0.5f+a*0.5f, 1.0f), coordsText(0.0, 0.8679f, 0.0, 1.0f));
            Enemies[a][b].velocity = 1.0f;
            Enemies[a][b].direction = 180.0f;
        }
    }
    
    //Bullets -- What Shooter is using to destroy Enemies
    Entity Bullet[8];
    for (int i = 0; i < 8; i++) {
        Bullet[i] = Entity(sprite, 0.2f, 0.2321f, Vector3(5.0f, 5.0f, 0.0f), coordsText(0.8773f, 1.0f, 0.0f, 0.3522));
    }
    
    GameState state = TITLE_SCREEN;
    
    glUseProgram(program.programID);
    
    float lastFrameTicks = 0.0f;
    float initialBullet = 0;
    int bullNum = 0;
    bool playing = false;
    SDL_Event event;
    bool done = false;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int enemiesLeft = 18;
    float lastBullFired = 0.0f;
    int score = 0;
    
    while (!done) {
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        initialBullet += elapsed;
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        while (SDL_PollEvent(&event)) {
            
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            
            if (state == TITLE_SCREEN && keys[SDL_SCANCODE_SPACE]){
                state = GAME_LEVEL;
                lastBullFired = ticks;
            }
            
            else if
                (state == GAME_LEVEL && keys[SDL_SCANCODE_SPACE]) {
                    if ((ticks - lastBullFired)*100 > 25.0f) {
                        bullNum++;
                        if (bullNum >= 8.0){
                            bullNum = 0.0f;
                        }
                        Bullet[bullNum].alive = true;
                        Bullet[bullNum].pos.x = Shooter.pos.x;
                        Bullet[bullNum].pos.y = Shooter.pos.y + Shooter.height;
                        Bullet[bullNum].velocity = 2.0f;
                        Bullet[bullNum].direction = 90.0f;
                        lastBullFired = ticks;
                    }
                }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(modelviewMatrix);
        
        if (state == TITLE_SCREEN) {
            DrawText(&program, sprFont, "SPACE INVADERS", 0.33f, -0.05f, -1.8f, 0.5f);
            DrawText(&program, sprFont, "Press Space to Start", 0.3f, 0.0f, -2.8f, 0.0f);
        }
        
        else { //draw the enemies
            DrawText(&program, sprFont, ("SCORE: "+std::to_string(score)), 0.3f, 0.0f, -2.8f, 1.85f);
            for (int i = 0; i < 3; i++){
                for (int j = 0; j < 6; j++){
                    Enemies[i][j].Update(elapsed);
                    Enemies[i][j].Draw(program, enemy);
                }
            }
        }
        
        if (keys[SDL_SCANCODE_LEFT] && Shooter.pos.x >= -3.3){
            Shooter.pos.x -= 2.0 * elapsed;
        }
        else if (keys[SDL_SCANCODE_RIGHT] && Shooter.pos.x <= 3.3){
            Shooter.pos.x += 2.0 * elapsed;
        }
        
        Shooter.Draw(program, player); //draw shooter
        
        for (int i = 0; i < 8; i++){ //draw bullets
            Bullet[i].Update(elapsed);
            Bullet[i].Draw(program, bullet);
        }
        
        float checkDirec = 0.0f;
        
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 6; j++){
                for (int k = 0; k < 8; k++) { //check collisions
                    if (Enemies[i][j].Collision(Bullet[k])) {
                        Enemies[i][j].alive = false;
                        Bullet[k].alive = false;
                        enemiesLeft--;
                        score += 10;
                        if (enemiesLeft == 0) done = true;
                    }
                }
                
                if (Enemies[0][0].pos.x <= -3.1f) {
                    checkDirec = 2.0f;
                }
                
                if (Enemies[0][5].pos.x >= 3.1f) {
                    checkDirec = 1.0f;
                }
                
                if (checkDirec == 2.0f) {
                    Enemies[i][j].direction = 0.0f;
                    Enemies[i][j].pos.y -= 0.05f;
                    if (Enemies[i][j].pos.y <= -1.4f) done = true;
                }
                
                if (checkDirec == 1.0f){
                    Enemies[i][j].direction = 180.0f;
                    Enemies[i][j].pos.y -= 0.05f;
                    if (Enemies[i][j].pos.y <= -1.4f) done = true;
                }
            }
        }
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    
    
    SDL_Quit();
    return 0;
}