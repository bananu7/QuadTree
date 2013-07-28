#include <OpenGLWindow.hpp>
#include <GL/GL.h>
#include <string>
#include <iostream>
#include <array>
#include <vector>

#include "test.h"
#include "QuadTree.h"

using std::cout;
using std::endl;

QuadTree tree(100.f, 2);


void keyboard(unsigned char key, int x, int y);
void display(void);

void CheckForError() {
    GLint E = glGetError();
    if (E != 0)
        _CrtDbgBreak();
}

void DrawLine(float x, float y, float dx, float dy, float length) {
    glBegin(GL_LINES);
        glVertex2f(x, y);
        glVertex2f(x + dx*length, y + dy*length);
    glEnd();
}

void initGL () {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 101., 101., -1., -10., 10.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    oglw::Window w;
    initGL();
    w.keyupCallback = [](oglw::KeyInfo k){
        switch (k.key) {
        case '\x1B':
            exit(0);
        }
    };
    w.resizeCallback = [](unsigned x, unsigned y) {
        initGL();
        glViewport(0, 0, x, y);
    };

    bool mousePressed = false;

    w.mousedownCallback = [&](oglw::MouseInfo){ mousePressed = true; };
    w.mouseupCallback = [&](oglw::MouseInfo){ mousePressed = false; };
    w.mousemoveCallback = [&](oglw::MouseInfo mi) {
        if (mousePressed) {
            float x = mi.normX * 100.f;
            float y = mi.normY * 100.f;
            tree.set(x, y, 5);
        }
    };

    w.displayFunc = display;

    //test();
    tree.set(30.f, 30.f, 5);
    tree.set(51.f, 51.f, 5);

    while (w.display(), w.process()) { }
}

 
void display() {
    glClearColor(0.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
 
    tree.DebugDraw();

    glColor3ub(255, 0, 0);
    glPointSize(3.);
    glBegin(GL_POINTS);
        glVertex2f(30.f, 30.f);
        glVertex2f(51.f, 51.f);
    glEnd();

    {
        float x = 5.f, y = 5.f, dx = 5.f, dy = 3.f;

        // This magical piece of code normalizes the vector,
        // because I'm too lazy to calculate it by hand.
        float len = std::sqrt(dx*dx+dy*dy);
        dx /= len; dy /= len;

        glColor3ub(0, 0, 250);
        DrawLine(x, y, dx, dy, 100.f);
        tree.raycast(QuadTree::Ray{ x, y, dx, dy });
    }
 
    glFlush();
}
