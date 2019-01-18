#pragma once

#include "graph/graph.h"
#include "graph/algorithm/base.h"

class affi_directed_model : public graph::parallel_algo<graph::directed_graph<int, char *>>
{
public:
	typedef graph::directed_graph<int, char *> graph_t;

	affi_directed_model(graph_t &g, int cluster_num, size_t thread_num);
	~affi_directed_model();

	int node_num();
	int cluster_num();

	void init_neighborhood(bool *is_seed);
	void init_min_neighborhood(bool *is_seed = nullptr);
	void init_random(unsigned seed);
	void update_node(int node);

	float iterate_out(float alpha = 1.0, float scale = 1e-4, float decay = 0.5, bool *is_train = nullptr);
	float iterate_in(float alpha = 1.0, float scale = 1e-4, float decay = 0.5, bool *is_train = nullptr);

	float argmin_out(float alpha = 1.0, float scale = 1e-4, float decay = 0.5, bool *is_train = nullptr, float rel_improve = 1e-4);
	float argmin_in(float alpha = 1.0, float scale = 1e-4, float decay = 0.5, bool *is_train = nullptr, float rel_improve = 1e-4);

	float converge(float alpha = 1.0, float scale = 1e-4, float decay = 0.5, bool *is_train = nullptr, float rel_improve = 1e-4);

	float likelihood();
	float likelihood(int node);
	float likelihood(bool *used);

	float **affinity_out();
	float **affinity_in();

private:
	int _cluster_num;
	float _background_prob;
	float **_affi_out, *_affi_out_data;
	float **_affi_out_tmp, *_affi_out_tmp_data;
	float **_affi_out_d, *_affi_out_d_data;
	float **_affi_in, *_affi_in_data;
	float **_affi_in_tmp, *_affi_in_tmp_data;
	float **_affi_in_d, *_affi_in_d_data;
	float *_affi_sum_out, *_affi_sum_in;

	float *_likelihood_buf, *_likelihood_buf_tmp;
	float _likelihood, _likelihood_tmp;
	float *_conductance_tmp;

	enum _parallel_task_type
	{
		task_gradient_out, task_gradient_in, task_min_neighborhood, task_likelihood, task_edge_prob, task_make_affi_sum
	};
	_parallel_task_type _task_type;

	void _update_node_gradient_out(int node);
	void _update_node_gradient_in(int node);
	void _update_node_min_neighborhood(int node);
	void _update_node_likelihood(int node);
	void _update_node_make_affi_sum(int c);

	void _make_affi_sum();
	void _make_gradient_out();
	void _make_gradient_in();
};

