#include "OrientedGraph.hpp"

OrientedGraph::OrientedGraph(int maxVertexCount)
{
	graph_.resize(maxVertexCount);
}

void OrientedGraph::AddOrientedEdge(int a, int b)
{
	graph_[a].next.push(b);
	graph_[b].back.insert(a);
}

int OrientedGraph::DriveThrough(int from)
{
	int current = from;
	int result = 0;
	while (!graph_[current].next.empty())
	{
		int next = graph_[current].next.front();
		graph_[current].next.pop();
		graph_[next].back.erase(current);
		current = next;
		++result;
	}
	return result;
}

int OrientedGraph::FindRideStart() const
{
	int current = -1;
	for (int i = 0; i < graph_.size(); ++i)
	{
		if (!graph_[i].back.empty() || !graph_[i].next.empty())
		{
			current = i;
			break;
		}
	}

	if (current != -1)
	{
		std::map<int, int> visited;
		return FindRideStartImpl(current, visited);
	}
	else
	{
		return current;
	}
}

int OrientedGraph::FindRideStartImpl(int current, std::map<int, int>& visited) const
{
	while (!graph_[current].back.empty())
	{
		int offset = visited[current]++;
		auto it = graph_[current].back.begin();
		if (graph_[current].back.size() > offset)
		{
			std::advance(it, offset);
			current = *it;
		}
		else
		{
			break;
		}
	}
	return current;
}
