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

QuadTree tree(100.f, 4);
QuadTree::Ray ray { -5.f, -5.f, -8.f, 6.f };

void keyboard(unsigned char key, int x, int y);
void display(void);

void CheckForError() {
    GLint E = glGetError();
    if (E != 0)
        _CrtDbgBreak();
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


void DrawLine(float x, float y, float dx, float dy, float length) {
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glColor4f(.5f, .5f, .5f, 1.f);
    glVertex2f(x + dx*length, y + dy*length);
    glEnd();
}

void DrawLine(float x, float y, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x2, y2);
    glEnd();
}

void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
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
        float len = std::sqrt(ray.dx*ray.dx + ray.dy*ray.dy);
        ray.dx /= len; ray.dy /= len;

        glColor3ub(0, 0, 250);
        glLineWidth(4.f);
        DrawLine(ray.x, ray.y, ray.dx, ray.dy, 100.f);

        float arc = std::atan2(ray.dx, ray.dy);
        for (int i = -100; i < 100; ++i) {
            auto tempr = ray;
            float newarc = arc + (i / 180.f * 3.14f) * 0.1f;
            tempr.dx = cosf(newarc);
            tempr.dy = sinf(newarc);

            auto result = tree.raycast(tempr);
            glColor3ub(80, 80, 80);
            DrawLine(ray.x, ray.y, result.impactPoint.x, result.impactPoint.y);
            //DrawTriangle(tempr.x, tempr.y, result.extent.left, result.extent.top, result.extent.right, result.extent.bottom);
        }
    }

    glFlush();
}
