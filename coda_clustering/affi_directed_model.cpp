#include "affi_directed_model.h"

#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <random>

const static float min_p = 1e-6f;

affi_directed_model::affi_directed_model(graph_t &g, int cluster_num, size_t thread_num) : parallel_algo(g, thread_num)
{
	_cluster_num = cluster_num;
	int n = _graph.node_num();
	_background_prob = 1.0f / n;
	size_t size = (size_t)n * _cluster_num;
	_affi_out_data = new float[size];
	_affi_out_tmp_data = new float[size];
	_affi_out_d_data = new float[size];
	_affi_in_data = new float[size];
	_affi_in_tmp_data = new float[size];
	_affi_in_d_data = new float[size];

	_affi_out = new float*[n];
	_affi_out_tmp = new float*[n];
	_affi_out_d = new float*[n];
	_affi_in = new float*[n];
	_affi_in_tmp = new float*[n];
	_affi_in_d = new float*[n];
	
	for (int i = 0; i < n; ++i)
	{
		size_t offset = (size_t)i * _cluster_num;
		_affi_out[i] = _affi_out_data + offset;
		_affi_out_tmp[i] = _affi_out_tmp_data + offset;
		_affi_out_d[i] = _affi_out_d_data + offset;
		_affi_in[i] = _affi_in_data + offset;
		_affi_in_tmp[i] = _affi_in_tmp_data + offset;
		_affi_in_d[i] = _affi_in_d_data + offset;
	}

	_affi_sum_out = new float[_cluster_num];
	_affi_sum_in = new float[_cluster_num];

	_likelihood_buf = new float[n];
	_likelihood_buf_tmp = new float[n];
	_conductance_tmp = new float[n];

	_likelihood = 1.0;
}

affi_directed_model::~affi_directed_model()
{
	delete[] _affi_out_data;
	delete[] _affi_out_tmp_data;
	delete[] _affi_out_d_data;
	delete[] _affi_in_data;
	delete[] _affi_in_tmp_data;
	delete[] _affi_in_d_data;

	delete[] _affi_out;
	delete[] _affi_out_tmp;
	delete[] _affi_out_d;
	delete[] _affi_in;
	delete[] _affi_in_tmp;
	delete[] _affi_in_d;

	delete[] _affi_sum_out;
	delete[] _affi_sum_in;
	delete[] _likelihood_buf;
	delete[] _likelihood_buf_tmp;
	delete[] _conductance_tmp;
}

void affi_directed_model::update_node(int node)
{
	switch (_task_type)
	{
	case _parallel_task_type::task_gradient_out:
		_update_node_gradient_out(node);
		break;
	case _parallel_task_type::task_gradient_in:
		_update_node_gradient_in(node);
		break;
	case _parallel_task_type::task_min_neighborhood:
		_update_node_min_neighborhood(node);
		break;
	case _parallel_task_type::task_likelihood:
		_update_node_likelihood(node);
		break;
	case _parallel_task_type::task_make_affi_sum:
		_update_node_make_affi_sum(node);
		break;
	}
}

void affi_directed_model::_update_node_min_neighborhood(int node)
{
	long long cut = 0, degree_sum = _graph.out_degree(node) + _graph.in_degree(node);
	auto out_nbrs = _graph.out_neighbors(node);
	auto in_nbrs = _graph.in_neighbors(node);
	for (int i = 0; i < _graph.out_degree(node); ++i) degree_sum += _graph.out_degree(out_nbrs[i]) + _graph.in_degree(out_nbrs[i]);
	for (int i = _graph.bi_degree(node); i < _graph.in_degree(node); ++i) degree_sum += _graph.out_degree(in_nbrs[i]) + _graph.in_degree(in_nbrs[i]);

	int sample_rate = std::max(1, (int)(degree_sum / 100));
	int sample_size = 0;
	long long step_count = 0;

	for (int v : out_nbrs)
	{
		auto onbrs = _graph.out_neighbors(v);
		for (int i = (int)(step_count % sample_rate); i < _graph.out_degree(v); i += sample_rate)
		{
			int w = onbrs[i];
			if ((w != node) && !_graph.has_edge(node, w) && !_graph.has_edge(w, node)) ++cut;
			++sample_size;
		}
		step_count += _graph.out_degree(v);

		auto inbrs = _graph.in_neighbors(v);
		for (int i = (int)(step_count % sample_rate); i < _graph.in_degree(v); i += sample_rate)
		{
			int w = inbrs[i];
			if ((w != node) && !_graph.has_edge(node, w) && !_graph.has_edge(w, node)) ++cut;
			++sample_size;
		}
		step_count += _graph.in_degree(v);
	}

	for (int z = _graph.bi_degree(node); z < _graph.in_degree(node); ++z)
	{
		int v = in_nbrs[z];
		auto onbrs = _graph.out_neighbors(v);
		for (int i = (int)(step_count % sample_rate); i < _graph.out_degree(v); i += sample_rate)
		{
			int w = onbrs[i];
			if ((w != node) && !_graph.has_edge(node, w) && !_graph.has_edge(w, node)) ++cut;
			++sample_size;
		}
		step_count += _graph.out_degree(v);

		auto inbrs = _graph.in_neighbors(v);
		for (int i = (int)(step_count % sample_rate); i < _graph.in_degree(v); i += sample_rate)
		{
			int w = inbrs[i];
			if ((w != node) && !_graph.has_edge(node, w) && !_graph.has_edge(w, node)) ++cut;
			++sample_size;
		}
		step_count += _graph.in_degree(v);
	}

	cut = (cut == 0 || degree_sum == 0) ? 0 : cut * degree_sum / sample_size;
	long long size = std::min(degree_sum, _graph.edge_num());
	_conductance_tmp[node] = (size == 0) ? 1e38f : (float)cut / (float)size;
}

