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

pthread_barrier_t barrier;
pthread_mutex_t activeMutex;
pthread_mutex_t coutMutex;
pthread_cond_t  allDoneCondition;

int winX = 512;
int winY = 512;
int maxIt = 2000;     // Max iterations for the set computations

int activeThreads = 0;
// Contains the computed mandelbrot set represented by vectors of colors
vector< vector< vector<float> > > set;

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

void get_color_iterations(int num_iterations, vector<float>& color)
{
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

const int NUM_THREADS = 16;

void* calculate_set_thread(void* v)
{
    float pixel_step_x = (maxC.real - minC.real) / winX;
    float pixel_step_y = (maxC.imag - minC.imag) / winY;

    long thread_num = (long) v;

    //cout << "starting thread: " << thread_num << endl;
    const int cols_per_thread = winX / NUM_THREADS;
    int start = thread_num * cols_per_thread;
    // if we aren't perfectly divisible by 16, then let the last one do 
    // a few more columns
    int stop = thread_num == NUM_THREADS - 1 ? winX : start + cols_per_thread;

    for (int x = start; x < stop; ++x) {
        for (int y = 0; y < winY; ++y) {
            float real_val = x * pixel_step_x + minC.real;
            float imag_val = y * pixel_step_y + minC.imag;

            Complex c(real_val, imag_val);
            int iters = get_num_iterations(c);

            // fills the entry in set[x][y]
            get_color_iterations(iters, set[x][y]);
        }
    }


    pthread_mutex_lock(&activeMutex);
    activeThreads--;
    if (activeThreads == 0) {
        pthread_cond_signal(&allDoneCondition);
    }
    pthread_mutex_unlock(&activeMutex);
    return 0;
}

void calculate_set()
{
    pthread_t threads[NUM_THREADS];

    if (pthread_barrier_init(&barrier, NULL, NUM_THREADS)) {
        //cout << "Barrier couldn't be initialized" << endl;
    }

    pthread_mutex_init(&activeMutex, 0);
    pthread_mutex_init(&coutMutex, 0);
    pthread_cond_init(&allDoneCondition, 0);

    pthread_mutex_lock(&activeMutex);
    activeThreads = NUM_THREADS;
    // Create 16 threads
    for (long i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], 0, calculate_set_thread,  (void*) i);
        //cout << "threads: " << i << endl;
    }

    pthread_cond_wait(&allDoneCondition, &activeMutex);
    pthread_mutex_unlock(&activeMutex);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background) // Your OpenGL display code here
    cout << "display called" << endl;
    calculate_set();
    cout << "done calculating set" << endl;

    float pixel_step_x = (maxC.real - minC.real) / winX;
    float pixel_step_y = (maxC.imag - minC.imag) / winY;

    glBegin(GL_POINTS);
    for (int y = 0; y < winY; ++y) {
        for (int x = 0; x < winX; ++x) {
            glColor3fv(&set[x][y][0]);
            glVertex3f(x * pixel_step_x + minC.real, y * pixel_step_y + minC.imag, 0.0f);
        }
    }
    glEnd();

    glutSwapBuffers();
}

void reshape(int w, int h)
{ // Your OpenGL window reshape code here

    //glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer (background) // Your OpenGL display code here

    winX = w; winY = h;

    set.clear();
    for (int x = 0; x < winX; ++x) {
        set.push_back(vector< vector<float> >());
        for (int y = 0; y < winY; ++y) {
            set[x].push_back(vector<float>(3));
        }
    }

    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    // Initialize OpenGL, but only on the "master" thread or process.
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
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

    // The recalculation occurs when the display function is run, so for this
    // thread, we can simply enter the main loop.
    glutMainLoop();  // Enter the event-processing loop

    return 0;
}

