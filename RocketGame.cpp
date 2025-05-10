#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <GL/glut.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Checklist:
    - [x] 3D 
        - Contains 3D shapes
    - [x] Animation
        - The rocket moves up and down
        - Stars shine in the background
        - Earth rotates
    - [x] Keyboard & Mouse Handling
        - Keyboard Up & Down movement using WS, arrows or space
        - Keyboard ESC
        - Keyboard R to reset the game
        - Keyboard F11 to toggle fullscreen
        - Mouse click pauses the game
    - [x] Camera
        - The look at follows the rocket position
    - [x] Light
        - Applied ambient, diffuse and specular
    - [x] Texture
        - Applied to rocket, obstacles and earth
    - [x] Collision
        - When player (rocket) touches obstacle game over
        - Both player and obstacles have sphere collisions
*/


// game constants
const float PI = 3.14159265f;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GAME_SPEED = .05F;
const float ROCKET_SPEED = 0.1f;

// texture IDs
GLuint earthTexture;
GLuint rocketTexture;
GLuint obstacleTexture;
GLuint starTexture;

// earth variables
float earthRotationAngle = 0.0f;
const float earthRotationSpeed = 0.1f; 

struct GameObject {
    float x, y, z;  // position
    float radius;   // for collision detection
};
struct Rocket : GameObject {
    float velocity;
    float rotationY;
    bool isAlive;
};
struct Obstacle : GameObject {
    float rotationSpeed;
    float speed;
};

// game state
Rocket rocket;
std::vector<Obstacle> obstacles;
float gameTime = 0.0f;
float gameSpeed = .05f; // more speed more difficulty
bool isFullscreen = false;
bool gameOver = false;
bool gamePaused = false;

// camera parameters
float cameraDistance = 5.0f;
float cameraHeight = 2.0f;
float cameraAngle = 0.0f;


GLuint loadTexture(const char* filename);
void init();
void drawEarth();
void drawRocket();
void drawObstacle(Obstacle& obstacle);
void drawStars();
void reshape(int width, int height);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int key, int state, int x, int y);
void specialKeys(int key, int x, int y);
void keepRocketWithinBounds();
void moveObstacleForward();
void spawnObstaclesByChance();
void updateGame();
void checkCollisions();
void display();
void addObstacle();
void resetGame();

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutCreateWindow("Rocket Game");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, timer, 0);
    glutMouseFunc(mouse);

    init();

    glutMainLoop();
}

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (!image) {
        std::cout << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // image data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // free the image from memory
    stbi_image_free(image);

    return textureID;
}

