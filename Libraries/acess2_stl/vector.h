/*
 * Acess2 C++ Library
 * - By John Hodge (thePowersGang)
 *
 * vector (header)
 * - C++'s vector (dynamic array) type
 */

#ifndef _LIBCXX_VECTOR_
#define _LIBCXX_VECTOR_

#include "allocator.h"
#include "initialiser_list.h"

namespace astl
{
	namespace _bits
	{
		template <class VectorType, class T>
		class vector_iterator
		{
			friend VectorType;

			typedef typename VectorType::size_type size_type;
			typedef typename VectorType::difference_type difference_type;

			T*			m_array;
			size_type	m_pos;
			size_type	m_max;

		public:
			vector_iterator() : vector_iterator(0,0,0)
			{
			}
			vector_iterator(const vector_iterator& x) : vector_iterator()
			{
				*this = x;
			}
			vector_iterator(T* array, size_type start, size_type max) : m_array(array), m_pos(start), m_max(max)
			{
			}
			vector_iterator& operator=(const vector_iterator& x)
			{
				m_array = x.m_array;
				m_pos = x.m_pos;
				m_max = x.m_max;
				return *this;
			}

			bool operator==(const vector_iterator& other) const
			{
				return m_pos == other.m_pos;
			}
			bool operator!=(const vector_iterator& other) const
			{
				return !(*this == other);
			}
			T& operator*() const
			{
				return m_array[m_pos];
			}
			T& operator->() const
			{
				return m_array[m_pos];
			}
			T& operator[](difference_type n)
			{
				return *(*this + n);
			}
			vector_iterator& operator++()
			{
				if(m_pos < m_max)
				{
					m_pos++;
				}

				return *this;
			}
			const vector_iterator operator++(int)
			{
				vector_iterator ret(*this);
				++*this;
				return ret;
			}
			vector_iterator& operator--()
			{
				if(m_pos > 0)
				{
					m_pos--;
				}

				return *this;
			}
			const vector_iterator operator--(int)
			{
				vector_iterator ret(*this);
				--*this;
				return ret;
			}


			vector_iterator& operator+=(difference_type n)
			{
				if(n < 0)
					return (*this -= -n);

				if(n > 0)
					m_pos = (m_pos + n < m_max ? m_pos + n : m_max);

				return *this;
			}
			vector_iterator& operator-=(difference_type n)
			{
				if(n < 0)
					return (*this += -n);

				if(n > 0)
					m_pos = (m_pos >= n ? m_pos - n : 0);

				return *this;
			}

			const difference_type operator-(const vector_iterator& it2) const
			{
				// _libcxx_assert(m_array == it2.m_array);
				return m_pos - it2.m_pos;
			}

			bool operator<(const vector_iterator& o) const { return m_pos < o.m_pos; }
			bool operator>(const vector_iterator& o) const { return m_pos > o.m_pos; }
			bool operator<=(const vector_iterator& o) const { return m_pos <= o.m_pos; }
			bool operator>=(const vector_iterator& o) const { return m_pos >= o.m_pos; }
		};

		#define vector_iterator_tpl class VectorType, class T
		#define vector_iterator	vector_iterator<VectorType, T>

		template <vector_iterator_tpl>
		const vector_iterator operator+(const vector_iterator& it, typename VectorType::difference_type n)
		{
			return vector_iterator(it) += n;
		}
		template <vector_iterator_tpl>
		const vector_iterator operator+(typename VectorType::difference_type n, const vector_iterator& it)
		{
			return vector_iterator(it) += n;
		}
		template <vector_iterator_tpl>
		const vector_iterator operator-(const vector_iterator& it, typename VectorType::difference_type n)
		{
			return vector_iterator(it) -= n;
		}

		#undef vector_iterator_tpl
		#undef vector_iterator
	}









	template <class T, class Alloc = allocator<T> >
	class vector
	{
	public:
		typedef T												value_type;
		typedef Alloc											allocator_type;
		typedef typename allocator_type::reference				reference;
		typedef typename allocator_type::const_reference		const_reference;
		typedef typename allocator_type::pointer				pointer;
		typedef typename allocator_type::const_pointer			const_pointer;
		typedef int												difference_type;
		typedef size_t											size_type;
		typedef ::astl::_bits::vector_iterator<vector,T>		iterator;
		typedef ::astl::_bits::vector_iterator<vector,const T>	const_iterator;

	private:
		allocator_type	m_alloc;
		size_type		m_size;
		size_type		m_capacity;
		value_type*		m_data;

	public:
		vector(const allocator_type& alloc = allocator_type()) : m_alloc(alloc), m_size(0), m_capacity(0), m_data(0)
		{
		}
		vector(size_type n, const value_type& val = value_type(), const allocator_type& alloc = allocator_type()) : vector(alloc)
		{
			resize(n, val);
		}


		template <class InputIterator>
		vector(InputIterator first, InputIterator last, const allocator_type& alloc = allocator_type()) : vector(alloc)
		{
			insert(begin(), first, last);
		}

		vector(const vector& x) : vector(x.m_alloc)
		{
			*this = x;
		}


		#if _CXX11_AVAIL
		vector(vector&& x) : m_alloc(x.m_alloc), m_size(x.m_size), m_capacity(x.m_capacity), m_data(x.m_data)
		{
			x.m_data = nullptr;
			x.m_capacity = 0;
			x.m_size = 0;
		}

