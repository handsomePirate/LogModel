#include "AStarSolver.hpp"
#include <iostream>
#include <queue>
#include <string>

struct CompareNodes
{
	bool operator()(const std::shared_ptr<Node>& n1, std::shared_ptr<Node> n2)
	{
		return n1->heuristicCost > n2->heuristicCost ||
			(n1->heuristicCost == n2->heuristicCost && n1->depth < n2->depth);
	}
};

int AStarSolver::Solve(const IProblem& problem, std::vector<std::unique_ptr<IAction>>& solution, int maxIterations)
{
	std::priority_queue<std::shared_ptr<Node>> fringe;

	if (maxIterations == INT32_MAX)
		std::cout << "===========PRECISE SEARCH===========" << std::endl << std::endl;
	else
		std::cout << "===========LIMITED SEARCH===========" << std::endl << "--iteration limit: " << maxIterations << std::endl << std::endl;

	std::shared_ptr<IState> initialState = problem.GetInitialState();

	int deepeningStop = initialState->Heuristic();
	int deepeningIteration = 0;
	std::shared_ptr<Node> currentBestPathNode;

	int lastDepth = 0;
	// Iterative deepening.
	while (deepeningIteration < maxIterations)
	{
		// Start with the initial state.
		std::shared_ptr<Node> initialNode = std::make_shared<Node>();
		initialNode->depth = 0;
		initialNode->pathCost = 0;
		fringe.push(initialNode);
		int nextDeepeningStop = INT32_MAX;

		currentBestPathNode = initialNode;
		initialNode->heuristicCost = initialState->Heuristic();

		// While there are nodes to consider.
		while (!fringe.empty())
		{
			// For each step, expand the best node.
			std::shared_ptr<Node> bestNode = fringe.top();
			fringe.pop();

			lastDepth = bestNode->depth;

			// Test for goal state.
			int bestActionLength = (int)bestNode->actionsToReach.size();
			const std::shared_ptr<IState>& state = bestActionLength > 0 ?
				bestNode->actionsToReach[bestActionLength - 1]->state :
				problem.GetInitialState();
			if (problem.IsGoalState(state))
			{
				std::cout << "Found the solution at iteration number " + std::to_string(deepeningIteration++) + "." << std::endl;
				solution.reserve(bestNode->actionsToReach.size());
				for (const std::unique_ptr<IAction>& action : bestNode->actionsToReach)
				{
					solution.emplace_back(action->Clone());
				}
				return bestNode->pathCost;
			}

			if (bestNode->depth != 0)
			{
				if (currentBestPathNode->depth == 0 || bestNode->actionsToReach[bestNode->actionsToReach.size() - 1]->state->Heuristic() <=
					currentBestPathNode->actionsToReach[currentBestPathNode->actionsToReach.size() - 1]->state->Heuristic())
				{
					currentBestPathNode = bestNode;
				}
			}

			// Enumerate all the states that are reachable (by an action) from the best node state to the fringe.
			std::unordered_set<std::unique_ptr<IAction>, IActionHash> actions;

			problem.EnumeratePossibleActions(state, actions);

			for (const std::unique_ptr<IAction>& action : actions)
			{
				int heuristicCost = bestNode->pathCost + action->cost + action->state->Heuristic();
				if (heuristicCost > deepeningStop)
				{
					if (nextDeepeningStop > heuristicCost)
					{
						nextDeepeningStop = heuristicCost;
					}
				}
				else
				{
					std::shared_ptr<Node> insertedNode = MakeNode(bestNode, action);

					fringe.push(insertedNode);
				}
			}
		}
		deepeningStop = nextDeepeningStop;
		std::cout << "Done with iteration number " + std::to_string(deepeningIteration++) + "." << std::endl;
	}

	solution.reserve(currentBestPathNode->actionsToReach.size());
	for (const std::unique_ptr<IAction>& action : currentBestPathNode->actionsToReach)
	{
		solution.emplace_back(action->Clone());
	}
	return INT32_MAX;
}

int AStarSolver::NodeHeuristicCost(const std::shared_ptr<Node>& node, const std::shared_ptr<IState>& initialState)
{
	int actionLength = (int)node->actionsToReach.size();
	return node->pathCost + (actionLength ? node->actionsToReach[actionLength - 1]->state->Heuristic() :
		initialState->Heuristic());
}

std::shared_ptr<Node> AStarSolver::MakeNode(const std::shared_ptr<Node>& originalNode, const std::unique_ptr<IAction>& action)
{
	std::shared_ptr<Node> newNode = std::make_shared<Node>();
	newNode->pathCost = originalNode->pathCost + action->cost;
	newNode->depth = originalNode->depth + 1;

	newNode->actionsToReach.reserve(originalNode->actionsToReach.size() + 1);
	for (const std::unique_ptr<IAction>& action : originalNode->actionsToReach)
	{
		newNode->actionsToReach.emplace_back(action->Clone());
	}
	newNode->actionsToReach.emplace_back(action->Clone());

	return newNode;
}
