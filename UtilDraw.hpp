#pragma once
#include <Windows.h>
#include <GL/GL.h>

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
