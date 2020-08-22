#pragma once
#include <vector>
#include <queue>
#include <set>
#include <map>

struct OrientedGraphNode
{
	std::queue<int> next;
	std::set<int> back;
};

class OrientedGraph
{
public:
	OrientedGraph(int maxVertexCount);
	int GetLoopCount();
	void AddOrientedEdge(int a, int b);
	int DriveThrough(int from);
	int FindRideStart() const;
	std::set<int> EnumerateLooseEnds();
private:
	int FindRideStartImpl(int current, std::map<int, int>& visited, bool& ideal, int& ending) const;
	std::vector<OrientedGraphNode> graph_;
};
