#pragma once
#include "AStarInterface.hpp"
#include <memory>

struct Node
{
	Node() = default;
	Node(Node* other)
	{
		actionsToReach = std::move(other->actionsToReach);
		state = std::move(other->state);
		depth = other->depth;
		pathCost = other->pathCost;
		heuristicCost = other->heuristicCost;
	}
	Node(Node const* other)
	{
		actionsToReach.resize(other->actionsToReach.size());
		for (int i = 0; i < other->actionsToReach.size(); ++i)
		{
			actionsToReach[i] = std::unique_ptr<IAction>(other->actionsToReach[i]->Clone());
		}
		state = std::unique_ptr<IState>(other->state->Clone());
		depth = other->depth;
		pathCost = other->pathCost;
		heuristicCost = other->heuristicCost;
	}
	// The actions that were taken to reach this node.
	std::vector<std::unique_ptr<IAction>> actionsToReach;
	// The state the search is at in this node.
	std::unique_ptr<IState> state;
	// The depth of the node in the tree.
	int depth;
	// The cost of the path from the initial state.
	int pathCost;
	// The cost of the path from the initial state to the nearest goal state (using heuristics computation).
	int heuristicCost = -1;
};