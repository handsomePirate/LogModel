#pragma once
#include <vector>
#include <set>
#include <map>

struct OrientedGraphNode
{
	std::set<int> next;
};

class OrientedGraph
{
public:
	OrientedGraph(int maxVertexCount);
	int GetLoopCount(const std::set<int>& occupiedPlaces);
	void AddOrientedEdge(int v1, int v2);
private:
	bool IsCyclic(const std::set<int>& occupiedPlaces, int v, std::vector<bool>& visited, std::vector<bool>& recStack);
	std::vector<OrientedGraphNode> graph_;
};
