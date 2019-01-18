#pragma once

#include "../directed_graph.h"

#include <algorithm>

namespace graph
{
	template <class Node, class PredIt>
	class _directed_modularity_comparer
	{
	public:
		_directed_modularity_comparer(PredIt pred_first) : _preds(pred_first) { }

		bool operator()(Node a, Node b)
		{
			return _preds[a] < _preds[b];
		}

	private:
		PredIt _preds;
	};

	template <class Graph, class PredIt>
	double directed_modularity(Graph &g, PredIt pred_first)
	{
		Graph::node_t n = g.node_num();
		Graph::node_t *nodes = new Graph::node_t[n];
		for (Graph::node_t i = 0; i < n; ++i)
		{
			nodes[i] = i;
		}
		std::sort(nodes, nodes + n, _directed_modularity_comparer<Graph::node_t, PredIt>(pred_first));
		
		double result = 0.0;
		for (Graph::node_t i = 0; i < n; )
		{
			Graph::node_t j = i;
			while (j < n && pred_first[nodes[j]] == pred_first[nodes[i]]) ++j;
			long long sum_edge = 0;
			long long sum_out = 0, sum_in = 0;
			for ( ; i < j; ++i)
			{
				Graph::node_t u = nodes[i];
				sum_out += g.out_degree(u);
				sum_in += g.in_degree(u);
				auto nbrs = g.out_neighbors(u);
				for (Graph::node_t v : nbrs)
				{
					if (pred_first[u] == pred_first[v]) ++sum_edge;
				}
			}
			double part = (double)sum_edge / g.edge_num() - ((double)sum_out / g.edge_num()) * ((double)sum_in / g.edge_num());
			result += part;
			//printf("%f\n", part);
		}

		delete[] nodes;

		return result;
	}

	template <class Graph, class PredIt>
	double undirected_modularity(Graph &g, PredIt pred_first)
	{
		Graph::node_t n = g.node_num();
		Graph::node_t *nodes = new Graph::node_t[n];
		for (Graph::node_t i = 0; i < n; ++i)
		{
			nodes[i] = i;
		}
		std::sort(nodes, nodes + n, _directed_modularity_comparer<Graph::node_t, PredIt>(pred_first));

		long long edge_num = 0;
		for (Graph::node_t i = 0; i < n; ++i) edge_num += g.bi_degree(i);

		double result = 0.0;
		for (Graph::node_t i = 0; i < n;)
		{
			Graph::node_t j = i;
			while (j < n && pred_first[nodes[j]] == pred_first[nodes[i]]) ++j;
			long long sum_edge = 0;
			long long sum_out = 0, sum_in = 0;
			for (; i < j; ++i)
			{
				Graph::node_t u = nodes[i];
				sum_out += g.bi_degree(u);
				sum_in += g.bi_degree(u);
				auto nbrs = g.bi_neighbors(u);
				for (Graph::node_t v : nbrs)
				{
					if (pred_first[u] == pred_first[v]) ++sum_edge;
				}
			}
			double part = (double)sum_edge / edge_num - ((double)sum_out / edge_num) * ((double)sum_in / edge_num);
			result += part;
			//printf("%f\n", part);
		}

		delete[] nodes;

		return result;
	}

	template <class Graph, class InIt>
	double directed_cluster_coefficient(Graph &g, InIt nbr_first, InIt nbr_last)
	{
		size_t degree = nbr_last - nbr_first;
		if (degree <= 1) return 0.0;

		Graph::node_t *nbrs = new Graph::node_t[degree];
		std::copy(nbr_first, nbr_last, nbrs);
		double result = _directed_cluster_coefficient(g, nbrs, nbrs + degree);
		delete[] nbrs;
		return result;
	}

	template <class Graph, class RandIt>
	double _directed_cluster_coefficient(Graph &g, RandIt nbr_first, RandIt nbr_last)
	{
		size_t degree = 0, num = 0;
		std::sort(nbr_first, nbr_last);
		for (RandIt curr = nbr_first; curr != nbr_last; ++curr)
		{
			++degree;
			auto out_nbrs = g.out_neighbors(*curr);
			for (auto v : out_nbrs)
			{
				if (std::binary_search(nbr_first, nbr_last, v)) ++num;
			}
		}

		if (degree <= 1) return 0.0;

		return (double)num / ((double)degree * (degree - 1));
	}
};