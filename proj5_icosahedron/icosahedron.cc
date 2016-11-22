// Draw an Icosahedron
// Jeremy Feltracco

#include <iostream>
#include <vector>
#include <valarray>
#include <math.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

#define NFACE 20
#define NVERTEX 12

#define X .525731112119133606 
#define Z .850650808352039932

float rot = 0;

// These are the 12 vertices for the icosahedron
static GLfloat vdata[NVERTEX][3] = {    
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};

// These are the 20 faces.  Each of the three entries for each 
// vertex gives the 3 vertices that make the face.
static GLint tindices[NFACE][3] = { 
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

int testNumber; // Global variable indicating which test number is desired
int cmdDepth; // depth commanded for parts 5 and 6

static const float DEG2RAD = M_PI/180;
static int updateRate = 10;

void drawRaisedLine(float* vert)
{
    const float raise = 1.005;
    float x = vert[0] * raise;
    float y = vert[1] * raise;
    float z = vert[2] * raise;

    glVertex3f(x, y, z);
}

void drawTriangle(float* vert1, float* vert2, float* vert3)
{
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_TRIANGLES);

    glVertex3fv(vert1);
    glVertex3fv(vert2);
    glVertex3fv(vert3);

    glEnd();

    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
    
    drawRaisedLine(vert1);
    drawRaisedLine(vert2);
    drawRaisedLine(vert3);

    glEnd();

}


void normalize(float* v)
{
    float mag = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    for (int i = 0; i < 3; ++i) v[i] = v[i] / mag;
}

void subdivide(float* vert1, float* vert2, float* vert3, int depth)
{
    if (!depth) {
        drawTriangle(vert1, vert2, vert3);
    } else {
        // new vector midpoints
        float vert12[3];
        float vert23[3];
        float vert13[3];

        // sum the two vectors to get midpoint, mag will be off but doesn't
        // matter because we can simply take magnitude of it.
        for (int i = 0; i < 3; ++i) {
            vert12[i] = vert1[i] + vert2[i];
            vert23[i] = vert2[i] + vert3[i];
            vert13[i] = vert1[i] + vert3[i];
        }

        normalize(vert12); normalize(vert23); normalize(vert13);

        // vectors are normalized, now we need to subdivide again
        --depth;
        subdivide(vert1, vert12, vert13, depth);
        subdivide(vert2, vert12, vert23, depth);
        subdivide(vert23, vert13, vert3, depth);
        // this is the extra triangle in the middle
        subdivide(vert12, vert13, vert23, depth);
    }
}

void drawIcosahedron(int depth)
{
    for (int i = 0; i < NFACE; ++i) {
        subdivide(vdata[tindices[i][0]],
                  vdata[tindices[i][1]],
                  vdata[tindices[i][2]],
                  depth);
    }
    glutSwapBuffers(); 
}

void reshape(int w, int h)
{
    glViewport(0,0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45, 1, 3.5, 10);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void initBeforeDraw()
{
    glEnable(GL_DEPTH_TEST);

    // clear all
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Clear the matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set the viewing transformation
    gluLookAt(0, 0, 5.0, 0, 0, 0.0, 0.0, 1.0, 0.0);
}


// Test cases.  Fill in your code for each test case
void Test1()
{
    initBeforeDraw();
    drawIcosahedron(0);
}

void Test2()
{
    initBeforeDraw();
    rot += 1;
    glRotatef(rot, 1.0, 1.0, 0.0);
    drawIcosahedron(0);
}


void Test3()
{
    initBeforeDraw();
    drawIcosahedron(1);
}

void Test4()
{
    initBeforeDraw();
    rot += 1;
    glRotatef(rot, 1.0, 1.0, 0.0);
    drawIcosahedron(1);
}

void Test5(int depth)
{
    initBeforeDraw();
    drawIcosahedron(depth);
}

void Test6(int depth)
{
    initBeforeDraw();
    rot += 1;
    glRotatef(rot, 1.0, 1.0, 0.0);
    drawIcosahedron(depth);
}

void display()
{
    initBeforeDraw();
    switch (testNumber) {
        case 1:
            Test1();
            break;
        case 2:
            Test2();
            break;
        case 3:
            Test3();
            break;
        case 4:
            Test4();
            break;
        case 5:
            Test5(cmdDepth);
            break;
        case 6:
            Test6(cmdDepth);
            break;
    }
}

void timer(int)
{
    glutPostRedisplay();
    glutTimerFunc(1000.0 / updateRate, timer, 0);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Usage: icosahedron testnumber" << endl;
        exit(1);
    }

    // Set the global test number
    testNumber = atol(argv[1]);

    if (testNumber == 5 || testNumber == 6) {
        if (argc < 3) {
            std::cout << "Usage: icosahedron {5,6} {depth}" << endl;
            exit(1);
        } else {
            cmdDepth = atol(argv[2]);
        }
    }


    // Initialize glut  and create your window here
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Circle");
    //select clearing (background) color
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    // Set your glut callbacks here
    glutReshapeFunc(reshape);
    glutTimerFunc(1000.0 / updateRate, timer, 0);

    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}

