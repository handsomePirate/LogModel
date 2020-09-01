#pragma once
#include "AStarNode.hpp"
#include "AStarInterface.hpp"
#include <vector>
#include <unordered_set>

// This object is able to solve any search problem, as long as it is implemented following the 
// IProblem interface.
class AStarSolver 
{
public:
	// Solves the problem and returns the sequence of actions to take from the initial state 
	// to achieve the optimal solution.
	// Returns the cost of the action chain.
	// If maxIterations is less than INT32_MAX, it might happen that the solution does not get you to a goal state,
	// but only to the best state found in the allowed iterations.
	int Solve(const IProblem& problem, std::vector<std::unique_ptr<IAction>>& solution, int maxIterations = INT32_MAX);
private:
	static Node* MakeNode(Node const* originalNode, IAction* action, IState* state, int heuristicCost);
};
