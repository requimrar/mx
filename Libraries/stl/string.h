/*
 * Acess2 C++ Library
 * - By John Hodge (thePowersGang)
 *
 * string (header)
 * - C++'s String type
 */
#ifndef STL_STRING_H
#define STL_STRING_H

#include "allocator.h"
#include "initialiser_list.h"

namespace stl
{
	template <class charT>
	struct char_traits
	{
	};

	template <>
	struct char_traits<char>
	{
		typedef char	char_type;
		typedef int		int_type;

		static bool eq(const char_type& c, const char_type& d)
		{
			return c == d;
		}
		static bool lt(const char_type& c, const char_type& d)
		{
			return c < d;
		}
		static size_t length(const char_type* s)
		{
			size_t ret = 0;
			while(*s++)
				ret++;

			return ret;
		}
		static int compare(const char_type* p, const char_type* q, size_t n)
		{
	 		while(n--)
	 		{
				if(!eq(*p, *q))
				{
					return lt(*p, *q) ? -1 : 1;
				}

				++p; ++q;
			}

			return 0;
		}
	};

	template <>
	struct char_traits<wchar_t>
	{
		typedef wchar_t	char_type;
		typedef int	int_type;

		static size_t length(const char_type* s)
		{
			size_t ret = 0;
			while(*s++)
				ret++;

			return ret;
		}
		static bool eq(const char_type& c, const char_type& d)
		{
			return c == d;
		}
		static bool lt(const char_type& c, const char_type& d)
		{
			return c < d;
		}
	};

	// extern void _throw_out_of_range(const char* message);

	template <class charT, class traits = char_traits<charT>, class Alloc = allocator<charT>>
	class basic_string
	{
		public:
			typedef traits			traits_type;
			typedef Alloc			allocator_type;
			typedef charT			value_type;
			typedef typename		allocator_type::reference		reference;
			typedef typename		allocator_type::const_reference	const_reference;
			typedef size_t			size_type;

			typedef charT*			iterator;
			typedef const charT*	const_iterator;

		private:
			struct dynamic_info
			{
				allocator_type	m_allocator;
				int				m_ref_count = 1;
				size_type		m_capacity = 0;
				size_type		m_size = 0;

				typename allocator_type::pointer m_data = 0;

				dynamic_info(const allocator_type& alloc) : m_allocator(alloc)
				{
				}

				dynamic_info(const dynamic_info& other) : m_allocator(other.m_allocator), m_ref_count(1),
					m_capacity(other.m_capacity), m_size(other.m_size)
				{
					m_data = m_allocator.allocate(m_capacity);

					for(size_type i = 0; i < m_size; i++)
						m_data[i] = other.m_data[i];
				}
			};

			allocator_type	m_allocator;
			dynamic_info*	m_content;

		public:
			basic_string(const allocator_type& alloc = allocator_type()) : m_allocator(alloc), m_content(0)
			{
			}

			basic_string(const basic_string& str) : basic_string(allocator_type())
			{
				*this = str;
			}

			#if _CXX11_AVAIL
			basic_string(basic_string&& str) : m_allocator(str.m_allocator), m_content(str.m_content)
			{
				str.m_content = 0;
				// ::_sys::debug("basic_string(move) %p %s", m_content, c_str());
			}
			#endif


			basic_string(const basic_string& str, const allocator_type& alloc) : basic_string(str, 0, str.length(), alloc)
			{
			}

			basic_string(const basic_string& str, size_type pos, size_type len = npos,
				const allocator_type& alloc = allocator_type()) : basic_string(alloc)
			{
				if(pos < str.length())
				{
					if(len > str.length() - pos)
						len = str.length() - pos;

					reserve(len);

					for(size_type i = 0; i < len; i++)
						m_content->m_data[i] = str.m_content->m_data[pos + i];

					m_content->m_size = len;
				}
			}

			basic_string(const charT* s, const allocator_type& alloc = allocator_type()) : basic_string(s, traits::length(s), alloc)
			{
			}

