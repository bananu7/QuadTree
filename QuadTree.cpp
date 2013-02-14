#include "QuadTree.h"
#include <exception>
using std::exception;

namespace 
{
	enum EQuadrant : unsigned char
	{
		TOP_LEFT = 0,
		TOP_RIGHT,
		BOT_LEFT,
		BOT_RIGHT,
		RIGHT_BIT = 1,
		BOT_BIT = 2
	};

	EQuadrant calculate_quadrant(
		unsigned x, unsigned y,
		unsigned up_bound_x,
		unsigned low_bound_x,
		unsigned up_bound_y,
		unsigned low_bound_y
		)
	{
		unsigned xmid = (up_bound_x - low_bound_x) / 2 + low_bound_x;
		unsigned ymid = (up_bound_y - low_bound_y) / 2 + low_bound_y;
		unsigned char xbit = (x >= xmid) ? 1 : 0;
		unsigned char ybit = (y >= ymid) ? 2 : 0;
		
		return EQuadrant(xbit+ybit);
	}
}

void QuadTree::Add (TData element, unsigned x, unsigned y)
{
	SquareNodePtr act;
	unsigned level = startLevel;
	unsigned upper_x_bound = baseSize,
				upper_y_bound = baseSize,
				lower_x_bound = 0,
				lower_y_bound = 0;

	auto check_bounds = [&]() -> bool {
		if (x >= upper_x_bound)
			return false;
		if (y >= upper_y_bound)
			return false;
		if (x < lower_x_bound)
			return false;
		if (y < lower_y_bound)
			return false;
	};

	if (!check_bounds())
		throw exception("Adding beyond root boundaries");

	act = &root;

	while (level-- > 0)
	{
		EQuadrant next_q = calculate_quadrant(
			x, y,
			upper_x_bound, lower_x_bound,
			upper_y_bound, lower_y_bound
		);

		if (next_q & RIGHT_BIT)
			lower_x_bound = (upper_x_bound - lower_x_bound) / 2;
		else
			upper_x_bound = (upper_x_bound - lower_x_bound) / 2 + lower_y_bound;

		if (next_q & BOT_BIT)
			lower_y_bound = (upper_y_bound - lower_y_bound) / 2;
		else
			upper_y_bound = (upper_y_bound - lower_y_bound) / 2 + lower_y_bound;

		if (! (act->nodes[next_q])) {
			act->nodes[next_q] = new SquareNode;
		}
		act = act->nodes[next_q];
	}

	act->leaf = element;
}

boost::optional<TData &> QuadTree::Get (unsigned x, unsigned y)
{
	return boost::none;
}

boost::optional<TData const&> QuadTree::Get (unsigned x, unsigned y) const
{
	return boost::none;
}

boost::optional<TData const&> QuadTree::Raycast (float x, float y, float dx, float dy)
{
	return boost::none;
}

void QuadTree::RecursiveMap(SquareNodePtr node, std::function<void(SquareNodePtr)> const& fun)
{
	fun(node);

	for (unsigned i = 0; i < 4; ++i)
	{
		if (node->nodes[i])
			fun(node->nodes[i]);
	}
}

/*

#include <fstream>
using std::ofstream;

void QuadTree::DebugDraw()
{
	ofstream file ("out.txt");
	file << "DEBUG OUT\n";
	
	RecursiveMap (&root, [&file](SquareNodePtr node) {
		file << node->leaf << " ";
	});
}*/