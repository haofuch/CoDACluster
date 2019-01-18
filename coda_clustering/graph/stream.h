#pragma once

#include <cstdio>

namespace graph
{
	class istream
	{
	public:
		template <class T> size_t read(T *data, size_t num);
		void close();
	};

	class file_istream : public istream
	{
	public:
		file_istream(const char *path)
		{
			_fp_in = fopen(path, "rb");
		}

		~file_istream()
		{
			close();
		}

		template <class T> size_t read(T *data, size_t num)
		{
			const size_t block_size = (1 << 30) / sizeof(T);
			size_t count = 0;
			while (num > 0)
			{
				size_t more = std::min(block_size, num);
				size_t got = fread(data, sizeof(T), more, _fp_in);
				if (got == 0) break;
				count += got;
				num -= got;
				data += got;
			}
			return count;
		}

		void close()
		{
			if (_fp_in != NULL)
			{
				fclose(_fp_in);
				_fp_in = NULL;
			}
		}

	private:
		FILE *_fp_in;
	};

	class ostream
	{
	public:
		template <class T> size_t write(const T *data, size_t num);
		void close();
	};

	class file_ostream : ostream
	{
	public:
		file_ostream(const char *path)
		{
			_fp_out = fopen(path, "wb");
		}

		~file_ostream()
		{
			close();
		}

		template <class T> size_t write(const T *data, size_t num)
		{
			const size_t block_size = (1 << 30) / sizeof(T);
			size_t count = 0;
			while (num > 0)
			{
				size_t more = std::min(block_size, num);
				size_t got = fwrite(data, sizeof(T), more, _fp_out);
				count += got;
				num -= got;
				data += got;
				if (got != more) break;
			}
			return count;
		}

		void close()
		{
			if (_fp_out != NULL)
			{
				fclose(_fp_out);
				_fp_out = NULL;
			}
		}

	private:
		FILE *_fp_out;
	};

}