			basic_string(const charT* s, size_type n, const allocator_type& alloc = allocator_type()) : basic_string(alloc)
			{
				if(n > 0)
				{
					reserve(n);

					for(size_type i = 0; i < n; i++)
						m_content->m_data[i] = s[i];

					m_content->m_data[n] = 0;
					m_content->m_size = n;
				}
			}

			basic_string(size_type n, charT c, const allocator_type& alloc = allocator_type()) : basic_string(alloc)
			{
				if(n > 0)
				{
					reserve(n);
					for(size_type i = 0; i < n; i++)
						m_content->m_data[i] = c;

					m_content->m_data[n] = 0;
					m_content->m_size = n;
				}
			}

			#if __cplusplus < 199711L
			basic_string(basic_string&& str) noexcept : basic_string(allocator_type())
			{
			}

			basic_string(basic_string&& str, const allocator_type& alloc) : basic_string(alloc)
			{
				m_content = str.m_content;
				str.m_content = 0;
			}
			#endif

			~basic_string()
			{
				release_content();
			}

			basic_string& operator=(const basic_string& str)
			{
				return assign(str);
			}
			basic_string& operator=(const charT* s)
			{
				return assign(s);
			}
			basic_string& operator=(charT c)
			{
				return assign(c);
			}

			// iterators

			// capacity
			size_type size() const
			{
				return m_content ? m_content->m_size : 0;
			}

			size_type length() const
			{
				return size();
			}

			size_type max_size() const
			{
				return (size_type) -1;
			}

			void resize(size_type size, charT c = 0)
			{
				reserve(size);
				if(m_content->m_size < size)
				{
					for(size_type ofs = m_content->m_size; ofs < size; ofs++)
						m_content->m_data[ofs] = c;

					m_content->m_data[size] = 0;
				}

				m_content->m_size = size;
				m_content->m_data[size] = 0;
			}

			size_type capacity() const
			{
				return m_content ? m_content->m_capacity : 0;
			}

			void reserve(size_type size)
			{
				own_content();
				size = (size + 1 + 31) & ((size_type) ~31);
				if(size > m_content->m_capacity)
				{
					auto new_area = m_allocator.allocate(size);
					for(size_type i = 0; i < m_content->m_size; i++)
						new_area[i] = m_content->m_data[i];

					if(m_content->m_data && m_content->m_capacity > 0)
						m_allocator.deallocate(m_content->m_data, m_content->m_capacity);

					m_content->m_data = new_area;
					m_content->m_capacity = size;
				}
			}

			void clear()
			{
				own_content();
				m_content->m_size = 0;
			}

			bool empty() const
			{
				return length() == 0;
			}

			#if _CXX11_AVAIL
			void shrink_to_fit();
			#endif


			// Access
			reference operator[](size_type pos)
			{
				own_content();
				return m_content->m_data[pos];
			}

			const_reference operator[](size_type pos) const
			{
				return (m_content ? m_content->m_data[pos] : *(const charT*) 0);
			}

			reference at(size_type pos)
			{
				own_content();

				assert(pos < m_content->m_size);
				// if(pos >= m_content->m_size)
				// 	_throw_out_of_range("basic_string - at");

				return m_content->m_data[pos];
			}
			const_reference at(size_type pos) const
			{
				assert(m_content && pos < m_content->m_size);
				// if(!m_content || pos >= m_content->m_size)
				// 	_throw_out_of_range("basic_string - at");

				return m_content->m_data[pos];
			}


			#if _CXX11_AVAIL
			reference back()
			{
				own_content();
				return m_content->m_data[m_content->m_size];
			}

			const_reference back() const
			{
				return m_content->m_data[m_content->m_size];
			}

			reference front()
			{
				own_content();
				return m_content->m_data[0];
			}

			const_reference front() const
			{
				return m_content->m_data[0];
			}
			#endif



