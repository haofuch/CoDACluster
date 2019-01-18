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
		class DampContainer = void,
		class TransitInContainer = void>
	class pagerank : public parallel_algo<Graph>
	{
	public:
		pagerank(Graph &g, const BiasContainer &bias, const DampContainer &damp, TransitInContainer &transit, size_t thread_num) 
			: parallel_algo(g, thread_num), _bias(bias), _damp(damp), _transit(transit)
		{
			Graph::node_t n = g.node_num();
			_vals1 = new double[n];
			_vals2 = new double[n];
			for (Graph::node_t i = 0; i < n; ++i)
			{
				_vals1[i] = _bias[i];
			}
		}

		~pagerank()
		{
			delete[] _vals1;
			delete[] _vals2;
		}

		void update_node(typename Graph::node_t node)
		{
			auto in_nbrs = _graph.in_neighbors(node);
			double sum = _restart_sum * _bias[node];
			long long offset = _graph.in_edges().degree_sum(node);
			for (auto u : in_nbrs)
			{
				sum += _vals1[u] * _damp[u] * _transit[offset++];
			}
			_vals2[node] = sum;
		}

		double iterate()
		{
			_restart_sum = 0.0;
			Graph::node_t n = _graph.node_num();
			for (Graph::node_t i = 0; i < n; ++i)
			{
				if (_graph.out_degree(i) == 0)
				{
					_restart_sum += _vals1[i];
				}
				else
				{
					_restart_sum += _vals1[i] * (1.0 - _damp[i]);
				}
			}

			update();

			double diff = 0.0;
			for (Graph::node_t i = 0; i < n; ++i)
			{
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
		typename const DampContainer &_damp;
		typename const TransitInContainer &_transit;
		double *_vals1, *_vals2;
		double _restart_sum;
	};

	template <class Graph>
	class pagerank<Graph, void, void, void> : public parallel_algo<Graph>
	{
	public:
		pagerank(Graph &g, double damp, size_t thread_num) : parallel_algo(g, thread_num)
		{
			_damp = damp;
			Graph::node_t n = g.node_num();
			_vals1 = new double[n];
			_vals2 = new double[n];
			std::fill(_vals1, _vals1 + n, 1.0 / n);
		}

		~pagerank()
		{
			delete[] _vals1;
			delete[] _vals2;
		}

		void update_node(typename Graph::node_t node)
		{
			auto nbrs = _graph.in_neighbors(node);
			double sum = _sink_sum;
			for (auto u : nbrs)
			{
				sum += _vals1[u] / _graph.out_degree(u);
			}
			_vals2[node] = (1.0 - _damp) / _graph.node_num() + _damp * sum;
		}

		double iterate()
		{
			Graph::node_t n = _graph.node_num();

			_sink_sum = 0.0;
			for (Graph::node_t i = 0; i < n; ++i)
			{
				if (_graph.out_degree(i) == 0) _sink_sum += _vals1[i];
			}
			_sink_sum /= n;

			update();

			double diff = 0.0;
			for (Graph::node_t i = 0; i < n; ++i)
			{
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
			for (int loop = 0; ; ++loop)
			{
				double diff = iterate();
				printf("Loop %d: diff = %f\n", loop, diff);
				if (diff < eps) break;
			}
		}

		double *values()
		{
			return _vals1;
		}

	protected:
		double _damp;
		double _sink_sum;
		double *_vals1, *_vals2;

	};
}