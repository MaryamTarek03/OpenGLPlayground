#include <Windows.h>
#include <GL\glut.h>
#include <iostream>

// sun center
float x = 0.0f;
float y = 5.0f;

float sunAngle = 0.0f;
const float PI = 3.14159265358979323846f;
float sunUpTime = 2.0f;
bool isDay = true, checked = false, uptimeChecked = false;

void drawHouse() {
    // main house structure
    glColor3f(0.8f, 0.6f, 0.4f); // brown
    glBegin(GL_QUADS);
    glVertex2f(-3.0f, -7.5f);  // bottom-left
    glVertex2f(3.0f, -7.5f);   // bottom-right
    glVertex2f(3.0f, -3.5f);   // top-right
    glVertex2f(-3.0f, -3.5f);  // top-left
    glEnd();

    // roof
    glColor3f(0.5f, 0.0f, 0.0f); // red
    glBegin(GL_TRIANGLES);
    glVertex2f(-3.5f, -3.5f); // bottom-left
    glVertex2f( 3.5f, -3.5f); // bottom-right
    glVertex2f( 0.0f,  0.0f); // top
    glEnd();

    // door
    glColor3f(0.4f, 0.2f, 0.0f); // dark brown
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -7.5f);  // bottom-left
    glVertex2f( 1.0f, -7.5f);  // bottom-right
    glVertex2f( 1.0f, -5.0f);  // top-right
    glVertex2f(-1.0f, -5.0f);  // top-left
    glEnd();

    // windows
    glColor3f(0.8f, 0.8f, 1.0f); // light blue

    // left window
    glBegin(GL_QUADS);
    glVertex2f(-2.5f, -6.0f);  // bottom-left
    glVertex2f(-1.5f, -6.0f);  // bottom-right
    glVertex2f(-1.5f, -5.0f);  // top-right
    glVertex2f(-2.5f, -5.0f);  // top-left
    glEnd();

    // right window
    glBegin(GL_QUADS);
    glVertex2f(1.5f, -6.0f);   // bottom-left
    glVertex2f(2.5f, -6.0f);   // bottom-right
    glVertex2f(2.5f, -5.0f);   // top-right
    glVertex2f(1.5f, -5.0f);   // top-left
    glEnd();
}

void drawGround() {
    glColor3f(0.0f, 0.5f, 0.0f); // Green
    glBegin(GL_QUADS);
    glVertex2f(-20.0f, -7.5f );  // bottom-left
    glVertex2f( 20.0f, -7.5f );  // bottom-right
    glVertex2f( 20.0f, -10.0f);  // top-right
    glVertex2f(-20.0f, -10.0f);  // top-left
    glEnd();
}

void drawSun()
{
    float sunRadius = 2.0f;
    int triangleNumber = 360;

    if (isDay)
        glColor3f(1.0f, 1.0f, 0.0f); // yellow (day)
    else 
        glColor3f(0.8f, 0.8f, 0.8f); // grey (grey)

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // center point

    for (int i = 0; i <= triangleNumber; i++) 
    {
        // x = r.cos(angle);
        // y = r.sin(angle);
        // we add them to the center point because it changes
        float angle = i * sunRadius * PI / triangleNumber; // radian angle
        glVertex2f(x + sunRadius * cos(angle), y + sunRadius * sin(angle));
    }
    glEnd();
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT); // clear the screen before drawing
    glLoadIdentity();
   
    drawSun();
    drawHouse();
    drawGround();

    //glutSwapBuffers();
    
    glFlush();
}

// handle window resize
void reshape(int width, int height) {
    // set the viewport to the entire window
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspectRatio = (float)width / (float)height;
    if (width >= height) 
        gluOrtho2D(-10.0 * aspectRatio, 10.0 * aspectRatio, -10.0, 10.0);
    else gluOrtho2D(-10.0, 10.0, -10.0 / aspectRatio, 10.0 / aspectRatio);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    if (isDay)
        glClearColor(0.7f, 0.3f, 0.2f, 1.0f);
    else glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void update(int value) {
    if (sunUpTime <= 0)
    {
        // rotation center (mid bottom)
        float centerX = 0.0f;
        float centerY = -10.0f;

        // rotation radius
        float radius = 15.0f;

        // make sure it's faster to get up from under the viewport
        if (y > -10)
            sunAngle += 0.01f;
        else sunAngle += 0.1f;


        if (sunAngle > 2.0f * PI)
            sunAngle -= 2.0f * PI;

        // reassign circle center
        x = centerX + radius * sin(sunAngle);
        y = centerY + radius * cos(sunAngle);

        // update day and night
        if (y <= -10 && !checked)
        {
            isDay = !isDay;
            checked = true;
        }
        else if (y > -10)
            checked = false;

        // change background color
        if (isDay)
            glClearColor(0.7f, 0.3f, 0.2f, 1.0f);
        else glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else sunUpTime -= 0.01;

    if ((int)x == 0 && !uptimeChecked && y > -10)
    {
        sunUpTime = 2.0f;
        uptimeChecked = true;
    }
    else if (x > 1 || x < -1)
        uptimeChecked = false;

    // re-render the scene
    glutPostRedisplay();

    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv)
{
    float height = GetSystemMetrics(SM_CYSCREEN), width = GetSystemMetrics(SM_CXSCREEN);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(width / 4, height / 4);
    glutInitWindowSize(width / 2, height / 2);

    glutCreateWindow("A cute house!");

    glutTimerFunc(16, update, 0);

    init();

    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutMainLoop();
}