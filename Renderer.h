#pragma once
#include "QuadTree.h"
#include "UtilDraw.hpp"

class Renderer
{
public:
    struct Voxel {
        unsigned val;
        bool isOpaque() const {
            return val != 0;
        }
        bool operator != (Voxel const& other) {
            return val != other.val;
        }
        Voxel() : val(0) { }
        Voxel(unsigned val) : val(val) { }
    };

    Ray ray;
    QuadTree<Voxel> tree;

    Renderer()
        : ray(Ray{ 50.f, 50.f, -5.f, -5.f })
        , tree { 100.f, 4 }
    {

    }

    void render() {
        // This magical piece of code normalizes the vector,
        // because I'm too lazy to calculate it by hand.
        float len = std::sqrt(ray.dx*ray.dx + ray.dy*ray.dy);
        ray.dx /= len; ray.dy /= len;

        float arc = -std::atan2(ray.dx, ray.dy) + 3.146f / 2.f;



        float cone_size_degrees = 50;
        int sample_count = 100;

        glEnable(GL_STENCIL_TEST);

        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        //#pragma omp parallel for num_threads(8)
        for (int i = -sample_count / 2; i < sample_count / 2; ++i) {
            Ray tempr = ray;

            float newarc = arc + to_radians((float) i / sample_count * cone_size_degrees);
            //float newarc = arc;
            tempr.dx = cosf(newarc);
            tempr.dy = sinf(newarc);

            //if (cast)
            auto result = tree.raycast(tempr);
            glColor3ub(80, 80, 80);

            // if we hit something, draw a proper ray
            glLineWidth(3.f);
            if (result.node)
                DrawLine(ray.x, ray.y, result.impactPoint.x, result.impactPoint.y);
            //DrawLine(ray.x, ray.y, tempr.dx, tempr.dy, 50);
            //DrawTriangle(tempr.x, tempr.y, result.extent.left, result.extent.top, result.extent.right, result.extent.bottom);
        }

        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        tree.DebugDraw();

        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glColor3ub(255, 255, 0);
        for (int x = 0; x < 10; ++x)
            for (int y = 0; y < 10; ++y) {

                glColor3ub(255, 255, 255);
                if ((x + y) % 2)
                    glColor3ub(0, 0, 0);

                DrawSquare(x * 10, y * 10, 10, true);
            }

        glColor3ub(0, 0, 250);
        glLineWidth(4.f);
        DrawLine(ray.x, ray.y, ray.dx, ray.dy, 100.f);
    }
};