void affi_directed_model::init_neighborhood(bool *is_seed)
{
	int n = _graph.node_num(), m = 0;
	auto pairs = new std::pair<int, int>[n];
	for (int i = 0; i < n; ++i)
	{
		if (is_seed[i]) pairs[m++] = std::make_pair(_graph.out_degree(i) + _graph.in_degree(i) - _graph.bi_degree(i) + 1, i);
	}
	printf("%d seeds\n", m);
	std::sort(pairs, pairs + m);
	std::reverse(pairs, pairs + m);
	for (int i = 0; i < n; ++i)
	{
		std::fill(_affi_out[i], _affi_out[i] + _cluster_num, 0.0f);
		std::fill(_affi_in[i], _affi_in[i] + _cluster_num, 0.0f);
	}
	for (int c = 0; c < _cluster_num && c < m; ++c)
	{
		int node = pairs[c].second;
		int size = pairs[c].first;
		//printf("Node = %d, size = %d\n", node, size);

		auto out_nbrs = _graph.out_neighbors(node);
		auto in_nbrs = _graph.in_neighbors(node);
		int out_deg = _graph.out_degree(node);
		int in_deg = _graph.in_degree(node);
		int bi_deg = _graph.bi_degree(node);

		int *nodes = new int[in_deg + out_deg - bi_deg];
		std::copy(out_nbrs.begin(), out_nbrs.end(), nodes);
		std::copy(in_nbrs.begin() + bi_deg, in_nbrs.end(), nodes + out_deg);
		std::sort(nodes, nodes + in_deg + out_deg - bi_deg);

		for (int i = 0; i < bi_deg; ++i)
		{
			int v = out_nbrs[i];
			_affi_out[v][c] = 1.0;
			_affi_in[v][c] = 1.0;
		}

		for (int i = bi_deg; i < out_deg; ++i)
		{
			int v = out_nbrs[i];
			_affi_in[v][c] = 1.0;
			for (int w : _graph.out_neighbors(v))
			{
				if (std::binary_search(nodes, nodes + in_deg + out_deg - bi_deg, w))
				{
					_affi_out[v][c] = 1.0;
					break;
				}
			}
		}

		for (int i = bi_deg; i < in_deg; ++i)
		{
			int v = in_nbrs[i];
			_affi_out[v][c] = 1.0;
			for (int w : _graph.in_neighbors(v))
			{
				if (std::binary_search(nodes, nodes + in_deg + out_deg - bi_deg, w))
				{
					_affi_in[v][c] = 1.0;
					break;
				}
			}
		}

		delete[] nodes;
	}
	delete[] pairs;
	_make_affi_sum();
}

