#pragma once

#include "QuadTree.h"

inline void test1() {
    QuadTree tree(100.f, 3);
}

inline void test2() {
    QuadTree tree(100.f, 3);
    tree.set(30.f, 30.f, 5);

    auto v = tree.get(30.f, 30.f);
    if ((!v) || (v.get() != 5))
        assert(false);
}

inline void test() {
    test1();
    test2();
}