void init() {
    // lighting
    GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position[] = { -1.0f, 1.0f, 1.0f, 0.0f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    // depth testing
    glEnable(GL_DEPTH_TEST);

    // enable texture
    glEnable(GL_TEXTURE_2D);

    // load textures
    earthTexture = loadTexture("earth.jpg");
    rocketTexture = loadTexture("rocket.jpg");
    obstacleTexture = loadTexture("rock.jpg");
    starTexture = loadTexture("space.jpg");

    // initialize random seed which makes the spawning random
    srand(static_cast<unsigned int>(time(0)));

    // reset game
    resetGame();
}

// to reset all of our variables
void resetGame() {
    rocket.x = 0.0f;
    rocket.y = 1.0f;  // slightly above Earth
    rocket.z = 0.0f;
    rocket.radius = 0.2f; // for collision
    rocket.velocity = 0.0f;
    rocket.rotationY = 0.0f;
    rocket.isAlive = true;

    obstacles.clear();

    gameTime = 0.0f;
    gameOver = false;
    gameSpeed = GAME_SPEED;

    earthRotationAngle = 0.0f;

    // add 2 initial obstacles
    for (int i = 0; i < 2; i++) {
        addObstacle();
    }
}

// add an obstacle to our view
void addObstacle() {
    Obstacle obstacle;

    // decide if obstacle comes from left or right side, but it doesnt work :"(
    bool fromLeft = (rand() % 2 == 0);

    obstacle.z = rocket.z; // it stays in the same layer as the rocket (unity vibes)

    if (fromLeft) {
        obstacle.x = -8.0f; // left
    }
    else {
        obstacle.x = 8.0f;  // right
    }

    obstacle.y = 0.5f + static_cast<float>(rand() % 40) / 10.0f; // random height

    obstacle.radius = 0.3f + static_cast<float>(rand() % 20) / 500.0f;
    obstacle.rotationSpeed = static_cast<float>(rand() % 100) / 100.0f;
    obstacle.speed = static_cast<float>(rand() % 5);

    obstacles.push_back(obstacle);
}

// keyboard functions ----------------------
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
    case 'W':
    case ' ':
        // up
        rocket.velocity += 0.02f;
        break;
    case 's':
    case 'S':
        // down
        rocket.velocity -= 0.01f;
        break;
    case 'r':
    case 'R':
        // reset game if it is over only
        if (gameOver)
            resetGame();
        break;
    case 27:  // ESC key
        exit(0);
        break;
    }
}
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        gamePaused = !gamePaused;
}
void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        rocket.velocity += 0.02f;
        break;
    case GLUT_KEY_DOWN:
        rocket.velocity -= 0.01f;
        break;
    case GLUT_KEY_F11:
        isFullscreen = !isFullscreen;

        if (isFullscreen) 
            glutFullScreen();
        else {
            glutReshapeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
            glutPositionWindow(100, 100);
        }
        break;
    }
}
// -----------------------------------------

// update functions ------------------------
void keepRocketWithinBounds()
{
    if (rocket.y < 0.8f) {
        rocket.y = 0.8f;
        rocket.velocity = 0.0f;
    }
    if (rocket.y > 4.0f) {
        rocket.y = 4.0f;
        rocket.velocity = 0.0f;
    }
}
void moveObstacleForward()
{
    for (auto& obstacle : obstacles) {
        // move forward
        obstacle.x += gameSpeed * 2.0f;

        if (obstacle.x < 0) {
            obstacle.x += gameSpeed * 0.5f; // move right if on left side
        }
        else if (obstacle.x > 0) {
            obstacle.x -= gameSpeed * 0.5f; // move left if on right side
        }
    }
}
void spawnObstaclesByChance()
{
    // so it does not spawn every frame
    int spawnChance = 3;

    if (rand() % 100 < spawnChance) {
        addObstacle();
    }
}
void checkCollisions() {
    if (!rocket.isAlive) return;

    for (auto& obstacle : obstacles) {
        float dx = rocket.x - obstacle.x;
        float dy = rocket.y - obstacle.y;
        float dz = rocket.z - obstacle.z;
        float distance = sqrt(dx * dx + dy * dy + dz * dz);

        if (distance < (rocket.radius + obstacle.radius)) {
            // collision detected!
            rocket.isAlive = false;
            gameOver = true;
            std::cout << "Game Over!\n"; // make sure it is working 
            break;
        }
    }
}
// -----------------------------------------
void updateGame() {
    // so every thing stops when game is over
    if (gameOver || gamePaused) return;

    // update game time
    gameTime += gameSpeed;

    // update game difficulty with time
    gameSpeed += 0.0001;

    // update rocket position
    rocket.y += rocket.velocity;

    // apply gravity
    rocket.velocity -= 0.001f;

    keepRocketWithinBounds();
    moveObstacleForward();
    spawnObstaclesByChance();
    checkCollisions();
}

