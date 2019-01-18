#include <cstdio>
#include <cstdlib>
#include <direct.h>
#include <thread>
#include <algorithm>
#include <vector>

#include "graph/graph.h"
#include "affi_directed_model.h"

typedef graph::directed_graph<int, char *> graph_t;

template <class T> T** load_array2(const char *path, size_t num, size_t len)
{
	T *data = new T[num * len];
	T **entry = new T*[num];
	graph::file_istream is(path);
	for (size_t i = 0; i < num; ++i)
	{
		entry[i] = data + i * len;
		if (is.read(entry[i], len) != len)
		{
			fprintf(stderr, "Expected file end in %s\n", path);
			exit(1);
		}
	}
	return entry;
}

template <class T> void save_array2(const char *path, T **data, size_t num, size_t len)
{
	graph::file_ostream os(path);
	for (size_t i = 0; i < num; ++i)
	{
		os.write(data[i], len);
	}
}

void load_label_string(graph::directed_graph<int, char *> &g, char *label[], const char *path)
{
	int n = g.node_num();
	std::fill(label, label + n, nullptr);
	auto text = graph::text_file(path);
	char *data = new char[text.size()];
	for (auto line : text)
	{
		char uid[1024];
		if (sscanf(line, "%s%s", uid, data) != 2) continue;
		int node = (int)(graph::binary_search(g.node_attrs().begin(), g.node_attrs().end(), uid, graph::string_comparer()) - g.node_attrs().begin());
		if (node >= 0 && node < n)
		{
			label[node] = data;
			data += strlen(data) + 1;
		}
	}
}

void train_and_save(int cluster_num, const char *graph_path, const char *affi_in_path, const char *affi_out_path)
{
	graph_t g;
	g.load(graph::file_istream(graph_path));
	printf("%d nodes, %lld edges\n", g.node_num(), g.edge_num());
	affi_directed_model adm(g, cluster_num, 32);
	adm.init_min_neighborhood();

	printf("initialized\n");

	double improve = adm.converge(100.0f, 1e-3f, 0.5f, NULL, 1e-4f);
	printf("Improve = %f\n", improve);

	save_array2(affi_in_path, adm.affinity_in(), adm.node_num(), adm.cluster_num());
	save_array2(affi_out_path, adm.affinity_out(), adm.node_num(), adm.cluster_num());
}


int main()
{
	return 0;
}