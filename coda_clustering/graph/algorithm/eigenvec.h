#pragma once

#include <cmath>
#include <algorithm>

#include "base.h"
#include "../directed_graph.h"

namespace graph
{
	template <class Graph, class EdgeConv = void>
	class principal_eigenvec : parallel_algo<Graph>
	{
	};

	template <class Graph>
	class principal_eigenvec<Graph, void> : parallel_algo<Graph>
	{
	public:
		principal_eigenvec(Graph &g, size_t thread_num) : parallel_algo(g, thread_num)
		{
			auto n = g.node_num();
			_vals1 = new double[n];
			_vals2 = new double[n];
			std::fill(_vals1, _vals1 + n, 1.0);
			_eigenval = 0.0;
		}

		~principal_eigenvec()
		{
			delete[] _vals1;
			delete[] _vals2;
		}

		double *eigenvec()
		{
			return _vals1;
		}

		double eigenval()
		{
			return _eigenval;
		}

		void update_node(typename Graph::node_t node)
		{
			double sum = 0.0;
			auto nbrs = _graph.in_neighbors(node);
			for (auto u : nbrs)
			{
				sum += _vals1[u];
			}
			_vals2[node] = sum;
		}

		double iterate()
		{
			update();
			auto n = _graph.node_num();
			_eigenval = *std::max_element(_vals2, _vals2 + n);
			double diff = 0.0;
			if (_eigenval != 0.0)
			{
				for (Graph::node_t i = 0; i < n; ++i)
				{
					_vals2[i] /= _eigenval;
					diff = std::max(diff, fabs(_vals1[i] - _vals2[i]));
				}
			}
			std::swap(_vals1, _vals2);
			return diff;
		}

		void converge(double eps)
		{
			for (int loop = 0; ; ++loop)
			{
				double diff = iterate();
				printf("Loop %d: diff = %f\n", loop, diff);
				if (diff < eps) break;
			}
		}

	private:
		double *_vals1, *_vals2;
		double _eigenval;
	};

}