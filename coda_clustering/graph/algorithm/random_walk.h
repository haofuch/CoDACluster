#pragma once

#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "../directed_graph.h"
#include "base.h"

namespace graph
{
	template <
		class Graph,
		class BiasContainer = void,
		class TransitInContainer = void>
	class random_walk : parallel_algo<Graph>
	{
	public:
		random_walk(Graph &g, const BiasContainer &bias, TransitInContainer &transit, size_t thread_num)
			: parallel_algo(g, thread_num), _bias(bias), _transit(transit)
		{
			Graph::node_t n = g.node_num();
			_vals1 = new double[n];
			_vals2 = new double[n];
			for (Graph::node_t i = 0; i < n; ++i)
			{
				_vals1[i] = _bias[i];
			}
			_out_sum = new double[n];
			std::fill(_out_sum, _out_sum + n, 0.0);
			long long offset = 0;
			for (Graph::node_t v = 0; v < n; ++v)
			{
				for (Graph::node_t u : g.in_neighbors(v))
				{
					_out_sum[u] += transit[offset++];
				}
			}
		}

		~random_walk()
		{
			delete[] _vals1;
			delete[] _vals2;
			delete[] _out_sum;
		}

		void update_node(typename Graph::node_t node)
		{
			auto in_nbrs = _graph.in_neighbors(node);
			double sum = 0.0;
			long long offset = _graph.in_edges().degree_sum(node);
			for (auto u : in_nbrs)
			{
				sum += _vals1[u] * _transit[offset++];
			}
			_vals2[node] = sum;
		}

		double iterate()
		{
			update();
			double restart = 0.0;
			Graph::node_t n = _graph.node_num();
			for (Graph::node_t i = 0; i < n; ++i)
			{
				restart += _vals1[i] * _out_sum[i];
			}
			restart = 1.0 - restart;
			double diff = 0.0;
			for (Graph::node_t i = 0; i < n; ++i)
			{
				_vals2[i] += restart * _bias[i];
				if (_vals1[i] != 0.0)
				{
					double d = fabs(_vals1[i] - _vals2[i]) / _vals1[i];
					if (diff < d) diff = d;
				}
			}
			std::swap(_vals1, _vals2);
			return diff;
		}

		void converge(double eps)
		{
			for (int loop = 0;; ++loop)
			{
				double diff = iterate();
				printf("Loop %d: diff = %f\n", loop, diff);
				double sum = 0.0;
				for (Graph::node_t i = 0; i < _graph.node_num(); ++i) sum += _vals1[i];
				printf("sum = %f\n", sum);
				if (diff < eps) break;
			}
		}

		double *values()
		{
			return _vals1;
		}

	protected:
		typename const BiasContainer &_bias;
		typename const TransitInContainer &_transit;
		double *_out_sum;
		double *_vals1, *_vals2;

	};
}