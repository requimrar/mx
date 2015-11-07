/*
 * Acess2 C++ Library
 * - By John Hodge (thePowersGang)
 *
 * type_traits (header)
 * - C++11 type traits
 */
#ifndef STL_TYPE_TRAITS_H
#define STL_TYPE_TRAITS_H

#include "_libcxx_helpers.h"

#if !_CXX11_AVAIL
# error "This header requires C++11 support enabled"
#endif

template <class T> struct remove_reference		{ typedef T	type; };
template <class T> struct remove_reference<T&>	{ typedef T	type; };
template <class T> struct remove_reference<T&&>	{ typedef T	type; };

#endif

// vim: ft=cpp
