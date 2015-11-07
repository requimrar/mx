/*
 * Acess2 C++ Library
 * - By John Hodge (thePowersGang)
 *
 * string (header)
 * - C++'s String type
 */
#ifndef STL_UTILITY_H
#define STL_UTILITY_H

#include "_libcxx_helpers.h"
#include "type_traits.h"

namespace stl
{
	template <class T1, class T2>
	class pair
	{
	public:
		typedef T1 first_type;
		typedef T2 second_type;

		first_type first;
		second_type second;

		pair()
		{
		}

		template <class U, class V>
		pair(const pair<U,V>& pr): first (pr.first), second(pr.second)
		{
		}

		pair(const first_type& a, const second_type& b) : first (a), second(b)
		{
		}

		pair(const pair& pr): first(pr.first), second(pr.second)
		{
		}

		pair(pair&& pr): first(pr.first), second(pr.second)
		{
		}

		// operator = is implicit
		pair& operator=(const pair& x)
		{
			first = x.first;
			second = x.second;
			return *this;
		}
	};


	template <class T1, class T2>
	bool operator==(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}

	template <class T1, class T2>
	bool operator!= (const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return !(lhs == rhs);
	}

	#if _CXX11_AVAIL
	template <class T>
	T&& forward(typename remove_reference<T>::type& arg)
	{
		return static_cast<decltype(arg)&&>(arg);
	}
	template <class T>
	T&& forward(typename remove_reference<T>::type&& arg)
	{
		return static_cast<decltype(arg)&&>(arg);
	}

	template <class T>
	typename remove_reference<T>::type&& move(T&& t)
	{
		return static_cast<typename remove_reference<T>::type&&>(t);
	}

	//template <class T>
	//constexpr typename ::std::remove_reference<T>::type&& move( T&& t) noexcept {
	//	return static_cast<typename ::std::remove_reference<T>::type&&>(t);
	//}
	#endif

}

#endif






