#pragma once

#include <cstring>
#include <algorithm>

#include "container.h"
#include "stream.h"

namespace graph
{
	template <class Node>
	class _directed_graph_base3
	{
	public:

		typedef const Node *neighbor_iterator;

		typedef wrapper_container<neighbor_iterator> neighbor_container;

		template <class Node, class EdgeAttr>
		class edge
		{
		public:
			Node source, dest;
			EdgeAttr attr;

			edge() { }
			edge(Node source, Node dest, EdgeAttr attr) : source(source), dest(dest), attr(attr) { }
		};

		template <class Node>
		class edge <Node, void>
		{
		public:
			Node source, dest;

			edge() { }
			edge(Node source, Node dest) : source(source), dest(dest) { }
		};

		template <class Node, class EdgeAttr, class EdgeAttrContainer> class edge_container;

		template <class Node, class EdgeAttr, class EdgeAttrContainer>
		class edge_iterator
		{
		public:
			edge_iterator(long long offset, Node node, Node node_num, long long *offsets, Node *nodes, typename EdgeAttrContainer::iterator attrs, bool is_reversed)
				: _offset(offset), _node(node), _node_num(node_num), _offsets(offsets), _nodes(nodes), _attrs(attrs), _is_reversed(is_reversed)
			{
			}

			friend bool operator==(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset == b._offset;
			}

			friend bool operator!=(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset != b._offset;
			}

			friend bool operator<(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset < b._offset;
			}

			friend bool operator>(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset > b._offset;
			}

			friend bool operator<=(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset <= b._offset;
			}

			friend bool operator>=(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset >= b._offset;
			}

			edge_iterator &operator++()
			{
				++_offset;
				while (_node < _node_num && _offset == _offsets[_node + 1]) ++_node;
				return *this;
			}

			edge_iterator operator++(int)
			{
				edge_iterator tmp(*this);
				++_offset;
				while (_node < _node_num && _offset == _offsets[_node + 1]) ++_node;
				return tmp;
			}

			edge_iterator &operator--()
			{
				--_offset;
				while (_node > 0 && _offset < _offsets[_node]) --_node;
				return *this;
			}

			edge_iterator operator--(int)
			{
				edge_iterator tmp(*this);
				--_offset;
				while (_node > 0 && _offset < _offsets[_node]) --_node;
				return tmp;
			}

			const edge<Node, EdgeAttr> *operator->() const
			{
				return _is_reversed ?
					&edge<Node, EdgeAttr>(_nodes[_offset], _node, _attrs[_offset]) :
					&edge<Node, EdgeAttr>(_node, _nodes[_offset], _attrs[_offset]);
			}

			const edge<Node, EdgeAttr> operator*() const
			{
				return _is_reversed ?
					edge<Node, EdgeAttr>(_nodes[_offset], _node, _attrs[_offset]) :
					edge<Node, EdgeAttr>(_node, _nodes[_offset], _attrs[_offset]);
			}

			const edge<Node, EdgeAttr> operator[](long long index) const
			{
				return _is_reversed ?
					edge<Node, EdgeAttr>(_nodes[_offset + index], _node, _attrs[_offset + index]) :
					edge<Node, EdgeAttr>(_node, _nodes[_offset + index], _attrs[_offset + index]);
			}

			edge_iterator &operator+=(long long offset)
			{
				_offset += offset;
				if (_offset >= _offsets[_node_num])
				{
					_offset = _offsets[_node_num];
					_node = _node_num;
				}
				else if (_offset < _offsets[_node] || _offset >= _offsets[_node + 1])
				{
					_node = (Node)(std::upper_bound(_offsets, _offsets + _node_num + 1, _offset) - _offsets - 1);
				}
				return *this;
			}

			edge_iterator &operator-=(long long offset)
			{
				_offset -= offset;
				if (_offset >= _offsets[_node_num])
				{
					_offset = _offsets[_node_num];
					_node = _node_num;
				}
				else if (_offset < _offsets[_node] || _offset >= _offsets[_node + 1])
				{
					_node = (Node)(std::upper_bound(_offsets, _offsets + _node_num + 1, _offset) - _offsets - 1);
				}
				return *this;
			}

			friend edge_iterator operator+(const edge_iterator &a, long long offset)
			{
				edge_iterator tmp(a);
				tmp += offset;
				return tmp;
			}

			friend edge_iterator operator+(long long offset, const edge_iterator &a)
			{
				edge_iterator tmp(a);
				tmp += offset;
				return tmp;
			}

			friend edge_iterator operator-(const edge_iterator &a, long long offset)
			{
				edge_iterator tmp(a);
				tmp -= offset;
				return tmp;
			}

			friend long long operator-(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset - b._offset;
			}

			friend class edge_container<Node, EdgeAttr, EdgeAttrContainer>;

		private:
			long long _offset;
			Node _node, _node_num;
			long long *_offsets;
			Node *_nodes;
			typename EdgeAttrContainer::iterator _attrs;
			bool _is_reversed;
		};

		template <class Node>
		class edge_iterator<Node, void, void>
		{
		public:
			edge_iterator(long long offset, Node node, Node node_num, long long *offsets, Node *nodes, bool is_reversed)
				: _offset(offset), _node(node), _node_num(node_num), _offsets(offsets), _nodes(nodes), _is_reversed(is_reversed)
			{
			}

			friend bool operator==(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset == b._offset;
			}

