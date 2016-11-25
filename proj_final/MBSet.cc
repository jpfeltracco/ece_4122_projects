// Calculate and display the Mandelbrot set
// ECE4893/8893 final project, Fall 2011

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "complex.h"

using namespace std;

// Min and max complex plane values
//Complex minC(-2.0, -1.2);
//Complex maxC(1.0, 1.8);
// TODO: ASK WHY THE PREVIOUS MIN MAX VALUES WERE CHOSEN
Complex minC(-2, -1.5);
Complex maxC(1, 1.5);
int      maxIt = 2000;     // Max iterations for the set computations

const int N(512);

//int pixels[N * N][3];
uint8_t pixels[N][N][3];

void display()
{
    //glWindowPos2f(10, 200);
    glLoadIdentity();
    glRasterPos2f(-1, -1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background) // Your OpenGL display code here
    // Draw a Red 1x1 Square centered at origin
    /*
    glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        glVertex2f(-0.5f, -0.5f);    // x, y
        glVertex2f( 0.5f, -0.5f);
        glVertex2f( 0.5f,  0.5f);
        glVertex2f(-0.5f,  0.5f);
    glEnd();
    */

    glDrawPixels(N, N, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    glFlush();
}

void init()
{ // Your OpenGL initialization code here
}

void reshape(int w, int h)
{ // Your OpenGL window reshape code here

    cout << "Reshape called" << endl;
    //display();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Clear the matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, w, -1.0, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutPostRedisplay();
}

//void mouse(int button, int state, int x, int y)
//{ // Your mouse click processing here
// state == 0 means pressed, state != 0 means released
// Note that the x and y coordinates passed in are in
// PIXELS, with y = 0 at the top.
//}

//void motion(int x, int y)
//{ // Your mouse motion here, x and y coordinates are as above
//}

//void keyboard(unsigned char c, int x, int y)
//{ // Your keyboard processing here
//}

int main(int argc, char** argv)
{
    // Initialize OpenGL, but only on the "master" thread or process.

    glutInit(&argc, argv); // Initialize GLUT
    glutCreateWindow("Mandelbrot"); // Create a window with the given title
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(512, 512); // Set the window's initial width & height
    glutInitWindowPosition(0, 0); // Position the window's initial top-left corner
    glutReshapeFunc(reshape);

    const double deltaWidth = (maxC - minC).real / N;
    const double deltaHeight = (maxC - minC).imag / N;
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            double real = deltaWidth * x + minC.real;
            double imag = deltaHeight * y + minC.imag;

            Complex c(real, imag);
            Complex k(c);

            unsigned numIterations = 0;
            bool iterationTrip = false;
            while (k.Mag().real < 2.0) {
                k = k * k + c;
                numIterations++;
                if (numIterations > maxIt) {
                    iterationTrip = true;
                    break;
                }
            }
            
            if (!iterationTrip) {
                cout << numIterations << endl;
                pixels[y][x][0] = (numIterations * 953)  % 255;
                pixels[y][x][1] = (numIterations * 6269) % 255;
                pixels[y][x][2] = (numIterations * 4999) % 255;

                if (pixels[y][x][0] + pixels[y][x][1] + pixels[y][x][2] < 20) {
                    pixels[y][x][0] = (pixels[y][x][0] + 50);
                    pixels[y][x][1] = (pixels[y][x][1] + 132);
                    pixels[y][x][2] = (pixels[y][x][2] + 200);
                }
            } else {
                pixels[y][x][0] = 0;
                pixels[y][x][1] = 0;
                pixels[y][x][2] = 0;
            }
        }
    }
    glutDisplayFunc(display); // Register display callback handler for window re-paint
    glutIdleFunc(display);
    glutMainLoop();  // Enter the event-processing loop

    return 0;
}