			// Modifiers
			basic_string& operator+=(const basic_string& str)
			{
				return append(str);
			}
			basic_string& operator+=(const charT* s)
			{
				return append(s);
			}
			basic_string& operator+=(charT c)
			{
				push_back(c);
				return *this;
			}

			basic_string& append(const basic_string& str)
			{
				return append(str, 0, npos);
			}

			basic_string& append(const basic_string& str, size_type subpos, size_type sublen)
			{
				if(str.size() == 0) return *this;

				// if(subpos >= str.size())
				// 	_throw_out_of_range("basic_string - assign source");

				assert(subpos < str.size());

				if(sublen > str.size() - subpos)
					sublen = str.size() - subpos;

				append(str.data() + subpos, sublen);
				return *this;
			}

			basic_string& append(const charT* s)
			{
				return append(s, traits::length(s));
			}

			basic_string& append(const charT* s, size_type n)
			{
				reserve(size() + n);
				for(size_type i = 0; i < n; i++)
					m_content->m_data[size() + i] = s[i];

				m_content->m_data[size()+n] = '\0';
				m_content->m_size += n;

				return *this;
			}

			basic_string& append(size_type n, charT c)
			{
				reserve(size() + n);
				for(size_type i = 0; i < n; i++)
					m_content->m_data[size() + i] = c;

				m_content->m_data[size()+n] = '\0';
				m_content->m_size += n;

				return *this;
			}

			void push_back(charT c)
			{
				append(1, c);
			}

			basic_string& assign(const basic_string& str)
			{
				// Special case, triggers copy-on-write.
				release_content();
				m_content = str.m_content;
				m_content->m_ref_count++;
				return *this;
			}

			basic_string& assign(const basic_string& str, size_type subpos, size_type sublen)
			{
				// if(subpos >= str.size())
				// 	_throw_out_of_range("basic_string - assign source");

				assert(subpos <= str.size());

				if(sublen > str.size() - subpos)
					sublen = str.size() - subpos;

				return assign(str.data() + subpos, sublen);
			}

			basic_string& assign(const charT* s)
			{
				return assign(s, traits::length(s));
			}
			basic_string& assign(const charT* s, size_type n)
			{
				release_content();
				reserve(n);
				for(size_type i = 0; i < n; i ++)
					m_content->m_data[i] = s[i];

				m_content->m_data[n] = '\0';
				m_content->m_size = n;

				return *this;
			}

			basic_string& assign(size_type n, charT c)
			{
				release_content();
				reserve(n);
				for(size_type i = 0; i < n; i++)
					m_content->m_data[i] = c;

				m_content->m_data[n] = '\0';
				m_content->m_size = n;
				return *this;
			}

			basic_string& insert(size_type pos, const basic_string& str);
			basic_string& insert(size_type pos, const basic_string& str, size_type subpos, size_type sublen);
			basic_string& insert(size_type pos, const charT& s);
			basic_string& insert(size_type pos, const charT& s, size_type n);
			basic_string& insert(size_type pos, size_type n, charT c);
			iterator insert(const_iterator p, size_type n, charT c);
			iterator insert(const_iterator p, charT c);

			template <class InputIterator>
			iterator insert(iterator p, InputIterator first, InputIterator last);

			#if _CXX11_AVAIL
			basic_string& insert(const_iterator p, initialiser_list<charT> il);
			#endif


			basic_string& erase(size_type pos = 0, size_type len = npos);
			iterator erase(const_iterator p);
			iterator erase(const_iterator first, const_iterator last);

			basic_string& replace(size_type pos, size_type len, const basic_string& str);
			basic_string& replace(const_iterator i1, const_reference i2, const basic_string& str);
			basic_string& replace(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen);
			basic_string& replace(size_type pos, size_type len, const charT *s);
			basic_string& replace(const_iterator i1, const_reference i2, const charT* s);
			basic_string& replace(size_type pos, size_type len, const charT *s, size_type n);
			basic_string& replace(const_iterator i1, const_reference i2, const charT* s, size_type n);
			basic_string& replace(size_type pos, size_type len, size_type n, charT c);
			basic_string& replace(const_iterator i1, const_reference i2, size_type n, charT c);
			template <class InputIterator>
			basic_string& replace(const_iterator i1, const_reference i2, InputIterator first, InputIterator last);
			basic_string& replace(const_iterator i1, const_iterator i2, initialiser_list<charT> il);