void affi_directed_model::init_min_neighborhood(bool *is_seed)
{
	_task_type = _parallel_task_type::task_min_neighborhood;

	update();

	int n = _graph.node_num(), m = 0;
	auto pairs = new std::pair<int, int>[n];
	for (int i = 0; i < n; ++i)
	{
		bool is_min = true;
		bool has_seed_neighbor = false;

		auto out_nbrs = _graph.out_neighbors(i);
		for (int j : out_nbrs)
		{
			if (is_seed == nullptr || is_seed[j]) has_seed_neighbor = true;
			if (_conductance_tmp[i] >= _conductance_tmp[j])
			{
				is_min = false;
				break;
			}
		}

		auto in_nbrs = _graph.in_neighbors(i);
		for (int j : in_nbrs)
		{
			if (is_seed == nullptr || is_seed[j]) has_seed_neighbor = true;
			if (_conductance_tmp[i] >= _conductance_tmp[j])
			{
				is_min = false;
				break;
			}
		}

		if (is_min && has_seed_neighbor) pairs[m++] = std::make_pair(_graph.out_degree(i) + _graph.in_degree(i) - _graph.bi_degree(i) + 1, i);
	}
	printf("%d seeds\n", m);
	std::sort(pairs, pairs + m);
	std::reverse(pairs, pairs + m);
	for (int i = 0; i < n; ++i)
	{
		std::fill(_affi_out[i], _affi_out[i] + _cluster_num, 0.0f);
		std::fill(_affi_in[i], _affi_in[i] + _cluster_num, 0.0f);
	}
	for (int c = 0; c < _cluster_num && c < m; ++c)
	{
		int node = pairs[c].second;
		int size = pairs[c].first;
		//printf("node = %d %s, size = %d\n", node, _graph.node_attr(node), size);

		auto out_nbrs = _graph.out_neighbors(node);
		auto in_nbrs = _graph.in_neighbors(node);
		int out_deg = _graph.out_degree(node);
		int in_deg = _graph.in_degree(node);
		int bi_deg = _graph.bi_degree(node);
		
		int *nodes = new int[in_deg + out_deg - bi_deg];
		std::copy(out_nbrs.begin(), out_nbrs.end(), nodes);
		std::copy(in_nbrs.begin() + bi_deg, in_nbrs.end(), nodes + out_deg);
		std::sort(nodes, nodes + in_deg + out_deg - bi_deg);

		for (int i = 0; i < bi_deg; ++i)
		{
			int v = out_nbrs[i];
			_affi_out[v][c] = 1.0;
			_affi_in[v][c] = 1.0;
		}

		for (int i = bi_deg; i < out_deg; ++i)
		{
			int v = out_nbrs[i];
			_affi_in[v][c] = 1.0;
			for (int w : _graph.out_neighbors(v))
			{
				if (std::binary_search(nodes, nodes + in_deg + out_deg - bi_deg, w))
				{
					_affi_out[v][c] = 1.0;
					break;
				}
			}
		}

		for (int i = bi_deg; i < in_deg; ++i)
		{
			int v = in_nbrs[i];
			_affi_out[v][c] = 1.0;
			for (int w : _graph.in_neighbors(v))
			{
				if (std::binary_search(nodes, nodes + in_deg + out_deg - bi_deg, w))
				{
					_affi_in[v][c] = 1.0;
					break;
				}
			}
		}

		delete[] nodes;
	}
	delete[] pairs;
	_make_affi_sum();
}

void affi_directed_model::init_random(unsigned seed)
{
	std::default_random_engine engine(seed);
	std::uniform_real_distribution<float> out_distr(0.0f, 1.0f);
	std::uniform_real_distribution<float> in_distr(0.0f, 1.0f);
	int n = _graph.node_num();
	for (int i = 0; i < n; ++i)
	{
		for (int c = 0; c < _cluster_num; ++c)
		{
			_affi_out[i][c] = out_distr(engine);
			_affi_in[i][c] = in_distr(engine);
		}
	}
	_make_affi_sum();
}

void affi_directed_model::_make_affi_sum()
{
	std::fill(_affi_sum_out, _affi_sum_out + _cluster_num, 0.0f);
	std::fill(_affi_sum_in, _affi_sum_in + _cluster_num, 0.0f);

	_task_type = _parallel_task_type::task_make_affi_sum;
	update();

	int n = _graph.node_num();
	for (int c = n; c < _cluster_num; ++c)
	{
		for (int i = 0; i < n; ++i)
		{
			_affi_sum_out[c] += _affi_out[i][c];
			_affi_sum_in[c] += _affi_in[i][c];
		}
	}
}

void affi_directed_model::_update_node_make_affi_sum(int c)
{
	if (c >= _cluster_num) return;

	int n = _graph.node_num();
	for (int i = 0; i < n; ++i)
	{
		_affi_sum_out[c] += _affi_out[i][c];
		_affi_sum_in[c] += _affi_in[i][c];
	}
}

