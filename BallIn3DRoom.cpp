#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glut.h>

// room dimensions
const float ROOM_SIZE = 10.0f;
const float BALL_RADIUS = 0.5f;

// ball properties
float ballPos[3] = { 0, BALL_RADIUS, 0 };
// y is 0 because we dont want our ball to fly 
float ballVelocity[3] = { 0.1f, 0.0f, 0.1f };

// camera properties
// camera's x, y and z
float cameraPos[3] = { 0, ROOM_SIZE / 2, ROOM_SIZE };
// camera's center of the focus i think (x, y, z)
float lookAt[3] = { 0, 0, 0 };
// determine which axes is up i think too
float up[3] = { 0, 1, 0 };

GLuint textures[6];

// lighting
GLfloat light_position[] = { ROOM_SIZE, ROOM_SIZE, ROOM_SIZE, 1.0f };

void loadTexture(const char* filename, GLuint textureID) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);

    glBindTexture(GL_TEXTURE_2D, textureID);
    // upload to GPU memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    // texture set to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // x
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // y
    // linear: mix between colors near the center to cover the rest of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // free memory
    stbi_image_free(image);
}

void initTextures() {
    // generate the textures IDs
    glGenTextures(6, textures);
    loadTexture("golden-leaves-texture.jpg", textures[0]); // i have one texture only
}

void drawWalls() {
    glEnable(GL_TEXTURE_2D);

    // floor
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-ROOM_SIZE, 0, -ROOM_SIZE); // bottom_left
    glTexCoord2f(10, 0); glVertex3f( ROOM_SIZE, 0, -ROOM_SIZE); // bottom right
    glTexCoord2f(10, 10); glVertex3f( ROOM_SIZE, 0,  ROOM_SIZE); // top right
    glTexCoord2f(0, 10); glVertex3f(-ROOM_SIZE, 0,  ROOM_SIZE); // top left
    glEnd();

    // ceiling
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE);
    glTexCoord2f(0, 0); glVertex3f(-ROOM_SIZE, ROOM_SIZE,  ROOM_SIZE);
    glTexCoord2f(1, 0); glVertex3f( ROOM_SIZE, ROOM_SIZE,  ROOM_SIZE);
    glTexCoord2f(1, 1); glVertex3f( ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE);
    glEnd();

    // front wall
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-ROOM_SIZE, 0, ROOM_SIZE);
    glTexCoord2f(1, 0); glVertex3f( ROOM_SIZE, 0, ROOM_SIZE);
    glTexCoord2f(1, 1); glVertex3f( ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);
    glTexCoord2f(0, 1); glVertex3f(-ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);
    glEnd();

    // back wall
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f(-ROOM_SIZE, 0, -ROOM_SIZE);
    glTexCoord2f(0, 0); glVertex3f( ROOM_SIZE, 0, -ROOM_SIZE);
    glTexCoord2f(0, 1); glVertex3f( ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE);
    glTexCoord2f(1, 1); glVertex3f(-ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE);
    glEnd();

    // left wall
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-ROOM_SIZE, 0, -ROOM_SIZE);
    glTexCoord2f(1, 0); glVertex3f(-ROOM_SIZE, 0,  ROOM_SIZE);
    glTexCoord2f(1, 1); glVertex3f(-ROOM_SIZE, ROOM_SIZE,  ROOM_SIZE);
    glTexCoord2f(0, 1); glVertex3f(-ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE);
    glEnd();

    // right wall
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 0); glVertex3f(ROOM_SIZE, 0, -ROOM_SIZE);
    glTexCoord2f(0, 0); glVertex3f(ROOM_SIZE, 0, ROOM_SIZE);
    glTexCoord2f(0, 1); glVertex3f(ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);
    glTexCoord2f(1, 1); glVertex3f(ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawBall() {
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 100.0 };

    // specular lighting
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // so translation does not affect 
    glPushMatrix();
    glTranslatef(ballPos[0], ballPos[1], ballPos[2]);
    // higher values = smoother sphere
    glutSolidSphere(BALL_RADIUS, 32, 32);
    // reset the translatef (like we used to do in assembly yarab el sabr)
    glPopMatrix();
}

const float DRIFT_STRENGTH = 0.15f; 

void updateBall() {
    for (int i = 0; i < 3; i++) {
        ballPos[i] += ballVelocity[i];

        if (ballPos[i] + BALL_RADIUS > ROOM_SIZE || ballPos[i] - BALL_RADIUS < -ROOM_SIZE) {
            // reverse velocity
            ballVelocity[i] *= -1;

            // apply simple drift ro z axes so it feels random
            // i think i am increasing the velocity infinitly but that is a problem for another day
            ballVelocity[2] += DRIFT_STRENGTH;

            // clamp position to prevent glitches "it skipped walls :')"
            ballPos[i] = (ballPos[i] > 0) ?
                ROOM_SIZE - BALL_RADIUS :
                -ROOM_SIZE + BALL_RADIUS;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(
        cameraPos[0], cameraPos[1], cameraPos[2],
        lookAt[0], lookAt[1], lookAt[2],
        up[0], up[1], up[2]);

    drawWalls();
    drawBall();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    float cameraSpeed = 0.5f; // how much it moves

    switch (key) {
        // change z
    case 'w': cameraPos[2] -= cameraSpeed; break;
    case 's': cameraPos[2] += cameraSpeed; break;
        // change x
    case 'a': cameraPos[0] -= cameraSpeed; break;
    case 'd': cameraPos[0] += cameraSpeed; break;
        // change y
    case 'q': cameraPos[1] += cameraSpeed; break;
    case 'e': cameraPos[1] -= cameraSpeed; break;
    case 27: exit(0); break; // ESC button
    }

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    float lookSpeed = 0.5f;

    switch (key) {
        // change y (up or down)
    case GLUT_KEY_UP: lookAt[1] += lookSpeed; break;
    case GLUT_KEY_DOWN: lookAt[1] -= lookSpeed; break;
        // change x (right or left)
    case GLUT_KEY_LEFT: lookAt[0] -= lookSpeed; break;
    case GLUT_KEY_RIGHT: lookAt[0] += lookSpeed; break;
    }

    glutPostRedisplay();
}

void init() {
    // enable depth, lightining, light 0
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // for the light 0, the parameter GL_POSITION, we gave it our light position
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    initTextures();
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    // 60 is eye angle
    gluPerspective(60, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void update(int v) {
    updateBall();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Room");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
}