		vector(vector&& x, const allocator_type& alloc) : m_alloc(alloc), m_size(x.m_size), m_capacity(x.m_capacity), m_data(x.m_data)
		{
			x.m_data = nullptr;
			x.m_capacity = 0;
			x.m_size = 0;
		}

		vector(astl::initialiser_list<value_type> il, const allocator_type& alloc = allocator_type()) : vector(alloc)
		{
			reserve(il.size());
			insert(begin(), il.begin(), il.end());
		}
		#endif





		vector& operator=(const vector& x)
		{
			clear();
			m_alloc.deallocate(m_data, m_capacity);
			m_capacity = 0;
			m_data = nullptr;

			reserve(x.size());
			for(size_type i = 0; i < x.size(); i++)
				push_back(x[i]);

			return *this;
		}

		~vector()
		{
			clear();
			m_alloc.deallocate(m_data, m_capacity);
			m_capacity = 0;
			m_data = nullptr;
		}

		// Iterators
		iterator       begin()		{ return iterator_to(0); }
		const_iterator begin() const{ return iterator_to(0); }
		iterator       end()		{ return iterator_to(m_size); }
		const_iterator end() const	{ return iterator_to(m_size); }

		// Capacity
		size_type size() const
		{
			return m_size;
		}

		size_type max_size() const
		{
			return -1 / sizeof(value_type);
		}

		void resize(size_type new_cap, value_type val = value_type())
		{
			reserve(new_cap);
			if(new_cap > m_size)
			{
				for(size_type i = m_size; i < new_cap; i++)
				{
					m_alloc.construct(&m_data[i], val);
				}
			}
			else
			{
				for(size_type i = new_cap; i < m_size; i++)
					m_alloc.destroy(&m_data[i]);
			}

			m_size = new_cap;
		}

		size_type capacity() const
		{
			return m_capacity;
		}

		bool empty() const
		{
			return m_size == 0;
		}

		void reserve(size_type n)
		{
			assert(n <= max_size());

			if(n > m_capacity)
			{
				size_type size = (n + 0x1F) & ((size_type) ~0x1F);

				auto new_area = m_alloc.allocate(size);
				for(size_type i = 0; i < m_size; i++)
					new_area[i] = m_data[i];

				m_alloc.deallocate(m_data, m_capacity);
				m_data = new_area;
				m_capacity = size;

				//::_SysDebug("::std::vector::resize - m_capacity=%i for n=%i", m_capacity, n);
			}
		}

		void shrink_to_fit()
		{
		}

		// Element access
		reference operator[](size_type n)
		{
			return m_data[n];
		}

		const_reference operator[](size_type n) const
		{
			return m_data[n];
		}

		reference at(size_type n)
		{
			assert(n < size());

			// if(n > size())
			// 	_throw_out_of_range("::std::vector - at");

			return m_data[n];
		}

		const_reference at(size_type n) const
		{
			assert(n < size());

			// if(n > size())
			// 	_throw_out_of_range("::std::vector - at");

			return m_data[n];
		}
		reference front()
		{
			return m_data[0];
		}
		const_reference front() const
		{
			return m_data[0];
		}

		reference back()
		{
			return m_data[size() - 1];
		}

		const_reference back() const
		{
			return m_data[size() - 1];
		}

		pointer data() noexcept
		{
			return m_data;
		}

		const_pointer data() const noexcept
		{
			return m_data;
		}


		// Modifiers
		void assign(size_type n, const value_type& val)
		{
			clear();
			resize(n, val);
		}
		void push_back(const value_type& val)
		{
			resize(size()+1, val);
		}

		void pop_back()
		{
			if(!empty())
			{
				resize(size() - 1);
			}
		}

		iterator insert(iterator position, const value_type& val)
		{
			insert(position, 1, val);
			return iterator_to(position.m_pos);
		}

		void insert(iterator position, size_type n, const value_type& val)
		{
			reserve(m_size + n);
			if(position != end())
			{
				// ::_sys::debug("TODO: vector::insert within vector (%i!=%i)",
				// 	position-begin(), end()-begin());
				// ::_sys::abort();

				assert(0);
			}

			size_type pos = m_size;
			while(n--)
			{
				//::_sys::debug("vector::insert - %x at %i", val, pos);
				m_alloc.construct( &m_data[pos], val );
				pos++;
				m_size++;
			}
		}

		template <class InputIterator>
		void insert(iterator position, InputIterator first, InputIterator last)
		{
			InputIterator it = first;
			size_type len = 0;
			while(it != last)
			{
				++it;
				len++;
			}

			reserve(m_size + len);

			it = first;
			while(it != last)
			{
				//::_sys::debug("vector::insert - to %i, from %p:%i",
				//	position.m_pos, it.m_array, it.m_pos);
				position = insert(position, *it) + 1;
				++it;
			}
		}

		iterator erase(iterator position);
		iterator erase(iterator first, iterator last);
		//void swap(vector& x) {
		//	::std::swap(m_size, x.m_size);
		//	::std::swap(m_capacity, x.m_capacity);
		//	::std::swap(m_data, x.m_data);
		//}

		void clear()
		{
			for(size_type i = 0; i < m_size; i++)
			{
				m_alloc.destroy( &m_data[i] );
			}
			m_size = 0;
		}

	private:
		iterator iterator_to(size_type index)
		{
			_libcxx_assert(index <= m_size);
			return iterator(m_data, index, m_size);
		}
		const_iterator iterator_to(size_type index) const
		{
			_libcxx_assert(index <= m_size);
			return const_iterator(m_data, index, m_size);
		}
	};

}

#endif








