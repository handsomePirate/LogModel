#include "OrientedGraph.hpp"
#include <stack>

OrientedGraph::OrientedGraph(int maxVertexCount)
{
	graph_.resize(maxVertexCount);
}

int OrientedGraph::GetLoopCount(std::set<int> occupiedPlaces)
{
	int result = 0;
	std::map<int, int> toBeVisited;
	for (int i = 0; i < graph_.size(); ++i)
	{
		if (graph_[i].next.size() > 0)
		{
			toBeVisited[i] = graph_[i].next.size();
		}
	}
	while (!toBeVisited.empty())
	{
		std::stack<int> dfsStack;
		int first = -1;
		bool truckRide = false;
		if (occupiedPlaces.empty() || toBeVisited.find(*occupiedPlaces.begin()) == toBeVisited.end())
		{
			first = toBeVisited.begin()->first;
		}
		else
		{
			auto it = occupiedPlaces.begin();
			first = *it;
			occupiedPlaces.erase(it);
			truckRide = true;
		}
		auto it = toBeVisited.find(first);
		if (it->second == 1)
		{
			toBeVisited.erase(it);
		}
		else
		{
			--it->second;
		}
		dfsStack.push(first);
		std::set<int> currentlyVisited;
		while (!dfsStack.empty())
		{
			int node = dfsStack.top();
			dfsStack.pop();
			
			currentlyVisited.insert(node);

			while (!graph_[node].next.empty())
			{
				int nextNode = graph_[node].next.front();
				graph_[node].next.pop();
				if (currentlyVisited.find(nextNode) != currentlyVisited.end())
				{
					if (first != nextNode || !truckRide)
					{
						++result;
					}
					continue;
				}
				dfsStack.push(nextNode);
				auto it = toBeVisited.find(nextNode);
				if (it != toBeVisited.end())
				{
					if (it->second == 1)
					{
						toBeVisited.erase(it);
					}
					else
					{
						--it->second;
					}
				}
			}
		}
	}
	return result;
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
		bool ideal;
		int ending = -1;
		int result = FindRideStartImpl(current, visited, ideal, ending);
		return (result == current && !ideal) ? ending : result;
	}
	else
	{
		return current;
	}
}

std::set<int> OrientedGraph::EnumerateLooseEnds()
{
	std::set<int> result;
	for (int i = 0; i < graph_.size(); ++i)
	{
		if (graph_[i].back.empty() && !graph_[i].next.empty())
		{
			result.insert(i);
		}
	}
	return result;
}

int OrientedGraph::FindRideStartImpl(int current, std::map<int, int>& visited, bool& ideal, int& ending) const
{
	int offset = visited[current]++;
	if (graph_[current].back.size() > offset)
	{
		do
		{
			auto it = graph_[current].back.begin();
			std::advance(it, offset);
			std::map<int, int> visitedCopy = visited;
			int result = FindRideStartImpl(*it, visitedCopy, ideal, ending);
			if (ideal)
			{
				return result;
			}
			else
			{
				offset = visited[current]++;
			}
		} while (!ideal && graph_[current].back.size() > offset);
	}
	if (ending == -1)
	{
		ending = current;
	}
	if (graph_[current].back.size() == 0)
	{
		ideal = true;
	}
	else
	{
		ideal = false;
	}
	return current;
}
