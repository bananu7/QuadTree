#include "QuadTree.h"
#include "UtilDraw.hpp"

void QuadTree::DrawRecursive(QuadTree::SquareNodePtr node, int level, float x, float y)
{
    if (node == nullptr)
        return;

    float sz = sizeInUnits / std::pow(2.f, level);

    if (node->leaf != 0) {
        glColor3ub(0, 200, 0);
        DrawSquare(x, y, sz, true);

        glColor3ub(255, 200, 255);
        //DrawSquare(x, y, sz, false);

        return;
    }

    glColor3ub(255, 200, 255);
    //DrawSquare(x, y, sz, false);

    DrawRecursive(node->nodes[0], level + 1, x, y);
    DrawRecursive(node->nodes[1], level + 1, x + sz/2.f, y);
    DrawRecursive(node->nodes[2], level + 1, x, y + sz/2.f);
    DrawRecursive(node->nodes[3], level + 1, x + sz/2.f, y + sz/2.f);
}

void QuadTree::DebugDraw()
{
    DrawRecursive(&root, 0, 0, 0);
}