#pragma once

#include <array>
#include <memory>
#include <boost/optional.hpp>
#include <stack>
#include <algorithm>

typedef unsigned TData;

class QuadTree
{
// temp
public:
    enum Quadrant : unsigned char {
        TOP_LEFT = 0,
        TOP_RIGHT = 1,
        BOT_LEFT = 2,
        BOT_RIGHT = 3,
        RIGHT_BIT = 1,
        BOT_BIT = 2
    };
    enum class Direction { TOP, BOTTOM, LEFT, RIGHT };
    struct Extent {
        float left, top, right, bottom;
        void narrow(Quadrant toQ) {
            auto halfWidth = std::abs(right - left) * .5f;
            auto halfHeight = std::abs(bottom - top) * .5f;

            // if we are choosing right(->) quadrant, we have to move left border
            if (toQ & RIGHT_BIT)
                left += halfWidth;
            else
                right -= halfWidth;

            if (toQ & BOT_BIT)
                top += halfHeight;
            else
                bottom -= halfHeight;
        }
    };

    struct Ray {
        float x, y, dx, dy;
    };

    struct SquareNode
    {
        /*
            layout:
            0 1
            2 3
        */
        std::array<SquareNode*, 4> nodes;
        TData leaf;

        SquareNode() { 
            nodes[0] = nullptr;
            nodes[1] = nullptr;
            nodes[2] = nullptr;
            nodes[3] = nullptr;
            leaf = 0;
        }
    };
    typedef SquareNode* SquareNodePtr;

    struct FindResult {
        SquareNodePtr node;
        Extent extent;
        unsigned level;
    };

    float sizeInUnits;
    unsigned maxLOD;

    // utility functions
    SquareNode root;

protected:
    // do we need to extend the tree to save it?
    bool isPointInCurrentExtents(float x, float y) {
        // Support only for non-negative coords
        if (x < 0.f || y < 0.f) return false;
        return (x <= sizeInUnits && y <= sizeInUnits);
    }
    Quadrant calculateQuadrant(
        float x, float y,
        Extent const& ex
        )
    {
        float xmid = (ex.right - ex.left) / 2 + ex.left;
        float ymid = (ex.bottom - ex.top) / 2 + ex.top;
        unsigned char xbit = (x >= xmid) ? RIGHT_BIT : 0;
        unsigned char ybit = (y >= ymid) ? BOT_BIT : 0;

        return Quadrant(xbit + ybit);
    }

    void DrawRecursive(QuadTree::SquareNodePtr node, int level, float x, float y);

public:
    QuadTree(float sizeInUnits, unsigned maxLOD) 
        : sizeInUnits(sizeInUnits)
        , maxLOD(maxLOD)
    { }

    void DebugDraw();

    void recursiveDestroy(SquareNodePtr node) {
        if (!node)
            return;
        for (auto& n : node->nodes) {
            recursiveDestroy(n);
            if (n) {
                delete n;
                n = nullptr;
            }
        }
    }

    void recursiveMerge(SquareNodePtr current) {
        if (!current)
            return;

        for (auto n : current->nodes) {
            recursiveMerge(n);
        }

        if (!(current->nodes[0]))
            return;

        TData val = current->nodes[0]->leaf;
        // if not all of them are equal, merge basically fails
        for (unsigned i = 1; i < 4; ++i) {
            if (!(current->nodes[i]))
                return;
            if (current->nodes[i]->leaf != val)
                return;
        }

        recursiveDestroy(current);
        current->leaf = val;
    }

    void set(float x, float y, TData val) {
        if (!isPointInCurrentExtents(x, y)) {
            // resize the tree so that it fits
            // just throws for now
            throw std::logic_error("Point is outside tree area");
        }

        SquareNodePtr current = &root;
        Extent extent { 0.f, 0.f, sizeInUnits, sizeInUnits };

        for (unsigned level = 0; level <= maxLOD; ++level) {
            Quadrant q = calculateQuadrant(x, y, extent);

            // does that quadrant exist?
            if (!(current->nodes[q])) {
                // create it if not
                current->nodes[q] = new SquareNode();
            }

            current = current->nodes[q];
            // move the "iterator"
            // and narrow the extents of the current square
            extent.narrow(q);
        }

        current->leaf = val;
        recursiveMerge(&root);
    }

