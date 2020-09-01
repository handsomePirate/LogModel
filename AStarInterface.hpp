#pragma once
#include <memory>
#include <queue>

// A class to be used as an interface to the state of the search.
class IState
{
public:
	// Default constructor.
	IState() {}
	// Virtual desctructor.
	virtual ~IState() {};

	// Returns the heuristic property.
	int Heuristic() const { return heuristic; }
	// Allocates and initializes a clone of this state.
	virtual IState* Clone() const = 0;

protected:
	// The heuristic value of this state (how close is it to the solution).
	int heuristic = -1;
};

// A structure that contains the cost of the action.
class IAction
{
public:
	// The cost of this action.
	int cost = 0;

	// Default constructor.
	IAction() {};
	// Virtual destructor.
	virtual ~IAction() {};

	// Allocates and initializes a clone of this action.
	virtual IAction* Clone() const = 0;
};

// A structure that contains the definition of the problem that needs state search solving.
class IProblem
{
public:
	// Default constructor.
	IProblem() {}
	// Virtual destructor.
	virtual ~IProblem() {}

	// Should return the state that we want to start our search from.
	virtual IState const* GetInitialState() const = 0;
	// Should return true, if the state parameter is the goal state.
	virtual bool IsGoalState(IState const* state) const = 0;
	// Should enumerate all actions that can be taken from the input state.
	virtual void EnumeratePossibleActions(IState const* state,
		std::queue<std::pair<IAction*, IState*>>& possibleActions) const = 0;
};
