#pragma once
#include <vector>
#include <set>
#include <map>

struct OrientedGraphNode
{
	std::set<int> next;
	std::set<int> back;
};

class OrientedGraph
{
public:
	OrientedGraph(int maxVertexCount);
	int GetLoopCount(const std::set<int>& occupiedPlaces);
	void AddOrientedEdge(int a, int b);
private:
	bool IsCyclicUtil(const std::set<int>& occupiedPlaces, int v, std::vector<bool>& visited, std::vector<bool>& recStack);
	std::vector<OrientedGraphNode> graph_;
};