			friend bool operator!=(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset != b._offset;
			}

			friend bool operator<(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset < b._offset;
			}

			friend bool operator>(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset > b._offset;
			}

			friend bool operator<=(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset <= b._offset;
			}

			friend bool operator>=(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset >= b._offset;
			}

			edge_iterator &operator++()
			{
				++_offset;
				while (_node < _node_num && _offset == _offsets[_node + 1]) ++_node;
				return *this;
			}

			edge_iterator operator++(int)
			{
				edge_iterator tmp(*this);
				++_offset;
				while (_node < _node_num && _offset == _offsets[_node + 1]) ++_node;
				return tmp;
			}

			edge_iterator &operator--()
			{
				--_offset;
				while (_node > 0 && _offset < _offsets[_node]) --_node;
				return *this;
			}

			edge_iterator operator--(int)
			{
				edge_iterator tmp(*this);
				--_offset;
				while (_node > 0 && _offset < _offsets[_node]) --_node;
				return tmp;
			}

			const edge<Node, void> *operator->() const
			{
				return _is_reversed ?
					&edge<Node, void>(_nodes[_offset], _node) :
					&edge<Node, void>(_node, _nodes[_offset]);
			}

			const edge<Node, void> operator*() const
			{
				return _is_reversed ?
					edge<Node, void>(_nodes[_offset], _node) :
					edge<Node, void>(_node, _nodes[_offset]);
			}

			const edge<Node, void> operator[](long long index) const
			{
				return _is_reversed ?
					edge<Node, void>(_nodes[_offset + index], _node) :
					edge<Node, void>(_node, _nodes[_offset + index]);
			}

			edge_iterator &operator+=(long long offset)
			{
				_offset += offset;
				if (_offset >= _offsets[_node_num])
				{
					_offset = _offsets[_node_num];
					_node = _node_num;
				}
				else if (_offset < _offsets[_node] || _offset >= _offsets[_node + 1])
				{
					_node = (Node)(std::upper_bound(_offsets, _offsets + _node_num + 1, _offset) - _offsets - 1);
				}
				return *this;
			}

			edge_iterator &operator-=(long long offset)
			{
				_offset -= offset;
				if (_offset >= _offsets[_node_num])
				{
					_offset = _offsets[_node_num];
					_node = _node_num;
				}
				else if (_offset < _offsets[_node] || _offset >= _offsets[_node + 1])
				{
					_node = (Node)(std::upper_bound(_offsets, _offsets + _node_num + 1, _offset) - _offsets - 1);
				}
				return *this;
			}

			friend edge_iterator operator+(const edge_iterator &a, long long offset)
			{
				edge_iterator tmp(a);
				tmp += offset;
				return tmp;
			}

			friend edge_iterator operator+(long long offset, const edge_iterator &a)
			{
				edge_iterator tmp(a);
				tmp += offset;
				return tmp;
			}

			friend edge_iterator operator-(const edge_iterator &a, long long offset)
			{
				edge_iterator tmp(a);
				tmp -= offset;
				return tmp;
			}

			friend long long operator-(const edge_iterator &a, const edge_iterator &b)
			{
				return a._offset - b._offset;
			}

			friend class edge_container<Node, void, void>;

		private:
			long long _offset;
			Node _node, _node_num;
			long long *_offsets;
			Node *_nodes;
			bool _is_reversed;
		};


		template <class Node, class EdgeAttr, class EdgeAttrContainer>
		class edge_container : public wrapper_container<edge_iterator<Node, EdgeAttr, EdgeAttrContainer>>
		{
		public:
			typedef edge_iterator<Node, EdgeAttr, EdgeAttrContainer> iterator;
			
			edge_container(iterator first, iterator last) : wrapper_container(first, last)
			{

			}

			long long degree_sum(Node node) const
			{
				return _first._offsets[node];
			}
		};

		_directed_graph_base3()
		{
			_node_num = 0;
			_edge_num = 0;
			_out_offsets = NULL;
			_in_offsets = NULL;
			_bi_degrees = NULL;
			_out_nbrs = NULL;
			_in_nbrs = NULL;
		}

		_directed_graph_base3(const _directed_graph_base3 &other)
		{
			if (other._node_num == 0)
			{
				_node_num = 0;
				_edge_num = 0;
				_out_offsets = NULL;
				_in_offsets = NULL;
				_bi_degrees = NULL;
				_out_nbrs = NULL;
				_in_nbrs = NULL;
			}
			else
			{
				_alloc(other._node_num, other._edge_num);
				std::copy(other._out_offsets, other._out_offsets + _node_num + 1, _out_offsets);
				std::copy(other._in_offsets, other._in_offsets + _node_num + 1, _in_offsets);
				std::copy(other._bi_degrees, other._bi_degrees + _node_num, _bi_degrees);
				std::copy(other._out_nbrs, other._out_nbrs + _edge_num, _out_nbrs);
				std::copy(other._in_nbrs, other._in_nbrs + _edge_num, _in_nbrs);
			}
		}

		~_directed_graph_base3()
		{
			_release();
		}

		void swap(_directed_graph_base3 &other)
		{
			std::swap(_node_num, other._node_num);
			std::swap(_edge_num, other._edge_num);
			std::swap(_out_offsets, other._out_offsets);
			std::swap(_in_offsets, other._in_offsets);
			std::swap(_bi_degrees, other._bi_degrees);
			std::swap(_out_nbrs, other._out_nbrs);
			std::swap(_in_nbrs, other._in_nbrs);
		}

