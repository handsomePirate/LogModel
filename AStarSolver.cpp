#include "AStarSolver.hpp"
#include <iostream>
#include <queue>
#include <set>
#include <string>

struct CompareNodes
{
	bool operator()(const std::unique_ptr<Node>& n1, const std::unique_ptr<Node>& n2)	
	{
		return n1->heuristicCost > n2->heuristicCost ||
			(n1->heuristicCost == n2->heuristicCost && n1->depth < n2->depth);
	}
};

int AStarSolver::Solve(const IProblem& problem, std::vector<std::unique_ptr<IAction>>& solution, int maxIterations)
{
	std::priority_queue<std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, CompareNodes> fringe;

	if (maxIterations == INT32_MAX)
		std::cout << "===========PRECISE SEARCH===========" << std::endl << std::endl;
	else
		std::cout << "===========LIMITED SEARCH===========" << std::endl << "--iteration limit: " << maxIterations << std::endl << std::endl;

	IState const* initialState = problem.GetInitialState();

	int deepeningStop = initialState->Heuristic();
	int deepeningIteration = 0;
	std::unique_ptr<Node> currentBestPathNode = nullptr;

	// Iterative deepening.
	while (deepeningIteration < maxIterations)
	{
		// Start with the initial state.
		Node* initialNode = new Node;
		initialNode->depth = 0;
		initialNode->pathCost = 0;
		initialNode->state = std::unique_ptr<IState>(initialState->Clone());
		fringe.emplace(initialNode);
		int nextDeepeningStop = INT32_MAX;

		currentBestPathNode = std::unique_ptr<Node>(new Node((Node const*)initialNode));
		initialNode->heuristicCost = initialState->Heuristic();

		// While there are nodes to consider.
		while (!fringe.empty())
		{
			// For each step, expand the best node.
			std::unique_ptr<Node> bestNode = std::unique_ptr<Node>(new Node(fringe.top().get()));
			fringe.pop();

			// Test for goal state.
			int bestActionLength = (int)bestNode->actionsToReach.size();
			IState const* state = bestActionLength > 0 ? bestNode->state.get() : problem.GetInitialState();
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
				if (currentBestPathNode->depth == 0 || bestNode->state->Heuristic() <=
					currentBestPathNode->state->Heuristic())
				{
					currentBestPathNode = std::unique_ptr<Node>(new Node((Node const*)bestNode.get()));
				}
			}

			// Enumerate all the states that are reachable (by an action) from the best node state of the fringe.
			std::queue<std::pair<IAction*, IState*>> actions;

			problem.EnumeratePossibleActions(state, actions);

			while (!actions.empty())
			{
				auto actionPair = actions.front();
				actions.pop();
				int heuristicCost = bestNode->pathCost + actionPair.first->cost + actionPair.second->Heuristic();
				if (heuristicCost > deepeningStop)
				{
					if (nextDeepeningStop > heuristicCost)
					{
						nextDeepeningStop = heuristicCost;
					}
				}
				else
				{
					Node* insertedNode = MakeNode(bestNode.get(), actionPair.first, actionPair.second, heuristicCost);

					fringe.emplace(insertedNode);
				}
			}
		}
		deepeningStop = nextDeepeningStop;
		std::cout << "Done with iteration number " + std::to_string(deepeningIteration++) + "." << std::endl;
	}

	if (currentBestPathNode)
	{
		solution.reserve(currentBestPathNode->actionsToReach.size());
		for (const std::unique_ptr<IAction>& action : currentBestPathNode->actionsToReach)
		{
			solution.emplace_back(action->Clone());
		}
	}
	return INT32_MAX;
}

int AStarSolver::NodeHeuristicCost(Node const* node, const std::shared_ptr<IState>& initialState)
{
	int actionLength = (int)node->actionsToReach.size();
	return node->pathCost + (actionLength ? node->state->Heuristic() :
		initialState->Heuristic());
}

Node* AStarSolver::MakeNode(Node const* originalNode, IAction* action, IState* state, int heuristicCost)
{
	Node* newNode = new Node;
	newNode->pathCost = originalNode->pathCost + action->cost;
	newNode->depth = originalNode->depth + 1;
	newNode->state = std::unique_ptr<IState>(state);
	newNode->heuristicCost = heuristicCost;

	newNode->actionsToReach.reserve(originalNode->actionsToReach.size() + 1);
	for (const std::unique_ptr<IAction>& action : originalNode->actionsToReach)
	{
		newNode->actionsToReach.emplace_back(action->Clone());
	}
	newNode->actionsToReach.emplace_back(action);

	return newNode;
}
