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
	void AddOrientedEdge(int v1, int v2);
	int GetLoopCountBreakLoops(const std::set<int>& occupiedPlaces);
	// IMPORTANT: To be used only after the loops are broken.
	int LimitLayerFlow(int limit);
private:
	bool IsCyclic(const std::set<int>& occupiedPlaces, int v, std::vector<bool>& visited, std::vector<bool>& recStack);
	std::vector<OrientedGraphNode> graph_;
	bool loopsBroken_ = false;
};
