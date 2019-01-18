#pragma once

#include <algorithm>
#include <queue>
#include <thread>
#include <functional>

namespace graph
{
	template <class RandIt, class T>
	RandIt binary_search(RandIt first, RandIt last, T value)
	{
		RandIt it = std::lower_bound(first, last, value);
		if (it == last) return last;
		return (*it == value) ? it : last;
	}

	template <class RandIt, class T, class Pred>
	RandIt binary_search(RandIt first, RandIt last, T value, Pred pred)
	{
		RandIt it = std::lower_bound(first, last, value, pred);
		return (it == last || pred(*it, value) || pred(value, *it)) ? last : it;
	}

	template <class RandIt> 
	void sort(RandIt first, RandIt last, size_t thread_num)
	{
		if (last <= first) return;

		if ((size_t)(last - first) <= thread_num * 32)
		{
			std::sort(first, last);
			return;
		}

		std::priority_queue<std::pair<size_t, RandIt>> queue;
		queue.push(std::make_pair(last - first, first));

		while (queue.size() < thread_num)
		{
			RandIt it1 = queue.top().second;
			RandIt it2 = it1 + queue.top().first;
			std::pair<RandIt, RandIt> mid = std::_Unguarded_partition(it1, it2, std::less<void>());
			queue.pop();
			queue.push(std::make_pair(mid.first - it1, it1));
			queue.push(std::make_pair(it2 - mid.second, mid.second));
		}

		std::thread *threads = new std::thread[thread_num];
		for (size_t i = 0; i < thread_num; ++i)
		{
			RandIt it1 = queue.top().second;
			RandIt it2 = it1 + queue.top().first;
			queue.pop();
			threads[i] = std::thread(std::sort<RandIt>,	it1, it2);
		}
		for (size_t i = 0; i < thread_num; ++i)
		{
			threads[i].join();
		}
		delete[] threads;
	}

	template <class RandIt, class Pred> 
	void sort(RandIt first, RandIt last, Pred pred, size_t thread_num)
	{
		if (last <= first) return;

		if ((size_t)(last - first) <= thread_num * 32)
		{
			std::sort(first, last, pred);
			return;
		}

		std::priority_queue<std::pair<size_t, RandIt>> queue;
		queue.push(std::make_pair(last - first, first));

		while (queue.size() < thread_num)
		{
			RandIt it1 = queue.top().second;
			RandIt it2 = it1 + queue.top().first;
			std::pair<RandIt, RandIt> mid = std::_Unguarded_partition(it1, it2, pred);
			queue.pop();
			queue.push(std::make_pair(mid.first - it1, it1));
			queue.push(std::make_pair(it2 - mid.second, mid.second));
		}

		std::thread *threads = new std::thread[thread_num];
		for (size_t i = 0; i < thread_num; ++i)
		{
			RandIt it1 = queue.top().second;
			RandIt it2 = it1 + queue.top().first;
			queue.pop();
			threads[i] = std::thread(std::sort<RandIt, Pred>, it1, it2, pred);
		}
		for (size_t i = 0; i < thread_num; ++i)
		{
			threads[i].join();
		}
		delete[] threads;
	}

	template<class T>
	size_t fread(T *data, size_t num, FILE *fp)
	{
		const size_t block_size = (1 << 30) / sizeof(T);
		size_t count = 0;
		while (num > 0)
		{
			size_t more = std::min(block_size, num);
			size_t got = fread(data, sizeof(T), more, fp);
			if (got == 0) break;
			count += got;
			num -= got;
			data += got;
		}
		return count;
	}

	template<class T>
	size_t fwrite(T *data, size_t num, FILE *fp)
	{
		const size_t block_size = (1 << 30) / sizeof(T);
		size_t count = 0;
		while (num > 0)
		{
			size_t more = std::min(block_size, num);
			size_t got = fwrite(data, sizeof(T), more, fp);
			count += got;
			num -= got;
			data += got;
			if (got != more) break;
		}
		return count;
	}

	class text_file
	{
	public:
		text_file(const char *path)
		{
			FILE *fp_in = fopen(path, "rb");
#ifdef  _WIN64
			_fseeki64(fp_in, 0, SEEK_END);
			_size = _ftelli64(fp_in);
			_fseeki64(fp_in, 0, SEEK_SET);
#else
			fseek(fp_in, 0, SEEK_END);
			_size = ftell(fp_in);
			fseek(fp_in, 0, SEEK_SET);
#endif
			_data = new char[_size + 1];
			graph::fread(_data, _size, fp_in);
			_data[_size] = 0;
			fclose(fp_in);

			_line_num = 0;
			for (size_t offset = 0; ; )
			{
				if (_data[offset] == '\r')
				{
					++_line_num;
					++offset;
					if (_data[offset] == '\n') ++offset;
					if (offset == _size) break;
				}
				else if (_data[offset] == '\n')
				{
					++_line_num;
					++offset;
					if (offset == _size) break;
				}
				else if (offset == _size)
				{
					if (offset > 0) ++_line_num;
					break;
				}
				else
				{
					++offset;
				}
			}

			_lines = new char*[_line_num];
			if (_line_num == 0) return;
			_lines[0] = _data;

			for (size_t offset = 0, index = 0; ; )
			{
				if (_data[offset] == '\r')
				{
					_data[offset] = 0;
					++offset;
					if (offset < _size && _data[offset] == '\n') ++offset;
					if (offset == _size) break;
					_lines[++index] = _data + offset;
				}
				else if (_data[offset] == '\n')
				{
					_data[offset] = 0;
					++offset;
					if (offset == _size) break;
					_lines[++index] = _data + offset;
				}
				else if (offset == _size)
				{
					break;
				}
				else
				{
					++offset;
				}
			}
		}