void affi_directed_model::_update_node_likelihood(int node)
{
	auto out_nbrs = _graph.out_neighbors(node);
	float sum = 0.0;
	for (int c = 0; c < _cluster_num; ++c)
	{
		sum -= _affi_out[node][c] * (_affi_sum_in[c] - _affi_in[node][c]);
	}
	for (int v : out_nbrs)
	{
		float affi_prod_sum = 0.0;
		for (int c = 0; c < _cluster_num; ++c)
		{
			affi_prod_sum += _affi_out[node][c] * _affi_in[v][c];
		}
		sum += affi_prod_sum;
		float p = std::max(-expm1(-affi_prod_sum), _background_prob);
		sum += log(p);
	}
	_likelihood_buf[node] = sum;
}

float affi_directed_model::likelihood()
{
	if (_likelihood <= 0.0) return _likelihood;
	_task_type = _parallel_task_type::task_likelihood;
	update();
	_likelihood = 0.0;
	int n = _graph.node_num();
	for (int i = 0; i < n; ++i)
	{
		_likelihood += _likelihood_buf[i];
	}
	return _likelihood;
}

float affi_directed_model::likelihood(int node)
{
	likelihood();
	return _likelihood_buf[node];
}

float affi_directed_model::likelihood(bool *used)
{
	likelihood();
	if (used == NULL) return _likelihood;
	float sum = 0.0;
	int n = _graph.node_num();
	for (int i = 0; i < n; ++i)
	{
		if (used[i]) sum += _likelihood_buf[i];
	}
	return sum;
}

void affi_directed_model::_update_node_gradient_out(int node)
{
	for (int c = 0; c < _cluster_num; ++c)
	{
		_affi_out_d[node][c] = _affi_in[node][c] - _affi_sum_in[c];
	}
	auto out_nbrs = _graph.out_neighbors(node);
	for (int v : out_nbrs)
	{
		float affi_prod_sum = 0.0;
		for (int c = 0; c < _cluster_num; ++c)
		{
			affi_prod_sum += _affi_out[node][c] * _affi_in[v][c];
		}
		float p = std::max(-expm1(-affi_prod_sum), _background_prob);
		for (int c = 0; c < _cluster_num; ++c)
		{
			_affi_out_d[node][c] += _affi_in[v][c] / p;
		}
	}
}

void affi_directed_model::_update_node_gradient_in(int node)
{
	for (int c = 0; c < _cluster_num; ++c)
	{
		_affi_in_d[node][c] = _affi_out[node][c] - _affi_sum_out[c];
	}
	auto in_nbrs = _graph.in_neighbors(node);
	for (int u : in_nbrs)
	{
		float affi_prod_sum = 0.0;
		for (int c = 0; c < _cluster_num; ++c)
		{
			affi_prod_sum += _affi_out[u][c] * _affi_in[node][c];
		}
		float p = std::max(-expm1(-affi_prod_sum), _background_prob);
		for (int c = 0; c < _cluster_num; ++c)
		{
			_affi_in_d[node][c] += _affi_out[u][c] / p;
		}
	}
}

void affi_directed_model::_make_gradient_out()
{
	_task_type = _parallel_task_type::task_gradient_out;
	update();
}

void affi_directed_model::_make_gradient_in()
{
	_task_type = _parallel_task_type::task_gradient_in;
	update();
}

float affi_directed_model::iterate_out(float alpha, float scale, float decay, bool *is_train)
{
	float l0 = likelihood(is_train);
	_make_gradient_out();
	std::swap(_affi_out, _affi_out_tmp);
	std::swap(_likelihood_buf, _likelihood_buf_tmp);
	std::swap(_likelihood, _likelihood_tmp);
	int n = _graph.node_num();

	float m = 0.0;
	for (int i = 0; i < n; ++i)
	{
		for (int c = 0; c < _cluster_num; ++c)
		{
			m += _affi_out_d[i][c] * _affi_out_d[i][c];
		}
	}
	float cm = m * scale;
	alpha /= sqrt(m);

	printf("alpha = %g, m = %f, cm = %f\n", alpha, m, cm);

	//while (alpha > 1e-6)
	for (int loop = 0; loop < 10; ++loop)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int c = 0; c < _cluster_num; ++c)
			{
				_affi_out[i][c] = std::max(0.0f, _affi_out_tmp[i][c] + alpha * _affi_out_d[i][c]);
			}
		}

		_make_affi_sum();
		_likelihood = 1.0;
		float l1 = likelihood(is_train);

		printf("alpha = %g, improve = %f\n", alpha, (l0 - l1) / l0);

		if (l1 > l0 + alpha * cm) return alpha;
		alpha *= decay;
	}

	std::swap(_affi_out, _affi_out_tmp);
	std::swap(_likelihood_buf, _likelihood_buf_tmp);
	std::swap(_likelihood, _likelihood_tmp);
	_make_affi_sum();
	return 0.0;
}

