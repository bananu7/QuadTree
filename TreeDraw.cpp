
#include "QuadTree.h"
#include <GL/glew.h>

void DrawSquare (int x, int y, int size, bool solid)
{
	glBegin((solid) ? GL_QUADS : GL_LINE_LOOP);
		glVertex2i(x, y);
		glVertex2i(x + size, y);
		glVertex2i(x + size, y + size);
		glVertex2i(x, y + size);
	glEnd();
}

void DrawRecursive(QuadTree::SquareNodePtr node, int level, int x, int y)
{
	if (node == nullptr)
		return;

	if (level == 0) {
		glColor3ub(0, 200, 0);
		DrawSquare(x, y, pow(2, level), true);
	}
	else {
		glColor3ub(255, 200, 255);
		DrawSquare(x, y, pow(2, level), false);
			
			unsigned adv = pow(2, level-1);

			DrawRecursive (node->nodes[0], level - 1, x, y);
			DrawRecursive (node->nodes[1], level - 1, x+adv, y);
			DrawRecursive (node->nodes[2], level - 1, x, y+adv);
			DrawRecursive (node->nodes[3], level - 1, x+adv, y+adv);
	}
}


void QuadTree::DebugDraw()
{
	DrawRecursive(&root, startLevel, 0, 0);
}