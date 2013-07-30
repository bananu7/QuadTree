#include <OpenGLWindow.hpp>
#include <GL/GL.h>
#include <string>
#include <iostream>
#include <array>
#include <vector>

#include "test.h"
#include "QuadTree.h"
#include "Renderer.h"
//#include "high_perf_windows_timer.hpp"

using std::cout;
using std::endl;

long long run_time;
Renderer r;
bool cast = false;

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
    glEnable(GL_STENCIL_TEST);
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

    w.mousedownCallback = [&](oglw::MouseInfo mi) {
        if (mi.button == oglw::MouseInfo::Button::Left)
            mousePressed = true; 
    };
    w.mouseupCallback = [&](oglw::MouseInfo mi) {
        mousePressed = false; 

        if (mi.button == oglw::MouseInfo::Button::Right)
            cast = !cast;

        if (mi.button == oglw::MouseInfo::Button::Middle) {
            // move the ray to the cursor
            r.ray.x = mi.normX * 100.f;
            r.ray.y = mi.normY * 100.f;
        }
    };
    w.mousemoveCallback = [&](oglw::MouseInfo mi) {
        float x = mi.normX * 100.f;
        float y = mi.normY * 100.f;
        if (mousePressed) {
            r.tree.set(x, y, Renderer::Voxel(5));
        }

        r.ray.dx = (x - r.ray.x);
        r.ray.dy = (y - r.ray.y);
    };

    w.displayFunc = display;

    //test();
    /*srand(42);
    for (int i = 0; i < 400; ++i) {
        float rx = (float)(rand()) / RAND_MAX * 60.f;
        float ry = (float) (rand()) / RAND_MAX * 60.f;
        tree.set(rx, ry, 5);
    }*/

    while (w.display(), w.process()) { }
}

void display() {
    glClearColor(0.5f, .5f, .5f, 1.f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //static qpc_clock clock;

    r.render();
    //auto time_a = clock.now();
    //auto time_b = clock.now();
    //run_time = std::chrono::duration_cast<std::chrono::microseconds>(time_b - time_a).count();

    glFlush();
}
