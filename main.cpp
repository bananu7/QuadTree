#include <GL/freeglut.h>
#include <GL/GL.h>
#include <string>
#include <iostream>
#include <array>
#include <vector>
#include "QuadTree.h"

using std::cout;
using std::endl;

QuadTree tree;

void keyboard(unsigned char key, int x, int y);
void display(void);

void CheckForError()
{
    GLint E = glGetError();
    if (E != 0)
        _CrtDbgBreak();
}

void initGL ()
{
    //glutInitContextVersion (4,0);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("GLUT Test");
 
    glutKeyboardFunc(&keyboard);
    glutDisplayFunc(&display);
 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-2., 10., 10., -2.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
	initGL();
 
	tree.Add (1, 5, 4);
	tree.Add (2, 4, 5);
	tree.Add (3, 3, 1);

	const unsigned x = 3, y = 1;
	auto v = tree.Get(x,y);
	if (v)
		cout << "Value in [" << x << ", " << y << "] = " << v.get() << endl;
	else
		cout << "Value in [" << x << ", " << y << "] = " << "empty" << endl;

	glutMainLoop();
    return EXIT_SUCCESS;
}


void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '\x1B':
        exit(EXIT_SUCCESS);
        break;
    }
}
 
void display()
{
    glClearColor(0.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
 
	tree.DebugDraw();
 
    glFlush();
}