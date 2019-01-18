#pragma once

#include <cstdlib>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <type_traits>

namespace graph
{

	class thread_pool
	{
	public:
		thread_pool(size_t thread_num)
		{
			_thread_num = thread_num;
			_is_destructed = false;
			_tasks = new std::function<void()>[_thread_num];
			_start_mutexes = new std::mutex[_thread_num];
			_start_cvs = new std::condition_variable[_thread_num];
			_threads = new std::thread[_thread_num];
			for (size_t i = 0; i < _thread_num; ++i)
			{
				_threads[i] = std::thread(&thread_pool::_thread_loop, this, i);
			}
		}

		~thread_pool()
		{
			_is_destructed = true;
			for (size_t i = 0; i < _thread_num; ++i)
			{
				std::unique_lock<std::mutex> lock(_start_mutexes[i]);
				_start_cvs[i].notify_one();
			}
			for (size_t i = 0; i < _thread_num; ++i)
			{
				_threads[i].join();
			}
			delete[] _tasks;
			delete[] _start_mutexes;
			delete[] _start_cvs;
			delete[] _threads;
		}

		template <class Func, class... Args>
		void invoke(Func&& func, Args&&... args)
		{
			std::unique_lock<std::mutex> lock(_ready_mutex);
			size_t id;
			while (true)
			{
				for (id = 0; id < _thread_num; ++id)
				{
					if (!_tasks[id]) break;
				}
				if (id < _thread_num) break;
				_ready_cv.wait(lock);
			}


			{
				std::unique_lock<std::mutex> lock(_start_mutexes[id]);
				typedef typename std::result_of<Func(Args...)>::type Result;
				_tasks[id] = std::bind(std::_Decay_copy(std::forward<Func>(func)), std::_Decay_copy(std::forward<Args>(args))...);
				_start_cvs[id].notify_one();
			}

		}

	private:
		size_t _thread_num;
		bool _is_destructed;
		std::thread *_threads;
		std::function<void()> *_tasks;
		std::mutex *_start_mutexes;
		std::mutex _ready_mutex;
		std::condition_variable *_start_cvs;
		std::condition_variable _ready_cv;

		void _thread_loop(size_t id)
		{
			while (true)
			{
				{
					std::unique_lock<std::mutex> lock(_start_mutexes[id]);
					while (!_is_destructed && !_tasks[id]) _start_cvs[id].wait(lock);
					if (!_tasks[id]) break;
				}

				_tasks[id]();

				{
					std::unique_lock<std::mutex> lock(_ready_mutex);
					std::function<void()> empty;
					empty.swap(_tasks[id]);
					_ready_cv.notify_one();
				}
			}
		}
	};

}