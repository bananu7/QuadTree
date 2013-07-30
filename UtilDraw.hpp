#pragma once
#include <Windows.h>
#include <GL/GL.h>

inline float to_radians(float degrees) {
    return degrees / 180.f * 3.14159265359f;
}

inline
void DrawSquare(float x, float y, float size, bool solid)
{
    glBegin((solid) ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size);
    glVertex2f(x, y + size);
    glEnd();
}

inline
void DrawLine(float x, float y, float dx, float dy, float length) {
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glColor4f(.5f, .5f, .5f, 1.f);
    glVertex2f(x + dx*length, y + dy*length);
    glEnd();
}

inline
void DrawLine(float x, float y, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x, y);
    //glColor4f(.5f, .5f, .5f, 1.f);
    glVertex2f(x2, y2);
    glEnd();
}

inline
void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}
