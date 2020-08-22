#pragma once
#include "AStarInterface.hpp"
#include <memory>

struct Node
{
	// The actions that were taken to reach this node.
	std::vector<std::unique_ptr<IAction>> actionsToReach;
	// The depth of the node in the tree.
	int depth;
	// The cost of the path from the initial state.
	int pathCost;
	// The cost of the path from the initial state to the nearest goal state (using heuristics computation).
	int heuristicCost = -1;
};