			void swap(basic_string& str)
			{
				auto tmp = m_content;
				m_content = str.m_content;
				str.m_content = tmp;
			}

			#if _CXX11_AVAIL
			void pop_back();
			#endif

			// String operations
			const charT* c_str() const
			{
				// TODO: this is const, but also might need to do processing
				if(m_content)
				{
					_libcxx_assert(m_content->m_data[m_content->m_size] == '\0');
				}

				return (m_content ? m_content->m_data : "");
			}

			const charT* data() const
			{
				return (m_content ? m_content->m_data : NULL);
			}

			allocator_type get_allocator() const
			{
				return m_allocator;
			}

			size_type copy(charT* s, size_type len, size_type pos = 0) const;

			size_type find(const basic_string& str, size_type pos = 0) const;
			size_type find(const charT* s, size_type pos = 0) const;
			size_type find(const charT* s, size_type pos, size_type n) const;
			size_type find(charT c, size_type pos = 0) const noexcept;

			size_type rfind(const basic_string& str, size_type pos = 0) const;
			size_type rfind(const charT* s, size_type pos = 0) const;
			size_type rfind(const charT* s, size_type pos, size_type n) const;
			size_type rfind(charT c, size_type pos = 0) const noexcept;

			size_type find_first_of(const basic_string& str, size_type pos = 0) const;
			size_type find_first_of(const charT* s, size_type pos = 0) const;
			size_type find_first_of(const charT* s, size_type pos, size_type n) const;
			size_type find_first_of(charT c, size_type pos = 0) const noexcept;

			size_type find_last_of(const basic_string& str, size_type pos = 0) const;
			size_type find_last_of(const charT* s, size_type pos = 0) const;
			size_type find_last_of(const charT* s, size_type pos, size_type n) const;
			size_type find_last_of(charT c, size_type pos = 0) const noexcept;

			size_type find_first_not_of(const basic_string& str, size_type pos = 0) const;
			size_type find_first_not_of(const charT* s, size_type pos = 0) const;
			size_type find_first_not_of(const charT* s, size_type pos, size_type n) const;
			size_type find_first_not_of(charT c, size_type pos = 0) const noexcept;

			size_type find_last_not_of(const basic_string& str, size_type pos = 0) const;
			size_type find_last_not_of(const charT* s, size_type pos = 0) const;
			size_type find_last_not_of(const charT* s, size_type pos, size_type n) const;
			size_type find_last_not_of(charT c, size_type pos = 0) const noexcept;

			basic_string substr(size_type pos = 0, size_type len = npos) const;

			int compare(const basic_string& str) const
			{
				return compare(0, size(), str.data(), str.size());
			}

			int compare(size_type pos, size_type len, const basic_string& str) const
			{
				_libcxx_assert(pos <= size());
				_libcxx_assert(len <= size());
				_libcxx_assert(pos+len <= size());
				return compare(pos, len, str.data(), str.size());
			}

			int compare(size_type pos, size_type len, const basic_string& str, size_type subpos, size_type sublen) const
			{
				// TODO: check
				_libcxx_assert(subpos <= str.size());
				_libcxx_assert(sublen <= str.size());
				_libcxx_assert(subpos+sublen <= str.size());
				return compare(pos, len, str.data()+subpos, sublen);
			}

			int compare(const charT* s) const
			{
				return compare(0, npos, s, traits::length(s));
			}

			int compare(size_type pos, size_type len, const charT* s) const
			{
				return compare(pos, len, s, traits::length(s));
			}

