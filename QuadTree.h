#pragma once

#include <array>
#include <memory>
#include <boost/optional.hpp>
#include <stack>
#include <algorithm>

//DEBUG only
#include <Windows.h>
#include <GL/GL.h>
#include "UtilDraw.hpp"

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
        Extent narrow(Quadrant toQ) const {
            // if we are choosing right(->) quadrant, we have to move left border
            Extent e (*this);
            if (toQ & RIGHT_BIT)
                e.left = left + halfWidth();
            else
                e.right = right - halfWidth();

            if (toQ & BOT_BIT)
                e.top = top + halfHeight();
            else
                e.bottom = bottom - halfHeight();
            return e;
        }
        inline float width() const { return std::abs(right - left); }
        inline float height() const { return std::abs(bottom - top); }
        // these are quite useful
        inline float halfWidth() const { return width() * .5f; }
        inline float halfHeight() const { return height() * .5f; }
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
    bool isPointInCurrentExtents(float x, float y) const {
        // Support only for non-negative coords
        if (x < 0.f || y < 0.f) return false;
        return (x <= sizeInUnits && y <= sizeInUnits);
    }
    static Quadrant calculateQuadrant(
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
        if (val == 0)
            return;
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
            extent = extent.narrow(q);
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
            extent = extent.narrow(q);
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

    static Direction calculateExitDirection (Ray ray, Extent extent) {
        bool xadv = (ray.dx > 0.f);
        bool yadv = (ray.dy > 0.f);

        float horiz_len = xadv ? (extent.right - ray.x) : (extent.left - ray.x);
        float vert_len = yadv ? (extent.bottom - ray.y) : (extent.top - ray.y);

        float horiz_t = horiz_len / ray.dx;
        float vert_t = vert_len / ray.dy;

        if (horiz_t < vert_t) //exiting in either left or right
            return xadv ? Direction::RIGHT : Direction::LEFT;
        else
            return yadv ? Direction::BOTTOM : Direction::TOP;
    };

    static boost::optional<Direction> calculateImpactDirection(Ray ray, Extent extent) {
        bool xadv = (ray.dx > 0.f);
        bool yadv = (ray.dy > 0.f);

        float horiz_len = xadv ? (extent.left - ray.x) : (extent.right - ray.x);
        float vert_len = yadv ? (extent.top - ray.y) : (extent.bottom - ray.y);

        // we have to determine which plane will be crossed first
        float horiz_t = horiz_len / ray.dx;
        float vert_t = vert_len / ray.dy;

        if ((horiz_t < 0) && (vert_t < 0)) {
            return boost::none;
        }
        else {
            if (horiz_t < vert_t) //exiting in either left or right
                return yadv ? Direction::TOP : Direction::BOTTOM;
            else
                return xadv ? Direction::LEFT : Direction::RIGHT;
        }
    }

    enum class Axis { VERTICAL, HORIZONTAL };
    static float calculateLineCrosspoint(float linePos, Axis a, Ray r) {
        if (a == Axis::VERTICAL) {
            return (linePos - r.x) / r.dx * r.dy + r.y;
        }
        else {
            return (linePos - r.y) / r.dy * r.dx + r.x;
        }
    }

    static Quadrant calculateImpactSubvoxel(Extent e, Ray r) {
        auto impactTest = calculateImpactDirection(r, e);
        if (!impactTest)
            throw std::logic_error("Can't determine impact  subvoxel because the ray is not hitting the voxel");

        // the subtracted value is the difference between
        // the raycast's position and the voxel beginning position
        // that's because we have to "convert" that value to voxel-based
        // number to compare it with the half of the voxel.

        switch (impactTest.get()){
            case Direction::TOP: {
                float hitPoint = calculateLineCrosspoint(e.top, Axis::HORIZONTAL, r);
                return (hitPoint > (e.left + e.halfWidth())) ? TOP_RIGHT : TOP_LEFT;
            }
            case Direction::BOTTOM: {
                float hitPoint = calculateLineCrosspoint(e.bottom, Axis::HORIZONTAL, r);
                return (hitPoint > (e.left + e.halfWidth())) ? BOT_RIGHT : BOT_LEFT;
            }
            case Direction::LEFT: {
                float hitPoint = calculateLineCrosspoint(e.left, Axis::VERTICAL, r);
                return (hitPoint > (e.top + e.halfHeight())) ? BOT_LEFT : TOP_LEFT;
            }
            case Direction::RIGHT: {
                float hitPoint = calculateLineCrosspoint(e.right, Axis::VERTICAL, r);
                return (hitPoint > (e.top + e.halfHeight())) ? BOT_RIGHT : TOP_RIGHT;
            }
        }
    }


    // TODO: handling misses? (not really necessary)
    static Point calculateImpactPoint(Ray ray, Extent e) {
        auto impactTest = calculateImpactDirection(ray, e);
        if (!impactTest)
            throw std::logic_error("Can't determine impact  subvoxel because the ray is not hitting the voxel");
        switch (impactTest.get()){
        case Direction::TOP: {
            float hitPoint = calculateLineCrosspoint(e.top, Axis::HORIZONTAL, ray);
            return Point { hitPoint, e.top };
            }
        case Direction::BOTTOM: {
            float hitPoint = calculateLineCrosspoint(e.bottom, Axis::HORIZONTAL, ray);
            return Point { hitPoint, e.bottom };
            }
        case Direction::LEFT: {
            float hitPoint = calculateLineCrosspoint(e.left, Axis::VERTICAL, ray);
            return Point { e.left, hitPoint };
            }
        case Direction::RIGHT: {
            float hitPoint = calculateLineCrosspoint(e.right, Axis::VERTICAL, ray);
            return Point { e.right, hitPoint };
            }
        }
    }


    // DEPRECATED
    // this apparently wasn't used ever
    /*Direction calculateImpactDirFromQuadrantProgression(Quadrant a, Quadrant b) {
        int x_bit_a = a & RIGHT_BIT;
        int x_bit_b = b & RIGHT_BIT;
        int y_bit_a = a & BOT_BIT;
        int y_bit_b = b & BOT_BIT;

        int x_bit_diff = x_bit_b - x_bit_a;
        int y_bit_diff = y_bit_b - y_bit_a;

        // if both are 0 or both are changing, it would imply diagonal
        // movement or no movement, which is forbidden
        if (x_bit_diff && y_bit_diff)
            throw std::logic_error("Diagonal voxel movement is forbidden");
        if ((!x_bit_diff) && (!y_bit_diff))
            throw std::logic_error("Can't calculate progress because quadrants are equal");

        if (x_bit_diff)
            return (x_bit_diff == 1) ? Direction::RIGHT : Direction::LEFT;
        else
            return (y_bit_diff == 1) ? Direction::BOTTOM : Direction::TOP;
    }*/

    // should the stack be popped?
    static bool isExitingParent(Quadrant q, Direction d) {
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
    static Quadrant nextNeighbourQuadrant(Quadrant q, Direction d) {
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

    struct RaycastResult {
        SquareNode const* node;
        Extent extent;
        Point impactPoint;
    };
    RaycastResult raycast(Ray ray) const {
        struct StackElem {
            SquareNode const* node;
            Extent extent;
            Quadrant q;
        };

        const Extent rootExtent { 0, 0, sizeInUnits, sizeInUnits };

        std::stack<StackElem> stack;
        // determine the wall of impact on root
        // if there's no impact on root, return
        // else push root to the stack

        // there is only one voxel path that has the beginning of 
        bool insideCurrentVoxelPath = isPointInCurrentExtents(ray.x, ray.y);

        if (insideCurrentVoxelPath) {
            // we have to localize it inside; that simply means preparing the proper stack beforehand
            Extent extent = rootExtent;
            SquareNode const* current = &root;
            // root node "is" an arbitrary quadrant
            Quadrant q = TOP_LEFT;
            while (true) {
                stack.push(StackElem { current, extent, q });

                q = calculateQuadrant(ray.x, ray.y, extent);
                extent = extent.narrow(q);

                if (!(current->nodes[q])) {
                    break;
                }
                current = current->nodes[q];
            }
            
            // starting a ray inside filled voxel could automatically
            // be terminated by using this
            /*if (current->leaf != 0) {

            }*/
        }
        // the ray is outside
        else {
            // check if it hits 
            auto impactTest = calculateImpactDirection(ray, rootExtent);
            if (!impactTest) {
                return RaycastResult { nullptr };
            }
            // The ray is hitting the root
            stack.push(StackElem{ &root, rootExtent, Quadrant::TOP_LEFT });
        }

        while (!stack.empty()) {
            // #### PUSH ####
            if (stack.top().node) {
                // if the voxel we are at is not filled, try narrowing the impact voxel
                do {
                    // check if we perhaps already hit something
                    // if the voxel we are in is filled, we're done
                    // TEMP: criteria of passing
                    if (stack.top().node->leaf != 0){
                        //DEBUG
                        glColor3ub(0, 0, 200);
                        DrawSquare(stack.top().extent.left, stack.top().extent.top, stack.top().extent.height(), true);
                        return RaycastResult { stack.top().node, stack.top().extent, calculateImpactPoint(ray, stack.top().extent) };
                    }

                    Quadrant q;
                    if (insideCurrentVoxelPath) {
                        // if the ray is inside, simply use basic calculateQuadrant
                        q = calculateQuadrant(ray.x, ray.y, stack.top().extent);
                    }
                    else {
                        // if it's not, check impact voxel from outside
                        q = calculateImpactSubvoxel(stack.top().extent, ray);
                    }

                    stack.push(StackElem {
                        stack.top().node->nodes[q],
                        stack.top().extent.narrow(q),
                        q
                    });

                } while (stack.top().node);
                // TODO: Check if the level isn't too big considering ray length
            } 

            // #### ADVANCE ####
            // if the ray exits the voxel from some side, 
            // it will also exit the subvoxel from the same side.
            // in other words, this information can be used to
            // assess the in-voxel traversion.

            // it doesn't matter if the voxel is actually empty or not at this point

            while (true) {
                Direction d = calculateExitDirection(ray, stack.top().extent);
                if (isExitingParent(stack.top().q, d)) {
                    stack.pop();
                    if (stack.empty()) {
                        return RaycastResult { nullptr };
                    }
                }
                else {
                    Quadrant nextQuadrant = nextNeighbourQuadrant(stack.top().q, d);

                    // pop the last node to get to its parent
                    stack.pop();

                    if (stack.empty()) {
                        // root has no siblings
                        return RaycastResult { nullptr };
                    }

                    // stack top refers to the parent now
                    Extent nextExtent = stack.top().extent.narrow(nextQuadrant);
                    SquareNodePtr nextNode = stack.top().node->nodes[nextQuadrant];

                    insideCurrentVoxelPath = false;
                    stack.push(StackElem{ nextNode, nextExtent, nextQuadrant });
                    break;
                }
            }
        }
        return RaycastResult { nullptr };
    }
};

