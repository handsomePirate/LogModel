#pragma once
#include <memory>
#include <queue>

// A class to be used as an interface to the state of the search.
class IState
{
public:
	IState() {}
	// Virtual desctructor.
	virtual ~IState() {};

	int Heuristic() const { return heuristic; }
	virtual IState* Clone() const { return nullptr; };

protected:
	// The heuristic value of this state (how close is it to the solution).
	int heuristic;
};

// A structure that contains the cost of the action and the state that it takes the search into.
class IAction
{
public:
	friend struct IActionHash;
	int cost;

	virtual ~IAction() {};

	bool operator==(const IAction& other) const { return false; }

	virtual IAction* Clone() const = 0;

	IAction()
		: hash_((size_t)this) {}
private:
	size_t hash_;
};

struct IActionHash
{
	size_t operator()(const std::unique_ptr<IAction>& action) const
	{
		return action->hash_;
	}
};

class IProblem
{
public:
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