// drawing functions -----------------------
void drawEarth() {
    glPushMatrix();

    glTranslatef(0.0f, -20.0f, 0.0f);

    // each time update is called rotate earth
    glRotatef(earthRotationAngle, 0.0f, 0.0f, 1.0f);

    // texture
    if (earthTexture) {
        glBindTexture(GL_TEXTURE_2D, earthTexture);
    }

    // sphere
    GLUquadricObj* earth = gluNewQuadric();
    gluQuadricTexture(earth, GL_TRUE);
    gluSphere(earth, 20.0f, 32, 32);
    gluDeleteQuadric(earth);

    glPopMatrix();
}
void drawRocket() {
    if (!rocket.isAlive) return;

    glPushMatrix();

    glTranslatef(rocket.x, rocket.y, rocket.z);

    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    if (rocketTexture) {
        glBindTexture(GL_TEXTURE_2D, rocketTexture);
    }

    // rocket body (cylinder)
    GLUquadricObj* body = gluNewQuadric();
    gluQuadricTexture(body, GL_TRUE);
    gluCylinder(body, 0.1f, 0.1f, 0.4f, 12, 12);

    // rocket cone
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.4f);  // top of cylinder
    GLUquadricObj* nose = gluNewQuadric();
    gluQuadricTexture(nose, GL_TRUE);
    gluCylinder(nose, 0.1f, 0.0f, 0.2f, 12, 12);
    gluDeleteQuadric(nose);
    glPopMatrix();

    // rocket base
    GLfloat baseMaterial[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, baseMaterial);

    // base right
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);    // Base center
    glTexCoord2f(0, 1); glVertex3f(0.15f, 0.0f, -0.1f);  // Right tip
    glTexCoord2f(1, 0); glVertex3f(0.0f, 0.0f, -0.2f);   // Back center
    glEnd();

    // base left
    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0); glVertex3f(0.0f, 0.0f, 0.0f);     // Base center
    glTexCoord2f(0, 1); glVertex3f(-0.15f, 0.0f, -0.1f);  // Left tip
    glTexCoord2f(1, 0); glVertex3f(0.0f, 0.0f, -0.2f);    // Back center
    glEnd();

    gluDeleteQuadric(body);
    glPopMatrix();
}
void drawObstacle(Obstacle& obstacle) {
    glPushMatrix();
    glTranslatef(obstacle.x, obstacle.y, obstacle.z);
    glRotatef(gameTime * 50.0f * obstacle.rotationSpeed, 1.0f, 1.0f, 0.0f);

    if (obstacleTexture) {
        glBindTexture(GL_TEXTURE_2D, obstacleTexture);
        glEnable(GL_TEXTURE_2D);
    }

    // a temporary quadric for texture coordinates
    GLUquadricObj* sphere = gluNewQuadric();
    gluQuadricTexture(sphere, GL_TRUE);  // enable texture coordinates
    gluSphere(sphere, obstacle.radius, 12, 12);
    gluDeleteQuadric(sphere);

    glPopMatrix();
}
void drawStars() {
    glPushMatrix();
    glDisable(GL_LIGHTING);

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);

    const int numStars = 2000;

    // star size
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 500; i++) {
        float x = -50.0f + static_cast<float>(rand() % 10000) / 100.0f;
        float y = -50.0f + static_cast<float>(rand() % 10000) / 100.0f;
        float z = -50.0f + static_cast<float>(rand() % 10000) / 100.0f;

        // blueish white?
        glColor3f(0.8f, 0.8f, 1.0f);
        glVertex3f(x, y, z);
    }
    glEnd();

    // reset rendering state
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SMOOTH);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}
// -----------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    gluLookAt(
        0.0f, cameraHeight, cameraDistance,  // position
        0.0f, rocket.y, rocket.z - 2.0f,   // look at rocket
        0.0f, 1.0f, 0.0f                   // up
    );

    drawStars();

    drawEarth();

    drawRocket();

    for (auto& obstacle : obstacles) {
        drawObstacle(obstacle);
    }

    // display text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // game over message 
    if (gameOver) {
        std::string gameOverText = "Game Over! Press 'R' to restart.";
        glColor3f(1.0f, 0.0f, 0.0f); // red
        glRasterPos2i(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2);
        for (char c : gameOverText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void timer(int value) {
    // update earth rotation, even if game is over (looks nicer)
    earthRotationAngle += earthRotationSpeed;
    if (earthRotationAngle > 360.0f) {
        earthRotationAngle -= 360.0f;
    }

    updateGame();

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // ~60 FPS
}
void reshape(int width, int height) {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
}