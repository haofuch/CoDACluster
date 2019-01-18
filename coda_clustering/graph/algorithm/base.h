#pragma once

#include "../directed_graph.h"

#include <thread>
#include <condition_variable>
#include <mutex>

namespace graph
{
	template <class Graph> class parallel_algo
	{
	public:
		parallel_algo(Graph &g, size_t thread_num) : _graph(g), _thread_num(thread_num)
		{
			_threads = new std::thread[thread_num];
			_cvs1 = new std::condition_variable[thread_num];
			_cvs2 = new std::condition_variable[thread_num];
			_signals = new _signal[thread_num];
			_mutexes = new std::mutex[thread_num];

			for (size_t i = 0; i < thread_num; ++i)
			{
				_signals[i] = _signal::suspend;
				_threads[i] = std::thread(&parallel_algo::_update_partition, this, (Graph::node_t)i);
			}

		}

		~parallel_algo()
		{
			for (size_t i = 0; i < _thread_num; ++i)
			{
				std::unique_lock<std::mutex> lock(_mutexes[i]);
				_signals[i] = _signal::exit;
				_cvs1[i].notify_one();
			}
			for (size_t i = 0; i < _thread_num; ++i)
			{
				_threads[i].join();
			}
			delete[] _mutexes;
			delete[] _signals;
			delete[] _threads;
			delete[] _cvs1;
			delete[] _cvs2;
		}

		virtual void update_node(typename Graph::node_t node) = 0;
		//void update_node(typename Graph::node_t node)
		//{
		//}

		void update()
		{
			for (size_t i = 0; i < _thread_num; ++i)
			{
				std::unique_lock<std::mutex> lock(_mutexes[i]);
				_signals[i] = _signal::run;
				_cvs1[i].notify_one();
			}

			for (size_t i = 0; i < _thread_num; ++i)
			{
				std::unique_lock<std::mutex> lock(_mutexes[i]);
				while (_signals[i] != _signal::suspend) _cvs2[i].wait(lock);
			}
		}

	protected:
		Graph &_graph;
		size_t _thread_num;
		std::thread *_threads;
		std::condition_variable *_cvs1, *_cvs2;
		std::mutex *_mutexes;

		enum _signal
		{
			suspend, run, exit
		};

		_signal *_signals;

		void _update_partition(typename Graph::node_t start)
		{
			while (true)
			{
				{
					std::unique_lock<std::mutex> lock(_mutexes[start]);
					while (_signals[start] == _signal::suspend) _cvs1[start].wait(lock);
					if (_signals[start] == _signal::exit) return;
				}

				//printf("Run %d\n", start);

				for (Graph::node_t node = start; node < _graph.node_num(); node += (Graph::node_t)_thread_num)
				{
					update_node(node);
				}

				//printf("Suspend %d\n", start);

				{
					std::unique_lock<std::mutex> lock(_mutexes[start]);
					_signals[start] = _signal::suspend;
					_cvs2[start].notify_one();
				}

			}
		}
	};
}
