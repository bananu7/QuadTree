#pragma once

#include <array>
#include <memory>
#include <boost/optional.hpp>

typedef unsigned TData;

class QuadTree
{
// temp
public:
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
			for (auto & n : nodes)
				n = nullptr;
		}
	};

	typedef SquareNode* SquareNodePtr;

	static const unsigned baseSize = 8;
	static const unsigned startLevel = 3;
	SquareNode root;

	void RecursiveMap(SquareNodePtr node, std::function<void(SquareNodePtr)> const& fun);

public:
	void QuadTree::Add (TData element, unsigned x, unsigned y);
	boost::optional<TData &> Get (unsigned x, unsigned y);
	boost::optional<TData const&> Get (unsigned x, unsigned y) const;
	boost::optional<TData const&> Raycast (float x, float y, float dx, float dy);

	void DebugDraw();
};

