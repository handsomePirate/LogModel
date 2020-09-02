#include "OrientedGraph.hpp"
#include <stack>

OrientedGraph::OrientedGraph(int maxVertexCount)
{
	graph_.resize(maxVertexCount);
}

int OrientedGraph::GetLoopCount(const std::set<int>& occupiedPlaces)
{
    // Mark all the vertices as not visited and not part of recursion 
    // stack 
    std::vector<bool> visited;
    visited.resize(graph_.size());
    std::vector<bool> recStack;
    recStack.resize(graph_.size());

    // Call the recursive helper function to detect cycle in different 
    // DFS trees 
    int result = 0;
    int last = 0;
    do
    {
        for (int i = 0; i < graph_.size(); ++i)
        {
            visited[i] = false;
            recStack[i] = false;
        }
        last = result;
        for (int i = 0; i < graph_.size(); ++i)
        {
            if (IsCyclic(occupiedPlaces, i, visited, recStack))
            {
                ++result;
                break;
            }
        }
    } while (last != result);

    return result;
}

void OrientedGraph::AddOrientedEdge(int v1, int v2)
{
	graph_[v1].next.insert(v2);
}

bool OrientedGraph::IsCyclic(const std::set<int>& occupiedPlaces, int v, std::vector<bool>& visited, std::vector<bool>& recStack)
{
    if (visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack 
        visited[v] = true;
        recStack[v] = true;

        // Recur for all the vertices adjacent to this vertex 
        for (auto i = graph_[v].next.begin(); i != graph_[v].next.end(); ++i)
        {
            if (!visited[*i] && IsCyclic(occupiedPlaces, *i, visited, recStack))
            {
                return true;
            }
            else if (recStack[*i] && occupiedPlaces.find(v) == occupiedPlaces.end())
            {
                graph_[v].next.erase(*i);
                return true;
            }
        }

    }
    recStack[v] = false;  // remove the vertex from recursion stack 
    return false;
}
