#ifndef GRAPH_HPP
#  define GRAPH_HPP

#include <vector>
#include <iostream>
#include <cassert>

template<typename T>
class Node
{
public:

    using Neighbors = std::vector<int>;

public:

    Node() = default;

    Node(T&& data_)
        : data(std::move(data_))
    {}

    Node(T& data_)
        : data(std::move(data_))
    {}

    inline void addNeighbor(int node)
    {
        m_neighbors.push_back(node);
    }

    inline bool removeNeighbor(int node)
    {
        size_t i = m_neighbors.size();
        while (i--)
        {
            std::swap(m_neighbors[i],
                      m_neighbors[m_neighbors.size() - 1]);
            m_neighbors.pop_back();
            return true;
        }

        return false;
    }

    inline const Neighbors& neighbors() const
    {
        return m_neighbors;
    }

    inline Neighbors& neighbors()
    {
        return m_neighbors;
    }

public:

    T data;

private:

    Neighbors m_neighbors;
};


class Edge
{
public:

    int src, dest;
};

template <typename T>
class Graph
{
public:

    Graph(std::vector<Edge> const& edges, std::vector<T> const& data, bool undirected = false)
    {
        m_nodes.resize(data.size());

        size_t i = m_nodes.size();
        while (i--)
        {
            m_nodes[i].data = data[i];
        }

        for (auto const& edge: edges)
        {
            m_nodes[edge.src].addNeighbor(edge.dest);
            if (undirected)
            {
                m_nodes[edge.dest].addNeighbor(edge.src);
            }
        }
    }

    inline std::vector<Node<T>> const& nodes() const
    {
        return m_nodes;
    }

    inline Node<T>& node(size_t i)
    {
        assert(i < m_nodes.size());
        return m_nodes[i];
    }

    inline Node<T> const& node(size_t i) const
    {
        assert(i < m_nodes.size());
        return m_nodes[i];
    }

    void print()
    {
        for (size_t j = 0; j < m_nodes.size(); ++j)
        {
            std::cout << "Node " << j << " (value: " << m_nodes[j].data << ") --> ";
            for (auto idx: m_nodes[j].neighbors())
            {
                std::cout << idx << "  ";
            }
            std::cout << std::endl;
        }
    }

private:

    std::vector<Node<T>> m_nodes;
};

#endif
