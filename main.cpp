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
QuadTree::Ray ray { 105.f, -5.f, -8.f, 6.f };

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
        float x = mi.normX * 100.f;
        float y = mi.normY * 100.f;
        if (mousePressed) {
            tree.set(x, y, 5);
        }

        ray.dx = (x - ray.x);
        ray.dy = (y - ray.y);
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
        // This magical piece of code normalizes the vector,
        // because I'm too lazy to calculate it by hand.
        float len = std::sqrt(ray.dx*ray.dx+ray.dy*ray.dy);
        ray.dx /= len; ray.dy /= len;

        glColor3ub(0, 0, 250);
        DrawLine(ray.x, ray.y, ray.dx, ray.dy, 100.f);
        tree.raycast(ray);
    }
 
    glFlush();
}
