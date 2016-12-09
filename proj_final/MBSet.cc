// Calculate and display the Mandelbrot set
// ECE4893/8893 final project, Fall 2011

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "complex.h"

using namespace std;

// Min and max complex plane values
Complex minC(-2.0, -1.2);
Complex maxC(1.0, 1.8);

// These values are more centered
//Complex minC(-2, -1.5);
//Complex maxC(1, 1.5);

int winX = 512;
int winY = 512;
int maxIt = 2000;     // Max iterations for the set computations

// Contains the computed mandelbrot set represented as the iteration numbers of each
vector< vector<int> > v;

float color_vec[3];

int get_num_iterations(Complex c)
{
    Complex k(c);

    unsigned numIterations = 0;
    while (k.Mag().real < 2.0) {
        k = k * k + c;
        numIterations++;
        if (numIterations > maxIt) {
            return -1;
        }
    }

    return numIterations;
}

void get_color_iterations(int num_iterations, float* color)
{
    color[0] = 0.0f; color[1] = 0.0f; color[2] = 0.0f;
    if (num_iterations == -1) {
        return;
    }

    // These essentially are random, but stay the same based on the
    // iteration number. They also avoid the color black which is
    // reserved for values in the mandelbrot set.
    color[0] = (num_iterations * 11)  % 205 + 50;
    color[1] = (num_iterations * 34) % 205 + 50;
    color[2] = (num_iterations * 4999) % 205 + 50;

    color[0] /= 255;
    color[1] /= 255;
    color[2] /= 255;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background) // Your OpenGL display code here
    glBegin(GL_POINTS);
    float pixel_step_x = (maxC.real - minC.real) / winX;
    float pixel_step_y = (maxC.imag - minC.imag) / winY;

    for (int y = 0; y < winY; ++y) {
        for (int x = 0; x < winX; ++x) {
            float real_val = x * pixel_step_x + minC.real;
            float imag_val = y * pixel_step_y + minC.imag;

            Complex c(real_val, imag_val);
            int iters = get_num_iterations(c);

            get_color_iterations(iters, color_vec);
            // set color of the point
            glColor3fv(color_vec);
            glVertex3f(real_val, imag_val, 0.0f);
        }
    }
    glEnd();

    glutSwapBuffers();
}

void reshape(int w, int h)
{ // Your OpenGL window reshape code here

    glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer (background) // Your OpenGL display code here

    winX = w; winY = h;

    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    // Initialize OpenGL, but only on the "master" thread or process.
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set background color to black and opaque

    // Window initialization
    glutInitWindowSize(winX, winY); // Set the window's initial width & height
    glutInitWindowPosition(0, 0); // Position the window's initial top-left corner
    glutCreateWindow("Mandelbrot"); // Create a window with the given title

    glViewport (0, 0, winX, winY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(minC.real, maxC.real, minC.imag, maxC.imag, -1.0, 1.0);

    glutReshapeFunc(reshape); // Window resize callback
    glutDisplayFunc(display); // Register display callback handler for window re-paint

    glutMainLoop();  // Enter the event-processing loop

    return 0;
}

