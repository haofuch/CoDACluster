#pragma once

#include <cstdlib>
#include <cstring>

namespace graph
{
	class string_container
	{
	public:
		typedef char **iterator;

		string_container()
		{
			_entry = NULL;
			_data = NULL;
		}

		~string_container()
		{
			if (_entry != NULL)
			{
				delete[] _entry;
				_entry = NULL;
				_size = 0;
			}

			if (_data != NULL)
			{
				delete[] _data;
				_data = NULL;
				_length = 0;
			}
		}

		iterator begin()
		{
			return _entry;
		}

		iterator end()
		{
			return _entry + _size;
		}

		char *operator[](size_t index)
		{
			return _entry[index];
		}

		template <class InIt> void load(InIt first, InIt last)
		{
			this->~string_container();
			_size = 0;
			_length = 0;
			for (InIt it = first; it != last; ++it)
			{
				_length += strlen(*it) + 1;
				++_size;
			}
			_entry = new char*[_size];
			_data = new char[_length];
			char *ptr = _data;
			for (long long i = 0; i < _size; ++i)
			{
				strcpy(ptr, *first);
				_entry[i] = ptr;
				ptr += strlen(ptr) + 1;
				++first;
			}
		}

		template <class IStream> bool load(IStream &istream)
		{
			this->~string_container();
			if (istream.read(&_length, 1) != 1) return false;
			_data = new char[_length];
			if (istream.read(_data, _length) != _length) return false;
			_size = 0;
			for (long long i = 0; i < _length; ++i)
			{
				if (_data[i] == 0) ++_size;
			}
			_entry = new char*[_size];
			char *ptr = _data;
			for (long long i = 0; i < _size; ++i)
			{
				_entry[i] = ptr;
				ptr += strlen(ptr) + 1;
			}
			return true;
		}

		template <class OStream> bool save(OStream &ostream)
		{
			if (ostream.write(&_length, 1) != 1) return false;
			if (ostream.write(_data, _length) != _length) return false;
			return true;
		}

	private:
		char **_entry;
		char *_data;
		long long _size;
		long long _length;
	};

	template <class T> 
	class default_container
	{
	public:
		typedef T *iterator;

		default_container()
		{
			_data = NULL;
			_size = 0;
		}

		~default_container()
		{
			if (_data != NULL)
			{
				delete[] _data;
				_data = NULL;
				_size = 0;
			}
		}

		iterator begin()
		{
			return _data;
		}

		iterator end()
		{
			return _data + _size;
		}

		T &operator[](size_t index)
		{
			return _data[index];
		}

		template <class InIt> void load(InIt first, InIt last)
		{
			this->~default_container();
			_size = last - first;
			_data = new T[_size];
			for (T *ptr = _data; first != last; ++ptr, ++first)
			{
				*ptr = *first;
			}
		}

		template <class IStream> bool load(IStream &istream)
		{
			this->~default_container();
			if (istream.read(&_size, 1) != 1) return false;
			_data = new T[_size];
			if (istream.read(_data, _size) != _size) return false;
			return true;
		}

		template <class OStream> bool save(OStream &ostream)
		{
			if (ostream.write(&_size, 1) != 1) return false;
			if (ostream.write(_data, _size) != _size) return false;
			return true;
		}

	private:
		T *_data;
		long long _size;
	};

	template<>
	class default_container<void>
	{
	};

	template<>
	class default_container<char *> : public string_container
	{
	};

	template <class RandIt>
	class wrapper_container
	{
	protected:
		RandIt _first, _last;

	public:
		wrapper_container(RandIt first, RandIt last) : _first(first), _last(last) { }

		RandIt begin() const
		{
			return _first;
		}

		RandIt end() const
		{
			return _last;
		}

		auto operator[](long long index) -> decltype(_first[index]) const
		{
			return _first[index];
		}

		auto operator[](int index) -> decltype(_first[index]) const
		{
			return _first[index];
		}

		auto operator[](size_t index) -> decltype(_first[index]) const
		{
			return _first[index];
		}

		auto size() -> decltype(_last - _first) const
		{
			return _last - _first;
		}

	};

	template<class T>
	class default_converter
	{
		T &operator()(T &val)
		{
			return val;
		}
	};
}