		Node node_num() const
		{
			return _node_num;
		}

		long long edge_num() const
		{
			return _edge_num;
		}

		Node out_degree(Node node) const
		{
			return (Node)(_out_offsets[node + 1] - _out_offsets[node]);
		}

		Node in_degree(Node node) const
		{
			return (Node)(_in_offsets[node + 1] - _in_offsets[node]);
		}

		Node bi_degree(Node node) const
		{
			return _bi_degrees[node];
		}

		neighbor_container out_neighbors(Node node) const
		{
			return neighbor_container(_out_nbrs + _out_offsets[node], _out_nbrs + _out_offsets[node + 1]);
		}

		neighbor_container in_neighbors(Node node) const
		{
			return neighbor_container(_in_nbrs + _in_offsets[node], _in_nbrs + _in_offsets[node + 1]);
		}

		neighbor_container bi_neighbors(Node node) const
		{
			return neighbor_container(_out_nbrs + _out_offsets[node], _out_nbrs + _out_offsets[node] + _bi_degrees[node]);
		}

		bool has_edge(Node out_node, Node in_node) const
		{
			if (out_degree(out_node) < in_degree(in_node))
			{
				Node *it1 = _out_nbrs + _out_offsets[out_node];
				Node *it2 = it1 + _bi_degrees[out_node];
				Node *it3 = _out_nbrs + _out_offsets[out_node + 1];
				return std::binary_search(it1, it2, in_node) || std::binary_search(it2, it3, in_node);
			}
			else
			{
				Node *it1 = _in_nbrs + _in_offsets[in_node];
				Node *it2 = it1 + _bi_degrees[in_node];
				Node *it3 = _in_nbrs + _in_offsets[in_node + 1];
				return std::binary_search(it1, it2, out_node) || std::binary_search(it2, it3, out_node);
			}
		}

		void reverse()
		{
			std::swap(_out_offsets, _in_offsets);
			std::swap(_out_nbrs, _in_nbrs);
		}

		template <class OStream> bool save(OStream &ostream, bool compress) const
		{
			if (compress)
			{
				int header = 1;
				if (ostream.write(&header, 1) != 1) return false;
				return _save_compressed(ostream);
			}
			else
			{
				int header = 0;
				if (ostream.write(&header, 1) != 1) return false;
				return _save_binary(ostream);
			}
		}

		template <class IStream> bool load(IStream &istream)
		{
			int header;
			if (istream.read(&header, 1) != 1) return false;
			bool compress = ((header & 1) != 0);
			return compress ? _load_compressed(istream) : _load_binary(istream);
		}

		template <class Node_InIt>
		void build(Node node_num, long long edge_num, Node_InIt out_first, Node_InIt in_first)
		{
			_release();
			_alloc(node_num, edge_num);

			std::fill(_out_offsets, _out_offsets + _node_num, 0);
			std::fill(_in_offsets, _in_offsets + _node_num, 0);
			Node_InIt out_it = out_first;
			for (long long i = 0; i < edge_num; ++i)
			{
				++_out_offsets[*out_it];
				++out_it;
			}
			for (Node i = 1; i < node_num; ++i)
			{
				_out_offsets[i] += _out_offsets[i - 1];
			}
			_out_offsets[node_num] = edge_num;

			out_it = out_first;
			Node_InIt in_it = in_first;
			for (long long i = 0; i < edge_num; ++i)
			{
				_out_nbrs[--_out_offsets[*out_it]] = *in_it;
				++_in_offsets[*in_it];
				++out_it;
				++in_it;
			}

			for (Node i = 1; i < node_num; ++i)
			{
				_in_offsets[i] += _in_offsets[i - 1];
			}
			_in_offsets[node_num] = edge_num;

			for (Node i = node_num - 1; i >= 0; --i)
			{
				for (long long k = _out_offsets[i + 1] - 1; k >= _out_offsets[i]; --k)
				{
					Node j = _out_nbrs[k];
					_in_nbrs[--_in_offsets[j]] = i;
				}
			}

			for (Node i = 0; i < _node_num; ++i)
			{
				_out_offsets[i] = _out_offsets[i + 1];
			}

			for (Node i = node_num - 1; i >= 0; --i)
			{
				for (long long k = _in_offsets[i + 1] - 1; k >= _in_offsets[i]; --k)
				{
					Node j = _in_nbrs[k];
					_out_nbrs[--_out_offsets[j]] = i;
				}
			}

			char *tags = new char[node_num];
			std::fill(tags, tags + node_num, 0);
			Node *nbrs = new Node[node_num];
			for (Node i = 0; i < node_num; ++i)
			{
				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					tags[_out_nbrs[k]] |= 1;
				}
				for (long long k = _in_offsets[i]; k < _in_offsets[i + 1]; ++k)
				{
					tags[_in_nbrs[k]] |= 2;
				}

				Node offset = 0;
				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					Node j = _out_nbrs[k];
					if (tags[j] == 3) nbrs[offset++] = j;
				}
				_bi_degrees[i] = offset;
				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					Node j = _out_nbrs[k];
					if (tags[j] == 1) nbrs[offset++] = j;
				}
				std::copy(nbrs, nbrs + offset, _out_nbrs + _out_offsets[i]);

