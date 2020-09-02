#include "OrientedGraph.hpp"
#include <stack>
#include <cassert>

OrientedGraph::OrientedGraph(int maxVertexCount)
{
	graph_.resize(maxVertexCount);
}

void OrientedGraph::AddOrientedEdge(int v1, int v2)
{
    graph_[v1].next.insert(v2);
    graph_[v2].back.insert(v1);
}

int OrientedGraph::GetLoopCountBreakLoops(const std::set<int>& occupiedPlaces)
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

    loopsBroken_ = true;
    return result;
}

int OrientedGraph::LimitLayerFlow(int limit)
{
    assert(loopsBroken_);
    std::set<int> ignoreSet;
    std::vector<std::set<int>> layers;
    std::map<int, int> layerMembership;

    for (int i = 0; i < graph_.size(); ++i)
    {
        if (graph_[i].next.empty() && graph_[i].back.empty())
        {
            ignoreSet.insert(i);
        }
    }

    while (ignoreSet.size() < graph_.size())
    {
        std::set<int> layer;

        for (int i = 0; i < graph_.size(); ++i)
        {
            if (ignoreSet.find(i) == ignoreSet.end())
            {
                bool shouldBeInLayer = true;
                for (int backNode : graph_[i].back)
                {
                    if (ignoreSet.find(backNode) == ignoreSet.end())
                    {
                        shouldBeInLayer = false;
                        break;
                    }
                }
                if (shouldBeInLayer)
                {
                    layerMembership[i] = layers.size();
                    layer.insert(i);
                }
            }
        }

        for (int layerElement : layer)
        {
            ignoreSet.insert(layerElement);
        }
        layers.push_back(layer);
    }

    
    for (int layer = 0; layer < layers.size(); ++layer)
    {
        std::set<int> eraseList;
        for (auto it = layers[layer].begin(); it != layers[layer].end(); ++it)
        {
            bool shouldBeMoved = true;
            for (auto itNext = graph_[*it].next.begin(); itNext != graph_[*it].next.end(); ++itNext)
            {
                if (layerMembership[*itNext] == layer + 1)
                {
                    shouldBeMoved = false;
                    break;
                }
            }
            if (shouldBeMoved && !graph_[*it].next.empty())
            {
                ++layerMembership[*it];
                layers[layer + 1].insert(*it);
                eraseList.insert(*it);
            }
        }
        for (auto it = eraseList.begin(); it != eraseList.end(); ++it)
        {
            layers[layer].erase(*it);
        }
    }

    int result = 0;

    int flow = 0;
    for (const auto& layer : layers)
    {
        for (const auto& element : layer)
        {
            flow += graph_[element].next.size();
            flow -= graph_[element].back.size();
        }
        result += flow / (limit + 1);
    }

    return result;
}

bool OrientedGraph::IsCyclic(const std::set<int>& occupiedPlaces, int v, std::vector<bool>& visited, std::vector<bool>& recStack)
{
    if (visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack 
        visited[v] = true;
        recStack[v] = true;

        std::set<int> eraseSet;
        // Recur for all the vertices adjacent to this vertex 
        for (auto i = graph_[v].next.begin(); i != graph_[v].next.end(); ++i)
        {
            if (!visited[*i] && IsCyclic(occupiedPlaces, *i, visited, recStack))
            {
                for (auto&& eraseElement : eraseSet)
                {
                    graph_[v].next.erase(eraseElement);
                    graph_[eraseElement].back.erase(v);
                }
                return true;
            }
            else if (recStack[*i] && occupiedPlaces.find(v) == occupiedPlaces.end())
            {
                int element = *i;
                graph_[v].next.erase(element);
                graph_[element].back.erase(v);
                for (auto&& eraseElement : eraseSet)
                {
                    graph_[v].next.erase(eraseElement);
                    graph_[eraseElement].back.erase(v);
                }
                return true;
            }
            else if (recStack[*i])
            {
                eraseSet.insert(*i);
            }
        }

        for (auto&& eraseElement : eraseSet)
        {
            graph_[v].next.erase(eraseElement);
            graph_[eraseElement].back.erase(v);
        }
    }
    recStack[v] = false;  // remove the vertex from recursion stack
    return false;
}