    FindResult _find(float x, float y, bool returnLastNonZeroTraversed) {
        SquareNodePtr current = &root;
        Extent extent { 0.f, 0.f, sizeInUnits, sizeInUnits };

        unsigned level = 0;
        while (level <= maxLOD) {
            Quadrant q = calculateQuadrant(x, y, extent);

            if (!(current->nodes[q])) {
                // returning nullptr is useful for functions that only want value
                // or 0 if the node is empty
                if (returnLastNonZeroTraversed)
                    return FindResult { current, extent, level };
                else
                    return FindResult { nullptr, extent, level };
            }

            current = current->nodes[q];
            extent.narrow(q);
            ++level;
        }
        return FindResult { current, extent, level };
    }

    boost::optional<TData> get(float x, float y) {
        SquareNodePtr current = _find(x, y, false).node;
        if (current)
            return current->leaf;
        else
            return boost::none;
    }


    struct Point { float x, y; };

    /*auto calculateExitPoint = [](
    float x, float y, float dx, float dy,
    Extent extent) -> Point {

    // sanity check if x,y is actually inside Extent
    };*/
    //Point p = calculateExitPoint(x, y, dx, dy, fr.extent);


    Direction calculateExitDirection (Ray ray, Extent extent) {
        bool xadv = (ray.dx > 0.f);
        bool yadv = (ray.dy > 0.f);

        float horiz_len = xadv ? (extent.right - ray.x) : (ray.x - extent.left);
        float vert_len = yadv ? (extent.bottom - ray.y) : (ray.y - extent.top);

        float horiz_t = horiz_len / ray.dx;
        float vert_t = horiz_len / ray.dy;

        if (horiz_t < vert_t) //exiting in either left or right
            return xadv ? Direction::BOTTOM : Direction::TOP;
        else
            return yadv ? Direction::RIGHT : Direction::LEFT;
    };

    // should the stack be popped?
    bool isExitingParent(Quadrant q, Direction d) {
        switch (d) {
        case Direction::TOP:
            return (q == TOP_LEFT) || (q == TOP_RIGHT);
        case Direction::BOTTOM:
            return (q == BOT_LEFT) || (q == BOT_RIGHT);
        case Direction::RIGHT:
            return (q == TOP_RIGHT) || (q == BOT_RIGHT);
        case Direction::LEFT:
            return (q == TOP_LEFT) || (q == BOT_LEFT);
        }
    }
    // if we don't exit parent, what quadrant are we switching to?
    Quadrant nextNeighbourQuadrant(Quadrant q, Direction d) {
        switch (d) {
        case Direction::TOP:
            return (q == BOT_LEFT) ? TOP_LEFT : TOP_RIGHT;
        case Direction::BOTTOM:
            return (q == TOP_LEFT) ? BOT_LEFT : BOT_RIGHT;
        case Direction::LEFT:
            return (q == BOT_RIGHT) ? BOT_LEFT : TOP_LEFT;
        case Direction::RIGHT:
            return (q == BOT_LEFT) ? BOT_RIGHT : TOP_RIGHT;
        }
    }

    SquareNodePtr raycast(Ray ray) {
        if (!isPointInCurrentExtents(ray.x, ray.y)) {
            throw std::logic_error("Point is outside tree area");
        }
        // first we have to go as deeply as we can looking for the raycast place

        struct StackElem {
            SquareNodePtr node;
            Extent extent;
            unsigned level;
            Quadrant q;
        };

        std::stack<StackElem> stack;
        {
            // prepare stack before further operations
            StackElem fr { &root, Extent{ 0.f, 0.f, sizeInUnits, sizeInUnits }, 0 };
            while (fr.level <= maxLOD) {
                fr.q = calculateQuadrant(ray.x, ray.y, fr.extent);
                stack.push(fr);

                // no children
                if (!(fr.node->nodes[fr.q]))
                    break;

                fr.node = fr.node->nodes[fr.q];
                fr.extent.narrow(fr.q);
                ++fr.level;
            }
        }

        while (!stack.empty()) {
            // check if we perhaps already hit something
            auto& voxel = stack.top();
            // TEMP: criteria of passing
            // #### INTERSECT ####
            if (voxel.node->leaf != 0){
                return voxel.node;
            }
            //else {
            // #### PUSH ####

            //stack.push(stack.top().node->nodes[stack.top().q]);

            // #### ADVANCE ####
            Direction d = calculateExitDirection(ray, voxel.extent);
            if (isExitingParent(voxel.q, d)) {
                stack.pop();
            }
            else {
                stack.top().q = nextNeighbourQuadrant(stack.top().q, d);
            }
        }
    }
};