		~text_file()
		{
			delete[] _data;
			delete[] _lines;
		}

		char *line(size_t index)
		{
			return _lines[index];
		}

		char **begin()
		{
			return _lines;
		}

		char **end()
		{
			return _lines + _line_num;
		}

		size_t line_num()
		{
			return _line_num;
		}

		size_t size()
		{
			return _size;
		}

	private:
		char *_data;
		char **_lines;
		size_t _line_num, _size;
	};

	struct string_comparer
	{
		bool operator()(const char *a, const char *b)
		{
			return strcmp(a, b) < 0;
		}
	};

	struct string_predicate
	{
		bool operator()(const char *a, const char *b)
		{
			return strcmp(a, b) == 0;
		}
	};

	struct _build_directed_graph_info
	{
		char **first_node;
		char **last_node;
		std::pair<char*, char*> *first_edge;
		std::pair<char*, char*> *last_edge;
	};

	template <class Node>
	void _build_directed_graph_parse_edge(const _build_directed_graph_info info, Node *out_nodes, Node *in_nodes)
	{
		static_assert(__is_enum(Node), "The node type is not supported.");
	}

	template <>
	inline void _build_directed_graph_parse_edge<int>(const _build_directed_graph_info info, int *out_nodes, int *in_nodes)
	{
		for (std::pair<char*, char*> *edge = info.first_edge; edge != info.last_edge; ++edge)
		{
			*out_nodes++ = (int)(std::lower_bound(info.first_node, info.last_node, edge->first, string_comparer()) - info.first_node);
			*in_nodes++ = (int)(std::lower_bound(info.first_node, info.last_node, edge->second, string_comparer()) - info.first_node);
		}
	}

	template <>
	inline void _build_directed_graph_parse_edge<long long>(const _build_directed_graph_info info, long long *out_nodes, long long *in_nodes)
	{
		for (std::pair<char*, char*> *edge = info.first_edge; edge != info.last_edge; ++edge)
		{
			*out_nodes++ = (long long)(std::lower_bound(info.first_node, info.last_node, edge->first, string_comparer()) - info.first_node);
			*in_nodes++ = (long long)(std::lower_bound(info.first_node, info.last_node, edge->second, string_comparer()) - info.first_node);
		}
	}

	template <class Node>
	directed_graph<Node, char *, void> build_directed_graph(const char *path, int separator, size_t thread_num)
	{
		text_file text(path);
		size_t edge_num = text.line_num();
		char **id_strs = new char*[edge_num * 2];
		std::pair<char*, char*> *edge_strs = new std::pair<char*, char*>[edge_num];
		for (size_t i = 0; i < edge_num; ++i)
		{
			char *str1 = text.line(i);
			char *str2 = strchr(str1, separator);
			if (str2 == NULL)
			{
				printf("Incorrect format in line %lld: %s\n", (long long)i, text.line(i));
				exit(1);
			}
			*str2++ = 0;
			id_strs[i * 2] = str1;
			id_strs[i * 2 + 1] = str2;
			edge_strs[i] = std::make_pair(str1, str2);
		}
		graph::sort(id_strs, id_strs + edge_num * 2, string_comparer(), thread_num);
		Node node_num = (Node)(std::unique(id_strs, id_strs + edge_num * 2, string_predicate()) - id_strs);

		Node *out_nodes = new Node[edge_num];
		Node *in_nodes = new Node[edge_num];

		std::thread *threads = new std::thread[thread_num];
		for (size_t i = 0; i < thread_num; ++i)
		{
			size_t lo = (size_t)((long long)i * edge_num / thread_num);
			size_t hi = (size_t)((long long)(i + 1) * edge_num / thread_num);
			_build_directed_graph_info info;
			info.first_edge = edge_strs + lo;
			info.last_edge = edge_strs + hi;
			info.first_node = id_strs;
			info.last_node = id_strs + node_num;
			threads[i] = std::thread(_build_directed_graph_parse_edge<Node>, info, out_nodes + lo, in_nodes + lo);
		}
		for (size_t i = 0; i < thread_num; ++i)
		{
			threads[i].join();
		}
		delete[] threads;

		directed_graph<Node, char*, void> g;
		g.build(node_num, id_strs, edge_num, out_nodes, in_nodes);

		delete[] in_nodes;
		delete[] out_nodes;
		delete[] edge_strs;
		delete[] id_strs;
		return g;
	}
}