			int compare(size_type pos, size_type len, const charT* s, size_type n) const
			{
				if(n <= len)
				{
					int rv = traits::compare(data() + pos, s, n);
					if(rv == 0 && n < len)
					{
						rv = -1;
					}
					return rv;
				}
				else
				{
					int rv = traits::compare(data() + pos, s, len);
					if(rv == 0)
					{
						rv = 1;
					}
					return rv;
				}
			}

			static const size_type npos = (size_type) -1;


		private:
			void own_content()
			{
				if(!m_content)
				{
					m_content = new dynamic_info(m_allocator);
				}
				else if(m_content->m_ref_count > 1)
				{
					dynamic_info* new_cont = new dynamic_info(*m_content);
					m_content->m_ref_count--;
					m_content = new_cont;
				}
				else
				{
					// already owned
				}
			}
			void release_content()
			{
				if(m_content)
				{
					m_content->m_ref_count--;
					if(m_content->m_ref_count == 0)
					{
						m_allocator.deallocate(m_content->m_data, m_content->m_capacity);
						delete m_content;
					}

					m_content = NULL;
				}
			}
	};

	typedef basic_string<char> string;

	#define _libcxx_str	basic_string<charT, traits, Alloc>

	template <class charT, class traits, class Alloc>
	basic_string<charT, traits, Alloc> operator+(const basic_string<charT, traits, Alloc>& lhs, const basic_string<charT, traits, Alloc>& rhs)
	{
		basic_string<charT, traits, Alloc> ret;
		ret.reserve(lhs.size() + rhs.size());
		ret += lhs;
		ret += rhs;
		return ret;
	}

	template <class charT, class traits, class Alloc>
	basic_string<charT, traits, Alloc> operator+(const basic_string<charT, traits ,Alloc>& lhs, const charT* rhs)
	{
		basic_string<charT, traits, Alloc> ret;
		ret.reserve(lhs.size() + traits::length(rhs));
		ret += lhs;
		ret += rhs;
		return ret;
	}
	template <class charT, class traits, class Alloc>
	basic_string<charT, traits, Alloc> operator+(const charT* lhs, const basic_string<charT, traits, Alloc>& rhs)
	{
		basic_string<charT, traits, Alloc> ret;
		ret.reserve(traits::length(lhs) + rhs.size());
		ret += lhs;
		ret += rhs;
		return ret;
	}
	template <class charT, class traits, class Alloc>
	basic_string<charT, traits, Alloc> operator+(const basic_string<charT, traits, Alloc>& lhs, const charT rhs)
	{
		basic_string<charT, traits, Alloc> ret;
		ret.reserve(lhs.size() + 1);
		ret += lhs;
		ret += rhs;
		return ret;
	}
	template <class charT, class traits, class Alloc>
	basic_string<charT, traits, Alloc> operator+(const charT lhs, const basic_string<charT, traits, Alloc>& rhs)
	{
		basic_string<charT, traits, Alloc> ret;
		ret.reserve(1 + rhs.size());
		ret += lhs;
		ret += rhs;
		return ret;
	}


	// Three overloads each
	// name: Actual operator, opp: reversed operator
	#define _libcxx_string_def_cmp(name, opp) \
		template <class charT, class traits, class Alloc> \
		bool operator name(const _libcxx_str& lhs, const _libcxx_str& rhs) { return lhs.compare(rhs) name 0; } \
		template <class charT, class traits, class Alloc> \
		bool operator name(const charT* lhs, const _libcxx_str& rhs) { return rhs.compare(lhs) opp 0; } \
		template <class charT, class traits, class Alloc> \
		bool operator name(const _libcxx_str& lhs, const charT* rhs) { return lhs.compare(rhs) name 0; }

	_libcxx_string_def_cmp(<, >)
	_libcxx_string_def_cmp(<=, >=)
	_libcxx_string_def_cmp(==, ==)
	_libcxx_string_def_cmp(>=, <=)
	_libcxx_string_def_cmp(>, <)
}

#endif










