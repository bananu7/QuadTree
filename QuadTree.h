#pragma once

#include <array>
#include <memory>
#include <boost/optional.hpp>

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
        }
    };
    typedef SquareNode* SquareNodePtr;

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
    }

    boost::optional<TData> get(float x, float y) {
        SquareNodePtr current = &root;
        Extent extent { 0.f, 0.f, sizeInUnits, sizeInUnits };

        unsigned level = 0;
        while (level <= maxLOD) {
            Quadrant q = calculateQuadrant(x, y, extent);

            if (!(current->nodes[q])) {
                return boost::none;
            }

            current = current->nodes[q];
            extent.narrow(q);
            ++level;
        }

        return current->leaf;
    }
};