				offset = _bi_degrees[i];
				for (long long k = _in_offsets[i]; k < _in_offsets[i + 1]; ++k)
				{
					Node j = _in_nbrs[k];
					if (tags[j] == 2) nbrs[offset++] = j;
				}
				std::copy(nbrs, nbrs + offset, _in_nbrs + _in_offsets[i]);

				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					tags[_out_nbrs[k]] = 0;
				}
				for (long long k = _in_offsets[i] + _bi_degrees[i]; k < _in_offsets[i + 1]; ++k)
				{
					tags[_in_nbrs[k]] = 0;
				}
			}
			delete[] nbrs;
			delete[] tags;
		}

	protected:
		Node _node_num;
		long long _edge_num;
		long long *_out_offsets, *_in_offsets;
		Node *_bi_degrees;
		Node *_out_nbrs, *_in_nbrs;

		template <class Pointer> inline void _delete(Pointer &ptr)
		{
			if (ptr != NULL)
			{
				delete[] ptr;
				ptr = NULL;
			}
		}

		void _release()
		{
			_delete(_out_offsets);
			_delete(_in_offsets);
			_delete(_bi_degrees);
			_delete(_out_nbrs);
			_delete(_in_nbrs);
			_node_num = 0;
			_edge_num = 0;
		}

		void _alloc(Node node_num, long long edge_num)
		{
			_node_num = node_num;
			_edge_num = edge_num;
			_out_offsets = new long long[_node_num + 1];
			_in_offsets = new long long[_node_num + 1];
			_bi_degrees = new Node[_node_num];
			_out_nbrs = new Node[_edge_num];
			_in_nbrs = new Node[_edge_num];
		}

		void *_compress_increasing(void *compressed, Node *first, Node *last) const
		{
			if (first == last) return compressed;

			unsigned char *ptr = (unsigned char *)compressed;
			Node prev = *first++;
			*((Node *)ptr) = prev;
			ptr += sizeof(Node);

			while (first != last)
			{
				Node curr = *first++;
				Node delta = curr - prev;
				while (delta >= 128)
				{
					*ptr++ = (unsigned char)(delta & 127);
					delta >>= 7;
				}
				*ptr++ = (unsigned char)(delta | 128);
				prev = curr;
			}

			return (void *)ptr;
		}

		void *_compress_neighbors(void *compressed, Node *neighbors, long long *offsets, Node *bi_degrees) const
		{
			for (Node i = 0; i < _node_num; ++i)
			{
				Node *first = neighbors + offsets[i];
				Node *second = first + bi_degrees[i];
				Node *last = neighbors + offsets[i + 1];
				compressed = _compress_increasing(compressed, first, second);
				compressed = _compress_increasing(compressed, second, last);
			}
			return compressed;
		}

		void *_decompress_increasing(void *compressed, Node *first, Node *last)
		{
			if (first == last) return compressed;

			unsigned char *ptr = (unsigned char *)compressed;
			Node prev = *((Node *)ptr);
			*first++ = prev;
			ptr += sizeof(Node);

			while (first != last)
			{
				Node delta = 0;
				int offset = 0;
				while ((*ptr & 128) == 0)
				{
					delta += (*ptr << offset);
					++ptr;
					offset += 7;
				}
				delta += (*ptr & 127) << offset;
				++ptr;

				Node curr = prev + delta;
				*first++ = curr;
				prev = curr;
			}

			return (void *)ptr;
		}

		void *_decompress_neighbors(void *compressed, Node *neighbors, long long *offsets, Node *bi_degrees)
		{
			for (Node i = 0; i < _node_num; ++i)
			{
				Node *first = neighbors + offsets[i];
				Node *second = first + bi_degrees[i];
				Node *last = neighbors + offsets[i + 1];
				compressed = _decompress_increasing(compressed, first, second);
				compressed = _decompress_increasing(compressed, second, last);
			}
			return compressed;
		}

		template <class OStream> bool _save_binary(OStream &ostream) const
		{
			if (ostream.write(&_node_num, 1) != 1) return false;
			if (ostream.write(&_edge_num, 1) != 1) return false;
			if (ostream.write(_out_offsets, _node_num + 1) != _node_num + 1) return false;
			if (ostream.write(_in_offsets, _node_num + 1) != _node_num + 1) return false;
			if (ostream.write(_bi_degrees, _node_num) != _node_num) return false;
			if (ostream.write(_out_nbrs, _edge_num) != _edge_num) return false;
			if (ostream.write(_in_nbrs, _edge_num) != _edge_num) return false;
			return true;
		}

		template <class OStream> bool _save_compressed(OStream &ostream) const
		{
			if (ostream.write(&_node_num, 1) != 1) return false;
			if (ostream.write(&_edge_num, 1) != 1) return false;
			
			Node *degrees = new Node[_node_num];
			for (Node i = 0; i < _node_num; ++i)
			{
				degrees[i] = (Node)(_out_offsets[i + 1] - _out_offsets[i]);
			}
			if (ostream.write(degrees, _node_num) != _node_num) return false;
			for (Node i = 0; i < _node_num; ++i)
			{
				degrees[i] = (Node)(_in_offsets[i + 1] - _in_offsets[i]);
			}
			if (ostream.write(degrees, _node_num) != _node_num) return false;
			delete[] degrees;

			if (ostream.write(_bi_degrees, _node_num) != _node_num) return false;

			char *buffer = new char[_edge_num * sizeof(Node)];
			
			long long count = (char *)_compress_neighbors(buffer, _out_nbrs, _out_offsets, _bi_degrees) - buffer;
			if (ostream.write(&count, 1) != 1) return false;
			if (ostream.write(buffer, count) != count) return false;

			count = (char *)_compress_neighbors(buffer, _in_nbrs, _in_offsets, _bi_degrees) - buffer;
			if (ostream.write(&count, 1) != 1) return false;
			if (ostream.write(buffer, count) != count) return false;

			delete[] buffer;

			return true;
		}

		template <class IStream> bool _load_binary(IStream &istream)
		{
			_release();

			if (istream.read(&_node_num, 1) != 1) return false;
			if (istream.read(&_edge_num, 1) != 1) return false;

			_alloc(_node_num, _edge_num);

			if (istream.read(_out_offsets, _node_num + 1) != _node_num + 1) return false;
			if (istream.read(_in_offsets, _node_num + 1) != _node_num + 1) return false;
			if (istream.read(_bi_degrees, _node_num) != _node_num) return false;
			if (istream.read(_out_nbrs, _edge_num) != _edge_num) return false;
			if (istream.read(_in_nbrs, _edge_num) != _edge_num) return false;
			return true;
		}

		template <class IStream> bool _load_compressed(IStream &istream)
		{
			_release();

			if (istream.read(&_node_num, 1) != 1) return false;
			if (istream.read(&_edge_num, 1) != 1) return false;

			_alloc(_node_num, _edge_num);

			Node *degrees = _bi_degrees;
			if (istream.read(degrees, _node_num) != _node_num) return false;
			_out_offsets[0] = 0;
			for (Node i = 0; i < _node_num; ++i)
			{
				_out_offsets[i + 1] = _out_offsets[i] + degrees[i];
			}
			if (istream.read(degrees, _node_num) != _node_num) return false;
			_in_offsets[0] = 0;
			for (Node i = 0; i < _node_num; ++i)
			{
				_in_offsets[i + 1] = _in_offsets[i] + degrees[i];
			}

			if (istream.read(_bi_degrees, _node_num) != _node_num) return false;

			long long buf_size;
			if (istream.read(&buf_size, 1) != 1) return false;
			char *buffer = new char[buf_size];
			if (istream.read(buffer, buf_size) != buf_size) return false;
			_decompress_neighbors(buffer, _out_nbrs, _out_offsets, _bi_degrees);
			delete[] buffer;

			if (istream.read(&buf_size, 1) != 1) return false;
			buffer = new char[buf_size];
			if (istream.read(buffer, buf_size) != buf_size) return false;
			_decompress_neighbors(buffer, _in_nbrs, _in_offsets, _bi_degrees);
			delete[] buffer;

			return true;
		}
	};

	template <
		class Node,
		class EdgeAttr,
		class EdgeAttrContainer>
	class _directed_graph_base2
		: public _directed_graph_base3<Node>
	{
	public:
		class neighbor_edge_iterator
		{
		public:
			neighbor_edge_iterator(Node *nodes, typename EdgeAttrContainer::iterator attrs) : _nodes(nodes), _attrs(attrs) { }
			
			friend bool operator==(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes == b._nodes;
			}

			friend bool operator!=(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes != b._nodes;
			}

			friend bool operator<(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes < b._nodes;
			}

			friend bool operator>(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes > b._nodes;
			}

			friend bool operator<=(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes <= b._nodes;
			}

			friend bool operator>=(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes >= b._nodes;
			}

			neighbor_edge_iterator &operator++()
			{
				++_nodes;
				++_attrs;
				return *this;
			}

			neighbor_edge_iterator operator++(int)
			{
				neighbor_edge_iterator tmp(*this);
				++_nodes;
				++_attrs;
				return tmp;
			}

			neighbor_edge_iterator &operator--()
			{
				--_nodes;
				--_attrs;
				return *this;
			}

			neighbor_edge_iterator operator--(int)
			{
				neighbor_edge_iterator tmp(*this);
				--_nodes;
				--_attrs;
				return tmp;
			}

			const std::pair<Node, const EdgeAttr> *operator->() const
			{
				return &(const std::pair<Node, const EdgeAttr>)std::make_pair(*_nodes, *_attrs);
			}

			const std::pair<Node, const EdgeAttr> operator*() const
			{
				return std::make_pair<Node, const EdgeAttr>(*_nodes, *_attrs);
			}

			const std::pair<Node, const EdgeAttr> operator[](long long index) const
			{
				return std::make_pair<Node, const EdgeAttr>(_nodes[index], _attrs[index]);
			}

			neighbor_edge_iterator &operator+=(long long offset)
			{
				_nodes += offset;
				_attrs += offset;
				return *this;
			}

			neighbor_edge_iterator &operator-=(long long offset)
			{
				_nodes -= offset;
				_attrs += offset;
				return *this;
			}

			friend neighbor_edge_iterator operator+(const neighbor_edge_iterator &a, long long offset)
			{
				return neighbor_edge_iterator(a._nodes + offset, a._attrs + offset);
			}

			friend neighbor_edge_iterator operator+(long long offset, const neighbor_edge_iterator &a)
			{
				return neighbor_edge_iterator(a._nodes + offset, a._attrs + offset);
			}

			friend neighbor_edge_iterator operator-(const neighbor_edge_iterator &a, long long offset)
			{
				return neighbor_edge_iterator(a._nodes - offset, a._attrs - offset);
			}

			friend long long operator-(const neighbor_edge_iterator &a, const neighbor_edge_iterator &b)
			{
				return a._nodes - b._nodes;
			}
			
		protected:
			Node *_nodes;
			typename EdgeAttrContainer::iterator _attrs;
		};

		class neighbor_edge_container
		{
		public:
			neighbor_edge_container(neighbor_edge_iterator first, neighbor_edge_iterator last) : _first(first), _last(last) { }

			neighbor_edge_iterator begin() const
			{
				return _first;
			}

			neighbor_edge_iterator end() const
			{
				return _last;
			}

			Node operator[](size_t index) const
			{
				return _first[index];
			}

			size_t size() const
			{
				return _last - _first;
			}

		protected:
			neighbor_edge_iterator _first, _last;
		};

		_directed_graph_base2()
		{
			_out_attrs = NULL;
			_in_attrs = NULL;
		}

		_directed_graph_base2(const _directed_graph_base2 &other) : _directed_graph_base3(other)
		{
			if (other._node_num == 0)
			{
				_out_attrs = NULL;
				_in_attrs = NULL;
			}
			else
			{
				_out_attrs = new EdgeAttrContainer();
				_in_attrs = new EdgeAttrContainer();
				_out_attrs->load(other._out_attrs->begin(), other._out_attrs->end());
				_in_attrs->load(other._in_attrs->begin(), other._in_attrs->end());
			}
		}

		~_directed_graph_base2()
		{
			_release();
		}

		void swap(_directed_graph_base2 &other)
		{
			_directed_graph_base3::swap(other);
			std::swap(_out_attrs, other._out_attrs);
			std::swap(_in_attrs, other._in_attrs);
		}

		void reverse()
		{
			_directed_graph_base3::reverse();
			std::swap(_out_attrs, _in_attrs);
		}

		

		neighbor_edge_container out_neighbor_edges(Node node) const
		{
			long long offset1 = _out_offsets[node], offset2 = _out_offsets[node + 1];
			return neighbor_edge_container(
				neighbor_edge_iterator(_out_nbrs + offset1, _out_attrs->begin() + offset1),
				neighbor_edge_iterator(_out_nbrs + offset2, _out_attrs->begin() + offset2));
		}

		neighbor_edge_container in_neighbor_edges(Node node) const
		{
			long long offset1 = _in_offsets[node], offset2 = _in_offsets[node + 1];
			return neighbor_edge_container(
				neighbor_edge_iterator(_in_nbrs + offset1, _in_attrs->begin() + offset1),
				neighbor_edge_iterator(_in_nbrs + offset2, _in_attrs->begin() + offset2));
		}

		neighbor_edge_container bi_neighbor_edges(Node node) const
		{
			long long offset1 = _out_offsets[node], offset2 = _out_offsets[node] + _bi_degrees[node];
			return neighbor_edge_container(
				neighbor_edge_iterator(_out_nbrs + offset1, _out_attrs->begin() + offset1),
				neighbor_edge_iterator(_out_nbrs + offset2, _out_attrs->begin() + offset2));
		}

		EdgeAttrContainer &out_attrs()
		{
			return *_out_attrs;
		}

		EdgeAttrContainer &in_attrs()
		{
			return *_in_attrs;
		}

		template <class Node_InIt, class EdgeAttr_InIt> 
		void build(Node node_num, long long edge_num, Node_InIt out_first, Node_InIt in_first, EdgeAttr_InIt edge_first)
		{
			_directed_graph_base3::_release();
			_release();

			_directed_graph_base3::_alloc(node_num, edge_num);
			_alloc(node_num, edge_num);

			EdgeAttr *out_attrs = new EdgeAttr[edge_num];
			EdgeAttr *in_attrs = new EdgeAttr[edge_num];

			std::fill(_out_offsets, _out_offsets + _node_num, 0);
			std::fill(_in_offsets, _in_offsets + _node_num, 0);
			Node_InIt out_it = out_first;
			for (long long i = 0; i < edge_num; ++i)
			{
				++_out_offsets[*out_it];
				++out_it;
			}
			for (Node i = 1; i < node_num; ++i)
			{
				_out_offsets[i] += _out_offsets[i - 1];
			}
			_out_offsets[node_num] = edge_num;

			out_it = out_first;
			for (long long i = 0; i < edge_num; ++i)
			{
				long long out_offset = --_out_offsets[*out_it];
				_out_nbrs[out_offset] = *in_first;
				out_attrs[out_offset] = *edge_first;
				++_in_offsets[*in_first];
				++out_it;
				++in_first;
				++edge_first;
			}

			for (Node i = 1; i < node_num; ++i)
			{
				_in_offsets[i] += _in_offsets[i - 1];
			}
			_in_offsets[node_num] = edge_num;

			for (Node i = node_num - 1; i >= 0; --i)
			{
				for (long long k = _out_offsets[i + 1] - 1; k >= _out_offsets[i]; --k)
				{
					Node j = _out_nbrs[k];
					long long in_offset = --_in_offsets[j];
					_in_nbrs[in_offset] = i;
					in_attrs[in_offset] = out_attrs[k];
				}
			}

			for (Node i = 0; i < _node_num; ++i)
			{
				_out_offsets[i] = _out_offsets[i + 1];
			}

			for (Node i = node_num - 1; i >= 0; --i)
			{
				for (long long k = _in_offsets[i + 1] - 1; k >= _in_offsets[i]; --k)
				{
					Node j = _in_nbrs[k];
					long long out_offset = --_out_offsets[j];
					_out_nbrs[out_offset] = i;
					out_attrs[out_offset] = in_attrs[k];
				}
			}

			char *tags = new char[node_num];
			std::fill(tags, tags + node_num, 0);
			Node *nbrs = new Node[node_num];
			EdgeAttr *attrs = new EdgeAttr[node_num];
			for (Node i = 0; i < node_num; ++i)
			{
				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					tags[_out_nbrs[k]] |= 1;
				}
				for (long long k = _in_offsets[i]; k < _in_offsets[i + 1]; ++k)
				{
					tags[_in_nbrs[k]] |= 2;
				}

				Node offset = 0;
				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					Node j = _out_nbrs[k];
					if (tags[j] == 3)
					{
						nbrs[offset] = j;
						attrs[offset] = out_attrs[k];
						++offset;
					}
				}
				_bi_degrees[i] = offset;
				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					Node j = _out_nbrs[k];
					if (tags[j] == 1)
					{
						nbrs[offset] = j;
						attrs[offset] = out_attrs[k];
						++offset;
					}
				}
				std::copy(nbrs, nbrs + offset, _out_nbrs + _out_offsets[i]);

				offset = 0;
				for (long long k = _in_offsets[i]; k < _in_offsets[i + 1]; ++k)
				{
					Node j = _in_nbrs[k];
					if (tags[j] == 3)
					{
						nbrs[offset] = j;
						attrs[offset] = in_attrs[k];
						++offset;
					}
				}
				for (long long k = _in_offsets[i]; k < _in_offsets[i + 1]; ++k)
				{
					Node j = _in_nbrs[k];
					if (tags[j] == 2)
					{
						nbrs[offset] = j;
						attrs[offset] = in_attrs[k];
						++offset;
					}
				}
				std::copy(nbrs, nbrs + offset, _in_nbrs + _in_offsets[i]);

				for (long long k = _out_offsets[i]; k < _out_offsets[i + 1]; ++k)
				{
					tags[_out_nbrs[k]] = 0;
				}
				for (long long k = _in_offsets[i] + _bi_degrees[i]; k < _in_offsets[i + 1]; ++k)
				{
					tags[_in_nbrs[k]] = 0;
				}
			}
			delete[] attrs;
			delete[] nbrs;
			delete[] tags;

			_out_attrs->load(out_attrs, out_attrs + edge_num);
			_in_attrs->load(in_attrs, in_attrs + edge_num);
		}

		template <class OStream> bool save(OStream &ostream, bool compress) const
		{
			if (!_directed_graph_base3::save(ostream, compress)) return false;
			if (!_out_attrs->save(ostream)) return false;
			if (!_in_attrs->save(ostream)) return false;
			return true;
		}

		template <class IStream> bool load(IStream &istream)
		{
			_release();
			_alloc(_node_num, _edge_num);

			if (!_directed_graph_base3::load(istream)) return false;

			if (!_out_attrs->load(istream)) return false;
			if (!_in_attrs->load(istream)) return false;
			return true;
		}

		edge_container<Node, EdgeAttr, EdgeAttrContainer> out_edges()
		{
			edge_iterator<Node, EdgeAttr, EdgeAttrContainer> first(0, 0, _node_num, _out_offsets, _out_nbrs, _out_attrs.begin(), false);
			edge_iterator<Node, EdgeAttr, EdgeAttrContainer> last(_edge_num, _node_num, _node_num, _out_offsets, _out_nbrs, _out_attrs.begin(), false);
			return edge_container<Node, EdgeAttr, EdgeAttrContainer>(first, last);
		}

		edge_container<Node, EdgeAttr, EdgeAttrContainer> in_edges()
		{
			edge_iterator<Node, EdgeAttr, EdgeAttrContainer> first(0, 0, _node_num, _in_offsets, _in_nbrs, _in_attrs.begin(), true);
			edge_iterator<Node, EdgeAttr, EdgeAttrContainer> last(_edge_num, _node_num, _node_num, _in_offsets, _in_nbrs, _in_attrs.begin(), true);
			return edge_container<Node, EdgeAttr, EdgeAttrContainer>(first, last);
		}

	protected:
		EdgeAttrContainer *_out_attrs, *_in_attrs;

		void _release()
		{
			if (_out_attrs != NULL)
			{
				delete _out_attrs;
				_out_attrs = NULL;
			}
			if (_in_attrs != NULL)
			{
				delete _in_attrs;
				_in_attrs = NULL;
			}
		}

		void _alloc(Node node_num, long long edge_num)
		{
			_out_attrs = new EdgeAttrContainer();
			_in_attrs = new EdgeAttrContainer();
		}
	};

	template <
		class Node,
		class EdgeAttrContainer>
	class _directed_graph_base2<Node, void, EdgeAttrContainer>
		: public _directed_graph_base3<Node>
	{
	public:
		edge_container<Node, void, void> out_edges()
		{
			edge_iterator<Node, void, void> first(0, 0, _node_num, _out_offsets, _out_nbrs, false);
			edge_iterator<Node, void, void> last(_edge_num, _node_num, _node_num, _out_offsets, _out_nbrs, false);
			return edge_container<Node, void, void>(first, last);
		}

		edge_container<Node, void, void> in_edges()
		{
			edge_iterator<Node, void, void> first(0, 0, _node_num, _in_offsets, _in_nbrs, true);
			edge_iterator<Node, void, void> last(_edge_num, _node_num, _node_num, _in_offsets, _in_nbrs, true);
			return edge_container<Node, void, void>(first, last);
		}
	};

	template <
		class Node,
		class NodeAttr,
		class EdgeAttr,
		class NodeAttrContainer,
		class EdgeAttrContainer>
	class _directed_graph_base1
		: public _directed_graph_base2<Node, EdgeAttr, EdgeAttrContainer>
	{
	public:
		_directed_graph_base1()
		{
			_node_attrs = NULL;
		}

		_directed_graph_base1(const _directed_graph_base1 &other) : _directed_graph_base2(other)
		{
			if (other._node_num == 0)
			{
				_node_attrs = NULL;
			}
			else
			{
				_node_attrs = new NodeAttrContainer();
				_node_attrs->load(other._node_attrs->begin(), other._node_attrs->end());
			}
		}

		~_directed_graph_base1()
		{
			_release();
		}

		void swap(_directed_graph_base1 &other)
		{
			_directed_graph_base2::swap(other);
			std::swap(_node_attrs, other._node_attrs);
		}

		template <class Node_InIt, class NodeAttr_InIt, class EdgeAttr_InIt>
		void build(Node node_num, NodeAttr_InIt node_first, long long edge_num, Node_InIt out_first, Node_InIt in_first, EdgeAttr_InIt edge_first)
		{
			_release();
			_alloc(_node_num, _edge_num);

			_directed_graph_base2::build(node_num, edge_num, out_first, in_first, edge_first);
			_node_attrs->load(node_first, node_first + node_num);
		}

		template <class Node_InIt, class NodeAttr_InIt>
		void build(Node node_num, NodeAttr_InIt node_first, long long edge_num, Node_InIt out_first, Node_InIt in_first)
		{
			_release();
			_alloc(_node_num, _edge_num);

			_directed_graph_base2::build(node_num, edge_num, out_first, in_first);
			_node_attrs->load(node_first, node_first + node_num);
		}

		NodeAttr node_attr(Node node) const
		{
			return (*_node_attrs)[node];
		}

		NodeAttrContainer &node_attrs()
		{
			return *_node_attrs;
		}

		template <class OStream> bool save(OStream &ostream, bool compress) const
		{
			if (!_directed_graph_base2::save(ostream, compress)) return false;
			if (!_node_attrs->save(ostream)) return false;
			return true;
		}

		template <class IStream> bool load(IStream &istream)
		{
			_release();
			_alloc(_node_num, _edge_num);

			if (!_directed_graph_base2::load(istream)) return false;
			if (!_node_attrs->load(istream)) return false;
			return true;
		}

	protected:
		NodeAttrContainer *_node_attrs;

		void _release()
		{
			if (_node_attrs != NULL)
			{
				delete _node_attrs;
				_node_attrs = NULL;
			}
		}

		void _alloc(Node node_num, long long edge_num)
		{
			_node_attrs = new NodeAttrContainer();
		}
	};

	template <
		class Node,
		class EdgeAttr,
		class NodeAttrContainer,
		class EdgeAttrContainer>
	class _directed_graph_base1<Node, void, EdgeAttr, NodeAttrContainer, EdgeAttrContainer>
		: public _directed_graph_base2<Node, EdgeAttr, EdgeAttrContainer>
	{
	};

	template <
		class Node = int,
		class NodeAttr = void,
		class EdgeAttr = void,
		class NodeAttrContainer = default_container<NodeAttr>,
		class EdgeAttrContainer = default_container<EdgeAttr> >
	class directed_graph
		: public _directed_graph_base1<Node, NodeAttr, EdgeAttr, NodeAttrContainer, EdgeAttrContainer>
	{
	public:
		
		typedef Node node_t;
		typedef NodeAttr node_attr_t;
		typedef EdgeAttr edge_attr_t;
		typedef NodeAttrContainer node_attr_container_t;
		typedef EdgeAttrContainer edge_attr_container_t;

		directed_graph() { }

		directed_graph(const directed_graph &other) : _directed_graph_base1(other)
		{
			//printf("copy constructor\n");
		}

		directed_graph(directed_graph &&other)
		{
			//printf("move constructor\n");
			this->swap(other);
			memset(&other, 0, sizeof(other));
		}

		directed_graph &operator=(directed_graph other)
		{
			this->swap(other);
			return *this;
		}

		template <class OStream> bool save(OStream &ostream, bool compress = true) const
		{
			return _directed_graph_base1::save(ostream, compress);
		}

		template <class IStream> bool load(IStream &istream)
		{
			return _directed_graph_base1::load(istream);
		}
	};
}

namespace std
{
	template <class Node, class NodeAttr, class EdgeAttr, class NodeAttrContainer, class EdgeAttrContainer>
	void swap(
		graph::directed_graph<Node, NodeAttr, EdgeAttr, NodeAttrContainer, EdgeAttrContainer> &left,
		graph::directed_graph<Node, NodeAttr, EdgeAttr, NodeAttrContainer, EdgeAttrContainer> &right)
	{
		left.swap(right);
	}
}