float affi_directed_model::iterate_in(float alpha, float scale, float decay, bool *is_train)
{
	float l0 = likelihood(is_train);
	_make_gradient_in();
	std::swap(_affi_in, _affi_in_tmp);
	std::swap(_likelihood_buf, _likelihood_buf_tmp);
	std::swap(_likelihood, _likelihood_tmp);
	int n = _graph.node_num();

	float m = 0.0;
	for (int i = 0; i < n; ++i)
	{
		for (int c = 0; c < _cluster_num; ++c)
		{
			m += _affi_in_d[i][c] * _affi_in_d[i][c];
		}
	}
	float cm = m * scale;
	alpha /= sqrt(m);

	printf("alpha = %g, m = %f, cm = %f\n", alpha, m, cm);

	//while (alpha > 1e-6)
	for (int loop = 0; loop < 10; ++loop)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int c = 0; c < _cluster_num; ++c)
			{
				_affi_in[i][c] = std::max(0.0f, _affi_in_tmp[i][c] + alpha * _affi_in_d[i][c]);
			}
		}

		_make_affi_sum();
		_likelihood = 1.0;
		float l1 = likelihood(is_train);

		printf("alpha = %g, improve = %f\n", alpha, (l0 - l1) / l0);

		if (l1 > l0 + alpha * cm) return alpha;
		alpha *= decay;
	}

	std::swap(_affi_in, _affi_in_tmp);
	std::swap(_likelihood_buf, _likelihood_buf_tmp);
	std::swap(_likelihood, _likelihood_tmp);
	_make_affi_sum();
	return 0.0;
}

float **affi_directed_model::affinity_out()
{
	return _affi_out;
}

float **affi_directed_model::affinity_in()
{
	return _affi_in;
}

float affi_directed_model::argmin_out(float alpha, float scale, float decay, bool *is_train, float rel_improve)
{
	printf("argmin_out: ");
	float l0 = likelihood(is_train);
	for (int loop = 0;; ++loop)
	{
		printf(".");
		float l1 = likelihood(is_train);
		float step = iterate_out(alpha, scale, decay, is_train);
		float l2 = likelihood(is_train);
		if (step == 0.0) break;
		float r = (l1 - l2) / l1;
		if (r < rel_improve) break;
	}
	float improve = (l0 - likelihood(is_train)) / l0;
	printf("\n");
	//printf("Improve: %f\n", improve);
	return improve;
}

float affi_directed_model::argmin_in(float alpha, float scale, float decay, bool *is_train, float rel_improve)
{
	printf("argmin_in: ");
	float l0 = likelihood(is_train);
	for (int loop = 0;; ++loop)
	{
		printf(".");
		float l1 = likelihood(is_train);
		float step = iterate_in(alpha, scale, decay, is_train);
		float l2 = likelihood(is_train);
		if (step == 0.0) break;
		float r = (l1 - l2) / l1;
		if (r < rel_improve) break;
	}
	float improve = (l0 - likelihood(is_train)) / l0;
	printf("\n");
	//printf("Improve: %f\n", improve);
	return improve;

}

float affi_directed_model::converge(float alpha, float scale, float decay, bool *is_train, float rel_improve)
{
	float l0 = likelihood(is_train);
	for (int loop = 0; loop < 50; ++loop)
	{
		float improve_out = argmin_out(alpha, scale, decay, is_train, rel_improve);
		float improve_in = argmin_in(alpha, scale, decay, is_train, rel_improve);
		printf("Loop %d: improve_out = %f, improve_in = %f\n", loop, improve_out, improve_in);
		//printf(".");
		if (improve_out < rel_improve && improve_in < rel_improve) break;
	}
	printf("\n");
	float l1 = likelihood(is_train);
	return (l0 - l1) / l0;
}

int affi_directed_model::node_num()
{
	return _graph.node_num();
}

int affi_directed_model::cluster_num()
{
	return _cluster_num;
}
