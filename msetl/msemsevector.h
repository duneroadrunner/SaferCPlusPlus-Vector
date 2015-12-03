
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSEMSEVECTOR_H
#define MSEMSEVECTOR_H

#include "mseprimitives.h"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>

namespace mse {

#ifndef _NOEXCEPT
#define _NOEXCEPT
#endif /*_NOEXCEPT*/

#ifndef _XSTD
#define _XSTD ::std::
#endif /*_XSTD*/

	template<class _Ty, _Ty _Val>
	struct integral_constant
	{	// convenient template for integral constant types
	static const _Ty value = _Val;

	typedef _Ty value_type;
	typedef integral_constant<_Ty, _Val> type;

	operator value_type() const
		{	// return stored value
		return (value);
		}
	};

	template<class _Iter>
	struct _mse_Is_iterator
	: public integral_constant<bool, !std::is_integral<_Iter>::value>
	{	// tests for reasonable iterator candidate
	};
#ifndef MSVC2010_COMPATIBILE
	template<typename _InIter>
	using _mse_RequireInputIter = typename
	  std::enable_if<std::is_convertible<typename
	  std::iterator_traits<_InIter>::iterator_category,
	  std::input_iterator_tag>::value>::type;
#endif /*MSVC2010_COMPATIBILE*/

	/* Note that, at the moment, msevector inherits publicly from std::vector. This is not intended to be a permanent
		characteristic of msevector and any reference to, or interpretation of, an msevector as an std::vector is (and has
		always been) depricated. msevector endeavors to support the subset of the std::vector interface that is compatible
		with the security/safety goals of msevector. (The remaining part of the std::vector interface may be supported, as a
		user option, for compatibility.)
		In particular, keep in mind that std::vector does not have a virtual destructor, so deallocating an msevector as an
		std::vector would result in memory leaks. */
	template<class _Ty, class _A = std::allocator<_Ty> >
	class msevector : public std::vector<_Ty, _A> {
	public:
		typedef std::vector<_Ty, _A> base_class;
		typedef msevector<_Ty, _A> _Myt;

		explicit msevector(const _A& _Al = _A())
			: base_class(_Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		explicit msevector(size_t _N, const _Ty& _V = _Ty(), const _A& _Al = _A())
			: base_class(_N, _V, _Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		msevector(base_class&& _X) : base_class(std::move(_X)), m_mmitset(*this) { m_debug_size = size(); }
		msevector(const base_class& _X) : base_class(_X), m_mmitset(*this) { m_debug_size = size(); }
		msevector(_Myt&& _X) : base_class(std::move(_X)), m_mmitset(*this) { m_debug_size = size(); }
		msevector(const _Myt& _X) : base_class(_X), m_mmitset(*this) { m_debug_size = size(); }
		typedef typename base_class::const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe typename base_class::iterator and/or pointer parameters. */
		msevector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { m_debug_size = size(); }
		msevector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { m_debug_size = size(); }
		template<class _Iter
#ifndef MSVC2010_COMPATIBILE
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter>
#endif /*MSVC2010_COMPATIBILE*/
		>
		msevector(_Iter _First, _Iter _Last) : base_class(_First, _Last), m_mmitset(*this) { m_debug_size = size(); }
		template<class _Iter
#ifndef MSVC2010_COMPATIBILE
			//, class = typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter>
#endif /*MSVC2010_COMPATIBILE*/
		>
		//msevector(_Iter _First, _Iter _Last, const typename base_class::_Alloc& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { m_debug_size = size(); }
		msevector(_Iter _First, _Iter _Last, const _A& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { m_debug_size = size(); }
		_Myt& operator=(const base_class& _X) {
			base_class::operator =(_X);
			m_debug_size = size();
			m_mmitset.reset();
			return (*this);
		}
		_Myt& operator=(_Myt&& _X) {
			operator=(std::move(static_cast<base_class&>(_X)));
			m_mmitset.reset();
			return (*this);
		}
		_Myt& operator=(const _Myt& _X) {
			operator=((base_class)_X);
			m_mmitset.reset();
			return (*this);
		}
		void reserve(size_t _Count)
		{	// determine new minimum length of allocated storage
			auto original_capacity = CSize_t((*this).capacity());

			base_class::reserve(_Count);

			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		void shrink_to_fit() {	// reduce capacity
			auto original_capacity = CSize_t((*this).capacity());

			base_class::shrink_to_fit();

			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		void resize(size_t _N, const _Ty& _X = _Ty()) {
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());
			bool shrinking = (_N < original_size);

			base_class::resize(_N, _X);
			m_debug_size = size();

			if (shrinking) {
				m_mmitset.invalidate_inclusive_range(_N, CSize_t(original_size - 1));
			}
			m_mmitset.shift_inclusive_range(original_size, original_size, CSize_t(_N) - original_size); /*shift the end markers*/
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		typename base_class::const_reference operator[](size_t _P) const {
			return at(_P);
		}
		typename base_class::reference operator[](size_t _P) {
			return at(_P);
		}
		typename base_class::reference front() {	// return first element of mutable sequence
			if (0 == size()) { throw(std::out_of_range("front() on empty - typename base_class::reference front() - msevector")); }
			return base_class::front();
		}
		typename base_class::const_reference front() const {	// return first element of nonmutable sequence
			if (0 == size()) { throw(std::out_of_range("front() on empty - typename base_class::const_reference front() - msevector")); }
			return base_class::front();
		}
		typename base_class::reference back() {	// return last element of mutable sequence
			if (0 == size()) { throw(std::out_of_range("back() on empty - typename base_class::reference back() - msevector")); }
			return base_class::back();
		}
		typename base_class::const_reference back() const {	// return last element of nonmutable sequence
			if (0 == size()) { throw(std::out_of_range("back() on empty - typename base_class::const_reference back() - msevector")); }
			return base_class::back();
		}
		void push_back(_Ty&& _X) {
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			base_class::push_back(std::move(_X));
			m_debug_size = size();

			assert((original_size + 1) == CSize_t((*this).size()));
			m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		void push_back(const _Ty& _X) {
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			base_class::push_back(_X);
			m_debug_size = size();

			assert((original_size + 1) == CSize_t((*this).size()));
			m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		void pop_back() {
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			if (0 == original_size) { throw(std::out_of_range("pop_back() on empty - void pop_back() - msevector")); }
			base_class::pop_back();
			m_debug_size = size();

			assert((original_size - 1) == CSize_t((*this).size()));
			m_mmitset.invalidate_inclusive_range(CSize_t(original_size - 1), CSize_t(original_size - 1));
			m_mmitset.shift_inclusive_range(original_size, original_size, -1); /*shift the end markers*/
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		void assign(_It _F, _It _L) {
			base_class::assign(_F, _L);
			m_debug_size = size();
			m_mmitset.reset();
		}
		template<class _Iter>
		void assign(_Iter _First, _Iter _Last) {	// assign [_First, _Last)
			base_class::assign(_First, _Last);
			m_debug_size = size();
			m_mmitset.reset();
		}
		void assign(size_t _N, const _Ty& _X = _Ty()) {
			base_class::assign(_N, _X);
			m_debug_size = size();
			m_mmitset.reset();
		}
		typename base_class::iterator insert(typename base_class::iterator _P, _Ty&& _X) {
			return (emplace(_P, std::move(_X)));
		}
		typename base_class::iterator insert(typename base_class::iterator _P, const _Ty& _X = _Ty()) {
			CInt di = std::distance(base_class::begin(), _P);
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			typename base_class::iterator retval = base_class::insert(_P, _X);
			m_debug_size = size();

			assert((original_size + 1) == CSize_t((*this).size()));
			assert(d == std::distance(base_class::begin(), retval));
			m_mmitset.shift_inclusive_range(d, original_size, 1);
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
			return retval;
		}

#if !(defined(GPP4P8_COMPATIBILE))
#ifndef MSVC2010_COMPATIBILE
		typename base_class::iterator
#else /*MSVC2010_COMPATIBILE*/
		void
#endif /*MSVC2010_COMPATIBILE*/
			insert(typename base_class::const_iterator _P, size_t _M, const _Ty& _X) {
				CInt di = std::distance(base_class::cbegin(), _P);
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

#ifndef MSVC2010_COMPATIBILE
			typename base_class::iterator retval =
#endif /*MSVC2010_COMPATIBILE*/
				base_class::insert(_P, _M, _X);
			m_debug_size = size();

			assert((original_size + _M) == CSize_t((*this).size()));
#ifndef MSVC2010_COMPATIBILE
			assert(d == std::distance(base_class::begin(), retval));
#endif /*MSVC2010_COMPATIBILE*/
			m_mmitset.shift_inclusive_range(d, original_size, _M);
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
#ifndef MSVC2010_COMPATIBILE
			return retval;
#endif /*MSVC2010_COMPATIBILE*/
		}
		template<class _Iter
#ifndef MSVC2010_COMPATIBILE
		//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, typename base_class::iterator>::type
		, class = _mse_RequireInputIter<_Iter> > typename base_class::iterator
#else /*MSVC2010_COMPATIBILE*/
		>typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
#endif /*MSVC2010_COMPATIBILE*/
			insert(typename base_class::const_iterator _Where, _Iter _First, _Iter _Last) {	// insert [_First, _Last) at _Where
				CInt di = std::distance(base_class::cbegin(), _Where);
				CSize_t d(di);
				if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

				auto _M = CInt(std::distance(_First, _Last));
				auto original_size = CSize_t((*this).size());
				auto original_capacity = CSize_t((*this).capacity());

				//if (0 > _M) { throw(std::out_of_range("invalid argument - typename base_class::iterator insert() - msevector")); }
#ifndef MSVC2010_COMPATIBILE
				auto retval = 
#endif /*MSVC2010_COMPATIBILE*/
					base_class::insert(_Where, _First, _Last);
				m_debug_size = size();

				assert((original_size + _M) == CSize_t((*this).size()));
#ifndef MSVC2010_COMPATIBILE
				assert(d == std::distance(base_class::begin(), retval));
#endif /*MSVC2010_COMPATIBILE*/
				m_mmitset.shift_inclusive_range(d, original_size, _M);
				auto new_capacity = CSize_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
#ifndef MSVC2010_COMPATIBILE
				return retval;
#endif /*MSVC2010_COMPATIBILE*/
			}

#else /*!(defined(GPP4P8_COMPATIBILE))*/

		/*typename base_class::iterator*/
		void
			/* g++4.8 seems to be using the c++98 version of this insert function instead of the c++11 version. */
			insert(typename base_class::/*const_*/iterator _P, size_t _M, const _Ty& _X) {
				CInt di = std::distance(base_class::/*c*/begin(), _P);
				CSize_t d(di);
				if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

				auto original_size = CSize_t((*this).size());
				auto original_capacity = CSize_t((*this).capacity());

				/*typename base_class::iterator retval =*/
					base_class::insert(_P, _M, _X);
				m_debug_size = size();

				assert((original_size + _M) == CSize_t((*this).size()));
				/*assert(d == std::distance(base_class::begin(), retval));*/
				m_mmitset.shift_inclusive_range(d, original_size, _M);
				auto new_capacity = CSize_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
				/*return retval;*/
			}
		template<class _Iter
			//>typename std::enable_if<_mse_Is_iterator<_Iter>::value, void>::type
			, class = _mse_RequireInputIter<_Iter> > void
		insert(typename base_class::/*const_*/iterator _Where, _Iter _First, _Iter _Last) {	// insert [_First, _Last) at _Where
				CInt di = std::distance(base_class::/*c*/begin(), _Where);
				CSize_t d(di);
				if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

				auto _M = CInt(std::distance(_First, _Last));
				auto original_size = CSize_t((*this).size());
				auto original_capacity = CSize_t((*this).capacity());

				//if (0 > _M) { throw(std::out_of_range("invalid argument - typename base_class::iterator insert() - msevector")); }
				/*auto retval =*/
					base_class::insert(_Where, _First, _Last);
				m_debug_size = size();

				assert((original_size + _M) == CSize_t((*this).size()));
				/*assert(d == std::distance(base_class::begin(), retval));*/
				m_mmitset.shift_inclusive_range(d, original_size, _M);
				auto new_capacity = CSize_t((*this).capacity());
				bool realloc_occured = (new_capacity != original_capacity);
				if (realloc_occured) {
					m_mmitset.sync_iterators_to_index();
				}
				/*return retval;*/
			}
#endif /*!(defined(GPP4P8_COMPATIBILE))*/

		template<class
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
			_Valty>
		void emplace_back(_Valty&&
#ifndef MSVC2010_COMPATIBILE
		...
#endif /*MSVC2010_COMPATIBILE*/
		_Val)
		{	// insert by moving into element at end
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			base_class::emplace_back(std::forward<_Valty>(_Val)
#ifndef MSVC2010_COMPATIBILE
				...
#endif /*MSVC2010_COMPATIBILE*/
				);
			m_debug_size = size();

			assert((original_size + 1) == CSize_t((*this).size()));
			m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		template<class
#ifndef MSVC2010_COMPATIBILE
			...
#endif /*MSVC2010_COMPATIBILE*/
			_Valty>
#if !(defined(GPP4P8_COMPATIBILE))
		typename base_class::iterator emplace(typename base_class::const_iterator _Where, _Valty&&
#ifndef MSVC2010_COMPATIBILE
		...
#endif /*MSVC2010_COMPATIBILE*/
		_Val)
		{	// insert by moving _Val at _Where
			CInt di = std::distance(base_class::cbegin(), _Where);
#else /*!(defined(GPP4P8_COMPATIBILE))*/
		typename base_class::iterator emplace(typename base_class::/*const_*/iterator _Where, _Valty&& ..._Val)
		{	// insert by moving _Val at _Where
			CInt di = std::distance(base_class::/*c*/begin(), _Where);
#endif /*!(defined(GPP4P8_COMPATIBILE))*/
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator emplace() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			auto retval = base_class::emplace(_Where, std::forward<_Valty>(_Val)
#ifndef MSVC2010_COMPATIBILE
				...
#endif /*MSVC2010_COMPATIBILE*/
				);
			m_debug_size = size();

			assert((original_size + 1) == CSize_t((*this).size()));
			assert(d == std::distance(base_class::begin(), retval));
			m_mmitset.shift_inclusive_range(d, original_size, 1);
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
			return retval;
		}
		typename base_class::iterator erase(typename base_class::iterator _P) {
			CInt di = std::distance(base_class::begin(), _P);
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator erase() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			if (end() == _P) { throw(std::out_of_range("invalid argument - typename base_class::iterator erase(typename base_class::iterator _P) - msevector")); }
			typename base_class::iterator retval = base_class::erase(_P);
			m_debug_size = size();

			assert((original_size - 1) == CSize_t((*this).size()));
			assert(d == std::distance(base_class::begin(), retval));
			{
				m_mmitset.invalidate_inclusive_range(d, d);
				m_mmitset.shift_inclusive_range(CSize_t(d + 1), original_size, -1);
			}
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
			return retval;
		}
		typename base_class::iterator erase(typename base_class::iterator _F, typename base_class::iterator _L) {
			CInt di = std::distance(base_class::begin(), _F);
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator erase() - msevector")); }
			CInt di2 = std::distance(base_class::begin(), _L);
			CSize_t d2(di2);
			if ((0 > di2) || (CSize_t((*this).size()) < di2)) { throw(std::out_of_range("index out of range - typename base_class::iterator erase() - msevector")); }

			auto _M = CInt(std::distance(_F, _L));
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			if ((end() == _F)/* || (0 > _M)*/) { throw(std::out_of_range("invalid argument - typename base_class::iterator erase(typename base_class::iterator _F, typename base_class::iterator _L) - msevector")); }
			typename base_class::iterator retval = base_class::erase(_F, _L);
			m_debug_size = size();

			assert((original_size - _M) == CSize_t((*this).size()));
			assert(d == std::distance(base_class::begin(), retval));
			{
				if (1 <= _M) {
					m_mmitset.invalidate_inclusive_range(d, CSize_t(d + _M - 1));
				}
				m_mmitset.shift_inclusive_range(CSize_t(d + _M), original_size, -_M);
			}
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
			return retval;
		}
		void clear() {
			base_class::clear();
			m_debug_size = size();
			m_mmitset.reset();
		}
		void swap(base_class& _X) {
			base_class::swap(_X);
			m_debug_size = size();
			m_mmitset.reset();
		}
		void swap(_Myt& _X) {
			swap(static_cast<base_class&>(_X));
			m_mmitset.reset();
		}

#ifndef MSVC2010_COMPATIBILE
		msevector(_XSTD initializer_list<typename base_class::value_type> _Ilist,
			const _A& _Al = _A())
			: base_class(_Ilist, _Al), m_mmitset(*this) {	// construct from initializer_list
			m_debug_size = size();
		}
		_Myt& operator=(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
			operator=((base_class)_Ilist);
			m_mmitset.reset();
			return (*this);
		}
		void assign(_XSTD initializer_list<typename base_class::value_type> _Ilist) {	// assign initializer_list
			base_class::assign(_Ilist);
			m_debug_size = size();
			m_mmitset.reset();
		}
#if defined(GPP4P8_COMPATIBILE)
		/* g++4.8 seems to be (incorrectly) using the c++98 version of this insert function instead of the c++11 version. */
		/*typename base_class::iterator*/void insert(typename base_class::/*const_*/iterator _Where, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
			CInt di = std::distance(base_class::/*c*/begin(), _Where);
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

			auto _M = _Ilist.size();
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			/*auto retval = */base_class::insert(_Where, _Ilist);
			m_debug_size = size();

			assert((original_size + _M) == CSize_t((*this).size()));
			/*assert(d == std::distance(base_class::begin(), retval));*/
			m_mmitset.shift_inclusive_range(d, original_size, _M);
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
			/*return retval;*/
		}
#else /*defined(GPP4P8_COMPATIBILE)*/
		typename base_class::iterator insert(typename base_class::const_iterator _Where, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
			CInt di = std::distance(base_class::cbegin(), _Where);
			CSize_t d(di);
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - typename base_class::iterator insert() - msevector")); }

			auto _M = _Ilist.size();
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			auto retval = base_class::insert(_Where, _Ilist);
			m_debug_size = size();

			assert((original_size + _M) == CSize_t((*this).size()));
			assert(d == std::distance(base_class::begin(), retval));
			m_mmitset.shift_inclusive_range(d, original_size, _M);
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
			return retval;
		}
#endif /*defined(GPP4P8_COMPATIBILE)*/
#endif /*MSVC2010_COMPATIBILE*/

		size_t m_debug_size;

		/* Note that, at the moment, mm_const_iterator_type inherits publicly from typename base_class::const_iterator. This is not intended to be a permanent
		characteristc of mm_const_iterator_type and any reference to, or interpretation of, an mm_const_iterator_type as an typename base_class::const_iterator is (and has
		always been) depricated. mm_const_iterator_type endeavors to support (and continue to support) the subset of the typename base_class::const_iterator
		interface that is compatible with the security/safety goals of mm_const_iterator_type.
		In particular, keep in mind that typename base_class::const_iterator does not have a virtual destructor, so deallocating an mm_const_iterator_type as an
		typename base_class::const_iterator would result in memory leaks. */
		class mm_const_iterator_type : public base_class::const_iterator {
		public:
			typedef typename base_class::const_iterator::iterator_category iterator_category;
			typedef typename base_class::const_iterator::value_type value_type;
			typedef typename base_class::const_iterator::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename base_class::const_iterator::pointer pointer;
			typedef typename base_class::const_iterator::pointer const_pointer;
			typedef typename base_class::const_iterator::reference reference;
			typedef typename base_class::const_reference const_reference;

			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if (m_points_to_an_item) { assert((1 <= m_owner_cref.size()) && (m_index < m_owner_cref.size())); return true; }
				else { assert(!((1 <= m_owner_cref.size()) && (m_index < m_owner_cref.size()))); return false; }
			}
			bool points_to_end_marker() const {
				if (false == points_to_an_item()) { assert(m_index == m_owner_cref.size()); return true; }
				else { return false; }
			}
			bool points_to_beginning() const {
				if (0 == m_index) { return true; }
				else { return false; }
			}
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return points_to_an_item(); } //his is
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return has_next_item_or_end_marker(); }
			bool has_previous() const { return (!points_to_beginning()); }
			void set_to_beginning() {
				m_index = 0;
				base_class::const_iterator::operator=(m_owner_cref.cbegin());
				if (1 <= m_owner_cref.size()) {
					m_points_to_an_item = true;
				}
				else { assert(false == m_points_to_an_item); }
			}
			void set_to_end_marker() {
				m_index = m_owner_cref.size();
				base_class::const_iterator::operator=(m_owner_cref.cend());
				m_points_to_an_item = false;
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					base_class::const_iterator::operator++();
					if (m_owner_cref.size() <= m_index) {
						(*this).m_points_to_an_item = false;
						if (m_owner_cref.size() < m_index) { assert(false); reset(); }
					}
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - void set_to_next() - mm_const_iterator_type - msevector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
					base_class::const_iterator::operator--();
					(*this).m_points_to_an_item = true;
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - void set_to_previous() - mm_const_iterator_type - msevector"));
				}
			}
			mm_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			mm_const_iterator_type operator++(int) { mm_const_iterator_type _Tmp = *this; ++*this; return (_Tmp); }
			mm_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			mm_const_iterator_type operator--(int) { mm_const_iterator_type _Tmp = *this; --*this; return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = CInt(m_index) + n;
				if ((0 > new_index) || (m_owner_cref.size() < new_index)) {
					throw(std::out_of_range("index out of range - void advance(difference_type n) - mm_const_iterator_type - msevector"));
				}
				else {
					m_index = new_index;
					base_class::const_iterator::operator+=(n);
					if (m_owner_cref.size() <= m_index) {
						(*this).m_points_to_an_item = false;
					}
					else {
						(*this).m_points_to_an_item = true;
					}
				}
			}
			void regress(difference_type n) { advance(-n); }
			mm_const_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			mm_const_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			mm_const_iterator_type operator+(difference_type n) const {
				mm_const_iterator_type retval(*this);
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			mm_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const mm_const_iterator_type &rhs) const {
				if ((&(rhs.m_owner_cref)) != (&((*this).m_owner_cref))) { throw(std::out_of_range("invalid argument - difference_type operator-(const mm_const_iterator_type &rhs) const - msevector::mm_const_iterator_type")); }
				auto retval = (static_cast<const typename base_class::const_iterator&>(*this) - static_cast<const typename base_class::const_iterator&>(rhs));
				assert((int)(m_owner_cref.size()) >= retval);
				return retval;
			}
			const_reference operator*() const {
				if (points_to_an_item()) {
					return m_owner_cref.at(mse::as_a_size_t(m_index));
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - const_reference operator*() const - mm_const_iterator_type - msevector"));
				}
			}
			const_reference item() const { return operator*(); }
			const_reference previous_item() const {
				if ((*this).has_previous()) {
					return m_owner_cref.at(m_index - 1);
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - const_reference previous_item() const - mm_const_iterator_type - msevector"));
				}
			}
			const_pointer operator->() const {
				if (points_to_an_item()) {
					const_cast<mm_const_iterator_type *>(this)->sync_const_iterator_to_index();
					//sync_const_iterator_to_index();
					return base_class::const_iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - pointer operator->() const - mm_const_iterator_type - msevector"));
				}
			}
			const_reference operator[](difference_type _Off) const { return (*(*this + _Off)); }
			/*
			mm_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref)
			{
			CInt d = std::distance<typename base_class::iterator>(m_owner_cref.cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cref.size() >= d)) {
			if (m_owner_cref.size() == d) {
			assert(m_owner_cref.cend() == _Right_cref);
			m_points_to_an_item = false;
			} else {
			m_points_to_an_item = true;
			}
			m_index = CSize_t(d);
			base_class::const_iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref) - mm_const_iterator_type - msevector"));
			}
			return (*this);
			}
			*/
			mm_const_iterator_type& operator=(const mm_const_iterator_type& _Right_cref)
			{
				if (&((*this).m_owner_cref) == &(_Right_cref.m_owner_cref)) {
					assert((*this).m_owner_cref.size() >= _Right_cref.m_index);
					(*this).m_points_to_an_item = _Right_cref.m_points_to_an_item;
					(*this).m_index = _Right_cref.m_index;
					base_class::const_iterator::operator=(_Right_cref);
				}
				else {
					throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_const_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - mm_const_iterator_type - msevector"));
				}
				return (*this);
			}
			bool operator==(const mm_const_iterator_type& _Right_cref) const { return (((&(_Right_cref.m_owner_cref)) == (&(m_owner_cref))) && (_Right_cref.m_index == m_index)); }
			bool operator!=(const mm_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const mm_const_iterator_type& _Right) const {
				if (&((*this).m_owner_cref) != &(_Right.m_owner_cref)) { throw(std::out_of_range("invalid argument - mm_const_iterator_type& operator<(const mm_const_iterator_type& _Right) - mm_const_iterator_type - msevector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const mm_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const mm_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const mm_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_const_item_pointer(const mm_const_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			void invalidate_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					(*this).reset();
				}
			}
			void shift_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last, mse::CInt shift) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					auto new_index = (*this).m_index + shift;
					if ((0 > new_index) || (m_owner_cref.size() < new_index)) {
						throw(std::out_of_range("void shift_inclusive_range() - mm_const_iterator_type - msevector"));
					}
					else {
						(*this).m_index = new_index;
						(*this).sync_const_iterator_to_index();
					}
				}
			}
			CSize_t position() const {
				return m_index;
			}
		private:
			mm_const_iterator_type(const _Myt& owner_cref) : m_owner_cref(owner_cref) { set_to_beginning(); }
			mm_const_iterator_type(const mm_const_iterator_type& src_cref) : m_owner_cref(src_cref.m_owner_cref) { (*this) = src_cref; }
			void sync_const_iterator_to_index() {
				assert(m_owner_cref.size() >= (*this).m_index);
				base_class::const_iterator::operator=(m_owner_cref.cbegin());
				base_class::const_iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CBool m_points_to_an_item;
			mse::CSize_t m_index;
			const _Myt& m_owner_cref;
			friend class mm_iterator_set_type;
			friend class /*_Myt*/msevector<_Ty, _A>;
			friend class mm_iterator_type;
		};
		/* Note that, at the moment, mm_iterator_type inherits publicly from base_class::iterator. This is not intended to be a permanent
		characteristc of mm_iterator_type and any reference to, or interpretation of, an mm_iterator_type as an base_class::iterator is (and has
		always been) depricated. mm_iterator_type endeavors to support (and continue to support) the subset of the base_class::iterator
		interface that is compatible with the security/safety goals of mm_iterator_type.
		In particular, keep in mind that base_class::iterator does not have a virtual destructor, so deallocating an mm_iterator_type as an
		base_class::iterator would result in memory leaks. */
		class mm_iterator_type : public base_class::iterator {
		public:
			typedef typename base_class::iterator::iterator_category iterator_category;
			typedef typename base_class::iterator::value_type value_type;
			typedef typename base_class::iterator::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename base_class::iterator::pointer pointer;
			typedef typename base_class::iterator::reference reference;

			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if (m_points_to_an_item) { assert((1 <= m_owner_ref.size()) && (m_index < m_owner_ref.size())); return true; }
				else { assert(!((1 <= m_owner_ref.size()) && (m_index < m_owner_ref.size())));  return false; }
			}
			bool points_to_end_marker() const {
				if (false == points_to_an_item()) { assert(m_index == m_owner_ref.size()); return true; }
				else { return false; }
			}
			bool points_to_beginning() const {
				if (0 == m_index) { return true; }
				else { return false; }
			}
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return points_to_an_item(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return has_next_item_or_end_marker(); }
			bool has_previous() const { return (!points_to_beginning()); }
			void set_to_beginning() {
				m_index = 0;
				base_class::iterator::operator=(m_owner_ref.begin());
				if (1 <= m_owner_ref.size()) {
					m_points_to_an_item = true;
				}
				else { assert(false == m_points_to_an_item); }
			}
			void set_to_end_marker() {
				m_index = m_owner_ref.size();
				base_class::iterator::operator=(m_owner_ref.end());
				m_points_to_an_item = false;
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					base_class::iterator::operator++();
					if (m_owner_ref.size() <= m_index) {
						(*this).m_points_to_an_item = false;
						if (m_owner_ref.size() < m_index) { assert(false); reset(); }
					}
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - void set_to_next() - mm_const_iterator_type - msevector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
					base_class::iterator::operator--();
					(*this).m_points_to_an_item = true;
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - void set_to_previous() - mm_iterator_type - msevector"));
				}
			}
			mm_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			mm_iterator_type operator++(int) { mm_iterator_type _Tmp = *this; ++*this; return (_Tmp); }
			mm_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			mm_iterator_type operator--(int) { mm_iterator_type _Tmp = *this; --*this; return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = CInt(m_index) + n;
				if ((0 > new_index) || (m_owner_ref.size() < new_index)) {
					throw(std::out_of_range("index out of range - void advance(difference_type n) - mm_iterator_type - msevector"));
				}
				else {
					m_index = new_index;
					base_class::iterator::operator+=(n);
					if (m_owner_ref.size() <= m_index) {
						(*this).m_points_to_an_item = false;
					}
					else {
						(*this).m_points_to_an_item = true;
					}
				}
			}
			void regress(int n) { advance(-n); }
			mm_iterator_type& operator +=(int n) { (*this).advance(n); return (*this); }
			mm_iterator_type& operator -=(int n) { (*this).regress(n); return (*this); }
			mm_iterator_type operator+(difference_type n) const {
				mm_iterator_type retval(*this);
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			mm_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const mm_iterator_type& rhs) const {
				if ((&rhs.m_owner_ref) != (&(*this).m_owner_ref)) { throw(std::out_of_range("invalid argument - difference_type operator-(const mm_iterator_type& rhs) const - msevector::mm_iterator_type")); }
				auto retval = (static_cast<const typename base_class::iterator&>(*this) - static_cast<const typename base_class::iterator&>(rhs));
				assert((int)(m_owner_ref.size()) >= retval);
				return retval;
			}
			reference operator*() {
				if (points_to_an_item()) {
					return m_owner_ref.at(mse::as_a_size_t(m_index));
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - reference operator*() - mm_iterator_type - msevector"));
				}
			}
			reference item() { return operator*(); }
			reference previous_item() {
				if ((*this).has_previous()) {
					return m_owner_ref.at(m_index - 1);
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - reference previous_item() - mm_const_iterator_type - msevector"));
				}
			}
			pointer operator->() {
				if (points_to_an_item()) {
					sync_iterator_to_index();
					return base_class::iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - pointer operator->() - mm_iterator_type - msevector"));
				}
			}
			reference operator[](difference_type _Off) { return (*(*this + _Off)); }
			/*
			mm_iterator_type& operator=(const typename base_class::iterator& _Right_cref)
			{
			CInt d = std::distance<typename base_class::iterator>(m_owner_ref.begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ref.size() >= d)) {
			if (m_owner_ref.size() == d) {
			assert(m_owner_ref.end() == _Right_cref);
			m_points_to_an_item = false;
			} else {
			m_points_to_an_item = true;
			}
			m_index = CSize_t(d);
			base_class::iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - mm_const_iterator_type - msevector"));
			}
			return (*this);
			}
			*/
			mm_iterator_type& operator=(const mm_iterator_type& _Right_cref)
			{
				if (&((*this).m_owner_ref) == &(_Right_cref.m_owner_ref)) {
					assert((*this).m_owner_ref.size() >= _Right_cref.m_index);
					(*this).m_points_to_an_item = _Right_cref.m_points_to_an_item;
					(*this).m_index = _Right_cref.m_index;
					base_class::iterator::operator=(_Right_cref);
				}
				else {
					throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - mm_const_iterator_type - msevector"));
				}
				return (*this);
			}
			bool operator==(const mm_iterator_type& _Right_cref) const { return ((_Right_cref.m_index == m_index) && (_Right_cref.m_owner_ref == m_owner_ref)); }
			bool operator!=(const mm_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const mm_iterator_type& _Right) const {
				if (&((*this).m_owner_ref) != &(_Right.m_owner_ref)) { throw(std::out_of_range("invalid argument - mm_iterator_type& operator<(const typename base_class::iterator& _Right) - mm_iterator_type - msevector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const mm_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const mm_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const mm_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_item_pointer(const mm_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			void invalidate_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					(*this).reset();
				}
			}
			void shift_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last, mse::CInt shift) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					auto new_index = (*this).m_index + shift;
					if ((0 > new_index) || (m_owner_ref.size() < new_index)) {
						throw(std::out_of_range("void shift_inclusive_range() - mm_iterator_type - msevector"));
					}
					else {
						(*this).m_index = new_index;
						(*this).sync_iterator_to_index();
					}
				}
			}
			CSize_t position() const {
				return m_index;
			}
			operator mm_const_iterator_type() const {
				mm_const_iterator_type retval(m_owner_ref);
				retval.set_to_beginning();
				retval.advance(mse::CInt(m_index));
				return retval;
			}
		private:
			mm_iterator_type(_Myt& owner_ref) : m_owner_ref(owner_ref) { set_to_beginning(); }
			mm_iterator_type(const mm_iterator_type& src_cref) : m_owner_ref(src_cref.m_owner_ref) { (*this) = src_cref; }
			void sync_iterator_to_index() {
				assert(m_owner_ref.size() >= (*this).m_index);
				base_class::iterator::operator=(m_owner_ref.begin());
				base_class::iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CBool m_points_to_an_item;
			mse::CSize_t m_index;
			_Myt& m_owner_ref;
			friend class mm_iterator_set_type;
			friend class /*_Myt*/msevector<_Ty, _A>;
		};

	private:
		typedef std::size_t CHashKey1;
		class mm_const_iterator_handle_type {
		public:
			mm_const_iterator_handle_type(const CHashKey1& key_cref, std::shared_ptr<mm_const_iterator_type>& shptr_cref) : m_shptr(shptr_cref), m_key(key_cref) {}
		private:
			std::shared_ptr<mm_const_iterator_type> m_shptr;
			CHashKey1 m_key;
			friend class /*_Myt*/msevector<_Ty, _A>;
			friend class mm_iterator_set_type;
		};
		class mm_iterator_handle_type {
		public:
			mm_iterator_handle_type(const CHashKey1& key_cref, std::shared_ptr<mm_iterator_type>& shptr_ref) : m_shptr(shptr_ref), m_key(key_cref) {}
		private:
			std::shared_ptr<mm_iterator_type> m_shptr;
			CHashKey1 m_key;
			friend class /*_Myt*/msevector<_Ty, _A>;
			friend class mm_iterator_set_type;
		};

		class mm_iterator_set_type {
		public:
			class CMMConstIterators : public std::unordered_map<CHashKey1, std::shared_ptr<mm_const_iterator_type>> {};
			class CMMIterators : public std::unordered_map<CHashKey1, std::shared_ptr<mm_iterator_type>> {};

			mm_iterator_set_type(_Myt& owner_ref) : m_owner_ref(owner_ref), m_next_available_key(0) {}
			void reset() {
				for (auto it = m_aux_mm_const_iterator_shptrs.begin(); m_aux_mm_const_iterator_shptrs.end() != it; it++) {
					(*it).second->reset();
				}
				for (auto it = m_aux_mm_iterator_shptrs.begin(); m_aux_mm_iterator_shptrs.end() != it; it++) {
					(*it).second->reset();
				}
			}
			void sync_iterators_to_index() {
				for (auto it = m_aux_mm_const_iterator_shptrs.begin(); m_aux_mm_const_iterator_shptrs.end() != it; it++) {
					(*it).second->sync_const_iterator_to_index();
				}
				for (auto it = m_aux_mm_iterator_shptrs.begin(); m_aux_mm_iterator_shptrs.end() != it; it++) {
					(*it).second->sync_iterator_to_index();
				}
			}
			void invalidate_inclusive_range(mse::CSize_t start_index, mse::CSize_t end_index) {
				for (auto it = m_aux_mm_const_iterator_shptrs.begin(); m_aux_mm_const_iterator_shptrs.end() != it; it++) {
					(*it).second->invalidate_inclusive_range(start_index, end_index);
				}
				for (auto it = m_aux_mm_iterator_shptrs.begin(); m_aux_mm_iterator_shptrs.end() != it; it++) {
					(*it).second->invalidate_inclusive_range(start_index, end_index);
				}
			}
			void shift_inclusive_range(mse::CSize_t start_index, mse::CSize_t end_index, mse::CInt shift) {
				for (auto it = m_aux_mm_const_iterator_shptrs.begin(); m_aux_mm_const_iterator_shptrs.end() != it; it++) {
					(*it).second->shift_inclusive_range(start_index, end_index, shift);
				}
				for (auto it = m_aux_mm_iterator_shptrs.begin(); m_aux_mm_iterator_shptrs.end() != it; it++) {
					(*it).second->shift_inclusive_range(start_index, end_index, shift);
				}
			}

			mm_const_iterator_handle_type allocate_new_const_item_pointer() {
				auto shptr = std::shared_ptr<mm_const_iterator_type>(new mm_const_iterator_type(m_owner_ref));
				auto key = m_next_available_key; m_next_available_key++;
				mm_const_iterator_handle_type retval(key, shptr);
				typename CMMConstIterators::value_type new_item(key, shptr);
				m_aux_mm_const_iterator_shptrs.insert(new_item);
				return retval;
			}
			void release_const_item_pointer(mm_const_iterator_handle_type handle) {
				auto it = m_aux_mm_const_iterator_shptrs.find(handle.m_key);
				if (m_aux_mm_const_iterator_shptrs.end() != it) {
					m_aux_mm_const_iterator_shptrs.erase(it);
				}
				else {
					/* Do we need to throw here? */
					throw(std::out_of_range("invalid handle - void release_aux_mm_const_iterator(mm_const_iterator_handle_type handle) - msevector::mm_iterator_set_type"));
				}
			}

			mm_iterator_handle_type allocate_new_item_pointer() {
				auto shptr = std::shared_ptr<mm_iterator_type>(new mm_iterator_type(m_owner_ref));
				auto key = m_next_available_key; m_next_available_key++;
				mm_iterator_handle_type retval(key, shptr);
				typename CMMIterators::value_type new_item(key, shptr);
				m_aux_mm_iterator_shptrs.insert(new_item);
				return retval;
			}
			void release_item_pointer(mm_iterator_handle_type handle) {
				auto it = m_aux_mm_iterator_shptrs.find(handle.m_key);
				if (m_aux_mm_iterator_shptrs.end() != it) {
					m_aux_mm_iterator_shptrs.erase(it);
				}
				else {
					/* Do we need to throw here? */
					throw(std::out_of_range("invalid handle - void release_aux_mm_iterator(mm_iterator_handle_type handle) - msevector::mm_iterator_set_type"));
				}
			}
			void release_all_item_pointers() {
				m_aux_mm_iterator_shptrs.clear();
			}
			mm_const_iterator_type &const_item_pointer(mm_const_iterator_handle_type handle) const {
				return (*(handle.m_shptr));
			}
			mm_iterator_type &item_pointer(mm_iterator_handle_type handle) {
				return (*(handle.m_shptr));
			}

		private:
			void release_all_const_item_pointers() {
				m_aux_mm_const_iterator_shptrs.clear();
			}
			CMMConstIterators m_aux_mm_const_iterator_shptrs;
			CMMIterators m_aux_mm_iterator_shptrs;
			CHashKey1 m_next_available_key;

			_Myt& m_owner_ref;

			friend class /*_Myt*/msevector<_Ty, _A>;
		};
		mutable mm_iterator_set_type m_mmitset;

	public:
		mm_const_iterator_type &const_item_pointer(mm_const_iterator_handle_type handle) const {
			return m_mmitset.const_item_pointer(handle);
		}
		mm_iterator_type &item_pointer(mm_iterator_handle_type handle) {
			return m_mmitset.item_pointer(handle);
		}

	private:
		mm_const_iterator_handle_type allocate_new_const_item_pointer() const { return m_mmitset.allocate_new_const_item_pointer(); }
		void release_const_item_pointer(mm_const_iterator_handle_type handle) const { m_mmitset.release_const_item_pointer(handle); }
		void release_all_const_item_pointers() const { m_mmitset.release_all_const_item_pointers(); }
		mm_iterator_handle_type allocate_new_item_pointer() const { return m_mmitset.allocate_new_item_pointer(); }
		void release_item_pointer(mm_iterator_handle_type handle) const { m_mmitset.release_item_pointer(handle); }
		void release_all_item_pointers() const { m_mmitset.release_all_item_pointers(); }

	public:
		class cipointer {
		public:
			typedef typename mm_const_iterator_type::iterator_category iterator_category;
			typedef typename mm_const_iterator_type::value_type value_type;
			typedef typename mm_const_iterator_type::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename mm_const_iterator_type::pointer pointer;
			typedef typename mm_const_iterator_type::const_pointer const_pointer;
			typedef typename mm_const_iterator_type::reference reference;
			typedef typename mm_const_iterator_type::const_reference const_reference;

			cipointer(const _Myt& owner_cref) : m_owner_cref(owner_cref) {
				mm_const_iterator_handle_type handle = m_owner_cref.allocate_new_const_item_pointer();
				m_handle_shptr = std::shared_ptr<mm_const_iterator_handle_type>(new mm_const_iterator_handle_type(handle));
			}
			cipointer(const cipointer& src_cref) : m_owner_cref(src_cref.m_owner_cref) {
				mm_const_iterator_handle_type handle = m_owner_cref.allocate_new_const_item_pointer();
				m_handle_shptr = std::shared_ptr<mm_const_iterator_handle_type>(new mm_const_iterator_handle_type(handle));
				const_item_pointer() = src_cref.const_item_pointer();
			}
			~cipointer() {
				m_owner_cref.release_const_item_pointer(*m_handle_shptr);
			}
			mm_const_iterator_type& const_item_pointer() const { return m_owner_cref.const_item_pointer(*m_handle_shptr); }
			mm_const_iterator_type& cip() const { return const_item_pointer(); }
			//const mm_const_iterator_handle_type& handle() const { return (*m_handle_shptr); }

			void reset() { const_item_pointer().reset(); }
			bool points_to_an_item() const { return const_item_pointer().points_to_an_item(); }
			bool points_to_end_marker() const { return const_item_pointer().points_to_end_marker(); }
			bool points_to_beginning() const { return const_item_pointer().points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return const_item_pointer().has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return const_item_pointer().has_next(); }
			bool has_previous() const { return const_item_pointer().has_previous(); }
			void set_to_beginning() { const_item_pointer().set_to_beginning(); }
			void set_to_end_marker() { const_item_pointer().set_to_end_marker(); }
			void set_to_next() { const_item_pointer().set_to_next(); }
			void set_to_previous() { const_item_pointer().set_to_previous(); }
			cipointer& operator ++() { const_item_pointer().operator ++(); return (*this); }
			cipointer operator++(int) { cipointer _Tmp = *this; ++*this; return (_Tmp); }
			cipointer& operator --() { const_item_pointer().operator --(); return (*this); }
			cipointer operator--(int) { cipointer _Tmp = *this; --*this; return (_Tmp); }
			void advance(difference_type n) { const_item_pointer().advance(n); }
			void regress(difference_type n) { const_item_pointer().regress(n); }
			cipointer& operator +=(int n) { const_item_pointer().operator +=(n); return (*this); }
			cipointer& operator -=(int n) { const_item_pointer().operator -=(n); return (*this); }
			cipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			cipointer operator-(int n) const { return ((*this) + (-n)); }
			difference_type operator-(const cipointer& _Right_cref) const { return const_item_pointer() - (_Right_cref.const_item_pointer()); }
			const_reference operator*() const { return const_item_pointer().operator*(); }
			const_reference item() const { return operator*(); }
			const_reference previous_item() const { return const_item_pointer().previous_item(); }
			const_pointer operator->() const { return const_item_pointer().operator->(); }
			const_reference operator[](difference_type _Off) const { return (*(*this + _Off)); }
			cipointer& operator=(const cipointer& _Right_cref) { const_item_pointer().operator=(_Right_cref.const_item_pointer()); return (*this); }
			bool operator==(const cipointer& _Right_cref) const { return const_item_pointer().operator==(_Right_cref.const_item_pointer()); }
			bool operator!=(const cipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const cipointer& _Right) const { return (const_item_pointer() < _Right.const_item_pointer()); }
			bool operator<=(const cipointer& _Right) const { return (const_item_pointer() <= _Right.const_item_pointer()); }
			bool operator>(const cipointer& _Right) const { return (const_item_pointer() > _Right.const_item_pointer()); }
			bool operator>=(const cipointer& _Right) const { return (const_item_pointer() >= _Right.const_item_pointer()); }
			void set_to_const_item_pointer(const cipointer& _Right_cref) { const_item_pointer().set_to_const_item_pointer(_Right_cref.const_item_pointer()); }
			CSize_t position() const { return const_item_pointer().position(); }
		private:
			const _Myt& m_owner_cref;
			std::shared_ptr<mm_const_iterator_handle_type> m_handle_shptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
		};
		class ipointer {
		public:
			typedef typename mm_iterator_type::iterator_category iterator_category;
			typedef typename mm_iterator_type::value_type value_type;
			typedef typename mm_iterator_type::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename mm_iterator_type::pointer pointer;
			typedef typename mm_iterator_type::reference reference;

			ipointer(_Myt& owner_ref) : m_owner_ref(owner_ref) {
				mm_iterator_handle_type handle = m_owner_ref.allocate_new_item_pointer();
				m_handle_shptr = std::shared_ptr<mm_iterator_handle_type>(new mm_iterator_handle_type(handle));
			}
			ipointer(const ipointer& src_cref) : m_owner_ref(src_cref.m_owner_ref) {
				mm_iterator_handle_type handle = m_owner_ref.allocate_new_item_pointer();
				m_handle_shptr = std::shared_ptr<mm_iterator_handle_type>(new mm_iterator_handle_type(handle));
				item_pointer() = src_cref.item_pointer();
			}
			~ipointer() {
				m_owner_ref.release_item_pointer(*m_handle_shptr);
			}
			mm_iterator_type& item_pointer() const { return m_owner_ref.item_pointer(*m_handle_shptr); }
			mm_iterator_type& ip() const { return item_pointer(); }
			//const mm_iterator_handle_type& handle() const { return (*m_handle_shptr); }
			operator cipointer() const {
				cipointer retval(m_owner_ref);
				retval.const_item_pointer().set_to_beginning();
				retval.const_item_pointer().advance(mse::CInt(item_pointer().position()));
				return retval;
			}

			void reset() { item_pointer().reset(); }
			bool points_to_an_item() const { return item_pointer().points_to_an_item(); }
			bool points_to_end_marker() const { return item_pointer().points_to_end_marker(); }
			bool points_to_beginning() const { return item_pointer().points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return item_pointer().has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return item_pointer().has_next(); }
			bool has_previous() const { return item_pointer().has_previous(); }
			void set_to_beginning() { item_pointer().set_to_beginning(); }
			void set_to_end_marker() { item_pointer().set_to_end_marker(); }
			void set_to_next() { item_pointer().set_to_next(); }
			void set_to_previous() { item_pointer().set_to_previous(); }
			ipointer& operator ++() { item_pointer().operator ++(); return (*this); }
			ipointer operator++(int) { ipointer _Tmp = *this; ++*this; return (_Tmp); }
			ipointer& operator --() { item_pointer().operator --(); return (*this); }
			ipointer operator--(int) { ipointer _Tmp = *this; --*this; return (_Tmp); }
			void advance(difference_type n) { item_pointer().advance(n); }
			void regress(difference_type n) { item_pointer().regress(n); }
			ipointer& operator +=(int n) { item_pointer().operator +=(n); return (*this); }
			ipointer& operator -=(int n) { item_pointer().operator -=(n); return (*this); }
			ipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			ipointer operator-(int n) const { return ((*this) + (-n)); }
			difference_type operator-(const ipointer& _Right_cref) const { return item_pointer() - (_Right_cref.item_pointer()); }
			reference operator*() const { return item_pointer().operator*(); }
			reference item() const { return operator*(); }
			reference previous_item() const { return item_pointer().previous_item(); }
			pointer operator->() const { return item_pointer().operator->(); }
			reference operator[](difference_type _Off) const { return (*(*this + _Off)); }
			ipointer& operator=(const ipointer& _Right_cref) { item_pointer().operator=(_Right_cref.item_pointer()); return (*this); }
			bool operator==(const ipointer& _Right_cref) const { return item_pointer().operator==(_Right_cref.item_pointer()); }
			bool operator!=(const ipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ipointer& _Right) const { return (item_pointer() < _Right.item_pointer()); }
			bool operator<=(const ipointer& _Right) const { return (item_pointer() <= _Right.item_pointer()); }
			bool operator>(const ipointer& _Right) const { return (item_pointer() > _Right.item_pointer()); }
			bool operator>=(const ipointer& _Right) const { return (item_pointer() >= _Right.item_pointer()); }
			void set_to_item_pointer(const ipointer& _Right_cref) { item_pointer().set_to_item_pointer(_Right_cref.item_pointer()); }
			CSize_t position() const { return item_pointer().position(); }
		private:
			_Myt& m_owner_ref;
			std::shared_ptr<mm_iterator_handle_type> m_handle_shptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
		};

		ipointer ibegin() {	// return ipointer for beginning of mutable sequence
			ipointer retval(*this);
			retval.set_to_beginning();
			return retval;
		}
		cipointer ibegin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.set_to_beginning();
			return retval;
		}
		ipointer iend() {	// return ipointer for end of mutable sequence
			ipointer retval(*this);
			retval.set_to_end_marker();
			return retval;
		}
		cipointer iend() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.set_to_end_marker();
			return retval;
		}
		cipointer cibegin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.set_to_beginning();
			return retval;
		}
		cipointer ciend() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.set_to_beginning();
			return retval;
		}

		msevector(const cipointer &start, const cipointer &end, const _A& _Al = _A())
			: base_class(static_cast<typename base_class::const_iterator>(start.const_item_pointer()), static_cast<typename base_class::const_iterator>(end.const_item_pointer()), _Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) {
			if (start.m_owner_cref != end.m_owner_cref) { throw(std::out_of_range("invalid arguments - void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
			typename base_class::const_iterator _F = start;
			typename base_class::const_iterator _L = end;
			(*this).assign(_F, _L);
		}
		void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) {
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			typename base_class::const_iterator _F = first;
			typename base_class::const_iterator _L = last;
			_L++;
			(*this).assign(_F, _L);
		}
		void assign(const cipointer &start, const cipointer &end) {
			assign(start.const_item_pointer(), end.const_item_pointer());
		}
		void assign_inclusive(const cipointer &first, const cipointer &last) {
			assign_inclusive(first.const_item_pointer(), last.const_item_pointer());
		}
		void insert_before(const mm_iterator_type &pos, size_t _M, const _Ty& _X) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			typename base_class::iterator _P = pos;
			(*this).insert(_P, _M, _X);
		}
		void insert_before(const mm_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			typename base_class::iterator _P = pos;
			(*this).insert(pos, 1, std::move(_X));
		}
		void insert_before(const mm_iterator_type &pos, const _Ty& _X = _Ty()) { (*this).insert(pos, 1, _X); }
		void insert_before(const mm_iterator_type &pos, const mm_const_iterator_type &start, const mm_const_iterator_type &end) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (start.m_owner_cref != end.m_owner_cref) { throw(std::out_of_range("invalid arguments - void insert_before(const mm_const_iterator_type &pos, const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
			typename base_class::iterator _P = pos;
			typename base_class::const_iterator _F = start;
			typename base_class::const_iterator _L = end;
			(*this).insert(_P, _F, _L);
		}
		void insert_before_inclusive(const mm_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void insert_before_inclusive(const mm_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void insert_before_inclusive(const mm_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			typename base_class::iterator _P = pos;
			typename base_class::const_iterator _F = first;
			typename base_class::const_iterator _L = last;
			_L++;
			(*this).insert(_P, _F, _L);
		}
#ifndef MSVC2010_COMPATIBILE
		void insert_before(const mm_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			typename base_class::iterator _P = pos;
			(*this).insert(_P, _Ilist);
		}
#endif /*MSVC2010_COMPATIBILE*/
		ipointer insert_before(const ipointer &pos, size_t _M, const _Ty& _X) {
			CSize_t original_pos = pos.position();
			insert_before(pos.item_pointer(), _M, _X);
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		ipointer insert_before(const ipointer &pos, _Ty&& _X) {
			CSize_t original_pos = pos.position();
			insert_before(pos.item_pointer(), std::move(_X));
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		ipointer insert_before(const ipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
		ipointer insert_before(const ipointer &pos, const cipointer &start, const cipointer &end) {
			CSize_t original_pos = pos.position();
			insert_before(pos.item_pointer(), start.const_item_pointer(), end.const_item_pointer());
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		ipointer insert_before_inclusive(const ipointer &pos, const cipointer &first, const cipointer &last) {
			auto end = last; end.set_to_next();
			return insert_before(pos, first, end);
		}
#ifndef MSVC2010_COMPATIBILE
		ipointer insert_before(const ipointer &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
			CSize_t original_pos = pos.position();
			(*this).insert(pos.item_pointer(), _Ilist);
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
#endif /*MSVC2010_COMPATIBILE*/
		void insert_before(CSize_t pos, _Ty&& _X) {
			typename base_class::iterator _P = (*this).begin() + mse::as_a_size_t(pos);
			(*this).insert(_P, std::move(_X));
		}
		void insert_before(CSize_t pos, const _Ty& _X = _Ty()) {
			typename base_class::iterator _P = (*this).begin() + mse::as_a_size_t(pos);
			(*this).insert(_P, _X);
		}
		void insert_before(CSize_t pos, size_t _M, const _Ty& _X) {
			typename base_class::iterator _P = (*this).begin() + mse::as_a_size_t(pos);
			(*this).insert(_P, _M, _X);
		}
#ifndef MSVC2010_COMPATIBILE
		void insert_before(CSize_t pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
			typename base_class::iterator _P = (*this).begin() + mse::as_a_size_t(pos);
			(*this).insert(_P, _Ilist);
		}
#endif /*MSVC2010_COMPATIBILE*/
		/* These insert() functions are just aliases for their corresponding insert_before() functions. */
		ipointer insert(const ipointer &pos, size_t _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
		ipointer insert(const ipointer &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
		ipointer insert(const ipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
		ipointer insert(const ipointer &pos, const cipointer &start, const cipointer &end) { return insert_before(pos, start, end); }
#ifndef MSVC2010_COMPATIBILE
		ipointer insert(const ipointer &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) { return insert_before(pos, _Ilist); }
#endif /*MSVC2010_COMPATIBILE*/
		void erase(const mm_iterator_type &pos) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			typename base_class::iterator _P = pos;
			(*this).erase(_P);
		}
		void erase(const mm_iterator_type &start, const mm_iterator_type &end) {
			if (start.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			if (end.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			typename base_class::iterator _F = start;
			typename base_class::iterator _L = end;
			(*this).erase(_F, _L);
		}
		void erase_inclusive(const mm_iterator_type &first, const mm_iterator_type &last) {
			if (first.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (last.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void erase_inclusive() - msevector")); }
			typename base_class::iterator _F = first;
			typename base_class::iterator _L = last;
			_L++;
			(*this).erase(_F, _L);
		}
		ipointer erase(const ipointer &pos) {
			auto retval = pos;
			//ipointer retval(*this); retval = pos;
			retval.set_to_next();
			erase(pos.item_pointer());
			return retval;
		}
		ipointer erase(const ipointer &start, const ipointer &end) {
			auto retval = end;
			erase(start.item_pointer(), end.item_pointer());
			return retval;
		}
		ipointer erase_inclusive(const ipointer &first, const ipointer &last) {
			auto end = last; end.set_to_next();
			return erase(first, end);
		}
		void erase_previous_item(const mm_iterator_type &pos) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			if (!(pos.has_previous())) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			typename base_class::iterator _P = pos;
			_P--;
			(*this).erase(_P);
		}
		ipointer erase_previous_item(const ipointer &pos) {
			erase_previous_item(pos.item_pointer());
			return pos;
		}


		/* Note that, at the moment, ss_const_iterator_type inherits publicly from base_class::const_iterator. This is not intended to be a permanent
		characteristc of ss_const_iterator_type and any reference to, or interpretation of, an ss_const_iterator_type as an base_class::const_iterator is (and has
		always been) depricated. ss_const_iterator_type endeavors to support (and continue to support) the subset of the base_class::const_iterator
		interface that is compatible with the security/safety goals of ss_const_iterator_type.
		In particular, keep in mind that base_class::const_iterator does not have a virtual destructor, so deallocating an ss_const_iterator_type as an
		base_class::const_iterator would result in memory leaks. */
		class ss_const_iterator_type : public base_class::const_iterator {
		public:
			typedef typename base_class::const_iterator::iterator_category iterator_category;
			typedef typename base_class::const_iterator::value_type value_type;
			typedef typename base_class::const_iterator::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename base_class::const_iterator::pointer pointer;
			typedef typename base_class::const_pointer const_pointer;
			typedef typename base_class::const_iterator::reference reference;
			typedef typename base_class::const_reference const_reference;

			ss_const_iterator_type() {}
			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if ((1 <= m_owner_cptr->size()) && (m_index < m_owner_cptr->size())) { return true; }
				else {
					if (m_index == m_owner_cptr->size()) { return false; }
					else { throw(std::out_of_range("attempt to use invalid ss_const_iterator_type - bool points_to_an_item() const - ss_const_iterator_type - msevector")); }
				}
			}
			bool points_to_end_marker() const {
				if (false == points_to_an_item()) {
					if (m_index == m_owner_cptr->size()) {
						return true;
					}
					else { throw(std::out_of_range("attempt to use invalid ss_const_iterator_type - bool points_to_end_marker() const - ss_const_iterator_type - msevector")); }
				}
				else { return false; }
			}
			bool points_to_beginning() const {
				if (0 == m_index) { return true; }
				else { return false; }
			}
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return points_to_an_item(); } //his is
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return has_next_item_or_end_marker(); }
			bool has_previous() const { return (!points_to_beginning()); }
			void set_to_beginning() {
				m_index = 0;
				base_class::const_iterator::operator=(m_owner_cptr->cbegin());
			}
			void set_to_end_marker() {
				m_index = m_owner_cptr->size();
				base_class::const_iterator::operator=(m_owner_cptr->cend());
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					base_class::const_iterator::operator++();
					if (m_owner_cptr->size() <= m_index) {
						if (m_owner_cptr->size() < m_index) { assert(false); reset(); }
					}
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - void set_to_next() - ss_const_iterator_type - msevector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
					base_class::const_iterator::operator--();
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - void set_to_previous() - ss_const_iterator_type - msevector"));
				}
			}
			ss_const_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			ss_const_iterator_type operator++(int) { ss_const_iterator_type _Tmp = *this; ++*this; return (_Tmp); }
			ss_const_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			ss_const_iterator_type operator--(int) { ss_const_iterator_type _Tmp = *this; --*this; return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = CInt(m_index) + n;
				if ((0 > new_index) || (m_owner_cptr->size() < new_index)) {
					throw(std::out_of_range("index out of range - void advance(difference_type n) - ss_const_iterator_type - msevector"));
				}
				else {
					m_index = new_index;
					base_class::const_iterator::operator+=(n);
				}
			}
			void regress(difference_type n) { advance(-n); }
			ss_const_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			ss_const_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			ss_const_iterator_type operator+(difference_type n) const {
				ss_const_iterator_type retval; retval.m_owner_cptr = m_owner_cptr;
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			ss_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const ss_const_iterator_type &rhs) const {
				if (rhs.m_owner_cptr != (*this).m_owner_cptr) { throw(std::out_of_range("invalid argument - difference_type operator-(const ss_const_iterator_type &rhs) const - msevector::ss_const_iterator_type")); }
				auto retval = (static_cast<const typename base_class::const_iterator&>(*this) - static_cast<const typename base_class::const_iterator&>(rhs));
				assert((int)((*m_owner_cptr).size()) >= retval);
				return retval;
			}
			const_reference operator*() const {
				if (points_to_an_item()) {
					return m_owner_cptr->at(mse::as_a_size_t(m_index));
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - const_reference operator*() const - ss_const_iterator_type - msevector"));
				}
			}
			const_reference item() const { return operator*(); }
			const_reference previous_item() const {
				if ((*this).has_previous()) {
					return m_owner_cptr->at(m_index - 1);
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - const_reference previous_item() const - ss_const_iterator_type - msevector"));
				}
			}
			const_pointer operator->() const {
				if (points_to_an_item()) {
					const_cast<ss_const_iterator_type *>(this)->sync_const_iterator_to_index();
					//sync_const_iterator_to_index();
					return base_class::const_iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - pointer operator->() const - ss_const_iterator_type - msevector"));
				}
			}
			const_reference operator[](difference_type _Off) const { return (*(*this + _Off)); }
			/*
			ss_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref)
			{
			CInt d = std::distance<typename base_class::iterator>(m_owner_cptr->cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cptr->size() >= d)) {
			if (m_owner_cptr->size() == d) {
			assert(m_owner_cptr->cend() == _Right_cref);
			}
			m_index = CSize_t(d);
			base_class::const_iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - ss_const_iterator_type& operator=(const typename base_class::const_iterator& _Right_cref) - ss_const_iterator_type - msevector"));
			}
			return (*this);
			}
			*/
			ss_const_iterator_type& operator=(const ss_const_iterator_type& _Right_cref) {
				((*this).m_owner_cptr) = _Right_cref.m_owner_cptr;
				(*this).m_index = _Right_cref.m_index;
				base_class::const_iterator::operator=(_Right_cref);
				return (*this);
			}
			bool operator==(const ss_const_iterator_type& _Right_cref) const { return ((_Right_cref.m_index == m_index) && (_Right_cref.m_owner_cptr == m_owner_cptr)); }
			bool operator!=(const ss_const_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ss_const_iterator_type& _Right) const {
				if (this->m_owner_cptr != _Right.m_owner_cptr) { throw(std::out_of_range("invalid argument - ss_const_iterator_type& operator<(const ss_const_iterator_type& _Right) - ss_const_iterator_type - msevector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const ss_const_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const ss_const_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const ss_const_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_const_item_pointer(const ss_const_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			void invalidate_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					(*this).reset();
				}
			}
			void shift_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last, mse::CInt shift) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					auto new_index = (*this).m_index + shift;
					if ((0 > new_index) || (m_owner_cptr->size() < new_index)) {
						throw(std::out_of_range("void shift_inclusive_range() - ss_const_iterator_type - msevector"));
					}
					else {
						(*this).m_index = new_index;
						(*this).sync_const_iterator_to_index();
					}
				}
			}
			CSize_t position() const {
				return m_index;
			}
		private:
			void sync_const_iterator_to_index() {
				assert(m_owner_cptr->size() >= (*this).m_index);
				base_class::const_iterator::operator=(m_owner_cptr->cbegin());
				base_class::const_iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CSize_t m_index;
			TSaferPtrForLegacy<const _Myt> m_owner_cptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
		};
		/* Note that, at the moment, ss_iterator_type inherits publicly from base_class::iterator. This is not intended to be a permanent
		characteristc of ss_iterator_type and any reference to, or interpretation of, an ss_iterator_type as an base_class::iterator is (and has
		always been) depricated. ss_iterator_type endeavors to support (and continue to support) the subset of the base_class::iterator
		interface that is compatible with the security/safety goals of ss_iterator_type.
		In particular, keep in mind that base_class::iterator does not have a virtual destructor, so deallocating an ss_iterator_type as an
		base_class::iterator would result in memory leaks. */
		class ss_iterator_type : public base_class::iterator {
		public:
			typedef typename base_class::iterator::iterator_category iterator_category;
			typedef typename base_class::iterator::value_type value_type;
			typedef typename base_class::iterator::difference_type difference_type;
			typedef difference_type distance_type;	// retained
			typedef typename base_class::iterator::pointer pointer;
			typedef typename base_class::iterator::reference reference;

			ss_iterator_type() {}
			void reset() { set_to_end_marker(); }
			bool points_to_an_item() const {
				if ((1 <= m_owner_ptr->size()) && (m_index < m_owner_ptr->size())) { return true; }
				else {
					if (m_index == m_owner_ptr->size()) { return false; }
					else { throw(std::out_of_range("attempt to use invalid ss_iterator_type - bool points_to_an_item() const - ss_iterator_type - msevector")); }
				}
			}
			bool points_to_end_marker() const {
				if (false == points_to_an_item()) {
					if (m_index == m_owner_ptr->size()) {
						return true;
					}
					else { throw(std::out_of_range("attempt to use invalid ss_iterator_type - bool points_to_end_marker() const - ss_iterator_type - msevector")); }
				}
				else { return false; }
			}
			bool points_to_beginning() const {
				if (0 == m_index) { return true; }
				else { return false; }
			}
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return points_to_an_item(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return has_next_item_or_end_marker(); }
			bool has_previous() const { return (!points_to_beginning()); }
			void set_to_beginning() {
				m_index = 0;
				base_class::iterator::operator=(m_owner_ptr->begin());
			}
			void set_to_end_marker() {
				m_index = m_owner_ptr->size();
				base_class::iterator::operator=(m_owner_ptr->end());
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					base_class::iterator::operator++();
					if (m_owner_ptr->size() <= m_index) {
						if (m_owner_ptr->size() < m_index) { assert(false); reset(); }
					}
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - void set_to_next() - ss_const_iterator_type - msevector"));
				}
			}
			void set_to_previous() {
				if (has_previous()) {
					m_index -= 1;
					base_class::iterator::operator--();
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - void set_to_previous() - ss_iterator_type - msevector"));
				}
			}
			ss_iterator_type& operator ++() { (*this).set_to_next(); return (*this); }
			ss_iterator_type operator++(int) { ss_iterator_type _Tmp = *this; ++*this; return (_Tmp); }
			ss_iterator_type& operator --() { (*this).set_to_previous(); return (*this); }
			ss_iterator_type operator--(int) { ss_iterator_type _Tmp = *this; --*this; return (_Tmp); }
			void advance(difference_type n) {
				auto new_index = CInt(m_index) + n;
				if ((0 > new_index) || (m_owner_ptr->size() < new_index)) {
					throw(std::out_of_range("index out of range - void advance(difference_type n) - ss_iterator_type - msevector"));
				}
				else {
					m_index = new_index;
					base_class::iterator::operator+=(n);
				}
			}
			void regress(difference_type n) { advance(-n); }
			ss_iterator_type& operator +=(difference_type n) { (*this).advance(n); return (*this); }
			ss_iterator_type& operator -=(difference_type n) { (*this).regress(n); return (*this); }
			ss_iterator_type operator+(difference_type n) const {
				ss_iterator_type retval; retval.m_owner_ptr = m_owner_ptr;
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			ss_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const ss_iterator_type& rhs) const {
				if (rhs.m_owner_ptr != (*this).m_owner_ptr) { throw(std::out_of_range("invalid argument - difference_type operator-(const ss_iterator_type& rhs) const - msevector::ss_iterator_type")); }
				auto retval = (static_cast<const typename base_class::iterator&>(*this) - static_cast<const typename base_class::iterator&>(rhs));
				assert((int)((*m_owner_ptr).size()) >= retval);
				return retval;
			}
			reference operator*() {
				if (points_to_an_item()) {
					return m_owner_ptr->at(mse::as_a_size_t(m_index));
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - reference operator*() - ss_iterator_type - msevector"));
				}
			}
			reference item() { return operator*(); }
			reference previous_item() {
				if ((*this).has_previous()) {
					return m_owner_ptr->at(m_index - 1);
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - reference previous_item() - ss_const_iterator_type - msevector"));
				}
			}
			pointer operator->() {
				if (points_to_an_item()) {
					sync_iterator_to_index();
					return base_class::iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - pointer operator->() - ss_iterator_type - msevector"));
				}
			}
			reference operator[](difference_type _Off) { return (*(*this + _Off)); }
			/*
			ss_iterator_type& operator=(const typename base_class::iterator& _Right_cref)
			{
			CInt d = std::distance<typename base_class::iterator>(m_owner_ptr->begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ptr->size() >= d)) {
			if (m_owner_ptr->size() == d) {
			assert(m_owner_ptr->end() == _Right_cref);
			}
			m_index = CSize_t(d);
			base_class::iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - ss_iterator_type& operator=(const typename base_class::iterator& _Right_cref) - ss_const_iterator_type - msevector"));
			}
			return (*this);
			}
			*/
			ss_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
				((*this).m_owner_ptr) = _Right_cref.m_owner_ptr;
				(*this).m_index = _Right_cref.m_index;
				base_class::iterator::operator=(_Right_cref);
				return (*this);
			}
			bool operator==(const ss_iterator_type& _Right_cref) const { return ((_Right_cref.m_index == m_index) && (_Right_cref.m_owner_ptr == m_owner_ptr)); }
			bool operator!=(const ss_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ss_iterator_type& _Right) const {
				if (this->m_owner_ptr != _Right.m_owner_ptr) { throw(std::out_of_range("invalid argument - ss_iterator_type& operator<(const ss_iterator_type& _Right) - ss_iterator_type - msevector")); }
				return (m_index < _Right.m_index);
			}
			bool operator<=(const ss_iterator_type& _Right) const { return (((*this) < _Right) || (_Right == (*this))); }
			bool operator>(const ss_iterator_type& _Right) const { return (!((*this) <= _Right)); }
			bool operator>=(const ss_iterator_type& _Right) const { return (!((*this) < _Right)); }
			void set_to_item_pointer(const ss_iterator_type& _Right_cref) {
				(*this) = _Right_cref;
			}
			void invalidate_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					(*this).reset();
				}
			}
			void shift_inclusive_range(mse::CSize_t index_of_first, mse::CSize_t index_of_last, mse::CInt shift) {
				if ((index_of_first <= (*this).m_index) && (index_of_last >= (*this).m_index)) {
					auto new_index = (*this).m_index + shift;
					if ((0 > new_index) || (m_owner_ptr->size() < new_index)) {
						throw(std::out_of_range("void shift_inclusive_range() - ss_iterator_type - msevector"));
					}
					else {
						(*this).m_index = new_index;
						(*this).sync_iterator_to_index();
					}
				}
			}
			CSize_t position() const {
				return m_index;
			}
			operator ss_const_iterator_type() const {
				ss_const_iterator_type retval;
				if (m_owner_ptr == nullptr) {
					retval = m_owner_ptr->ss_cbegin();
					retval.advance(mse::CInt(m_index));
				}
				return retval;
			}
		private:
			void sync_iterator_to_index() {
				assert(m_owner_ptr->size() >= (*this).m_index);
				base_class::iterator::operator=(m_owner_ptr->begin());
				base_class::iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CSize_t m_index;
			TSaferPtrForLegacy<_Myt> m_owner_ptr;
			friend class /*_Myt*/msevector<_Ty, _A>;
		};
		typedef std::reverse_iterator<ss_iterator_type> ss_reverse_iterator_type;
		typedef std::reverse_iterator<ss_const_iterator_type> ss_const_reverse_iterator_type;

		ss_iterator_type ss_begin()
		{	// return base_class::iterator for beginning of mutable sequence
			ss_iterator_type retval; retval.m_owner_ptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_const_iterator_type ss_begin() const
		{	// return base_class::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_iterator_type ss_end()
		{	// return base_class::iterator for end of mutable sequence
			ss_iterator_type retval; retval.m_owner_ptr = this;
			retval.set_to_end_marker();
			return retval;
		}

		ss_const_iterator_type ss_end() const
		{	// return base_class::iterator for end of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_set_to_end_marker();
			return retval;
		}

		ss_const_iterator_type ss_cbegin() const
		{	// return base_class::iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_const_iterator_type ss_cend() const
		{	// return base_class::iterator for end of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_set_to_end_marker();
			return retval;
		}

		ss_const_reverse_iterator_type ss_crbegin() const
		{	// return base_class::iterator for beginning of reversed nonmutable sequence
			return (ss_rbegin());
		}

		ss_const_reverse_iterator_type ss_crend() const
		{	// return base_class::iterator for end of reversed nonmutable sequence
			return (ss_rend());
		}

		ss_reverse_iterator_type ss_rbegin()
		{	// return base_class::iterator for beginning of reversed mutable sequence
			return (reverse_iterator(ss_end()));
		}

		ss_const_reverse_iterator_type ss_rbegin() const
		{	// return base_class::iterator for beginning of reversed nonmutable sequence
			return (const_reverse_iterator(ss_end()));
		}

		ss_reverse_iterator_type ss_rend()
		{	// return base_class::iterator for end of reversed mutable sequence
			return (reverse_iterator(ss_begin()));
		}

		ss_const_reverse_iterator_type ss_rend() const
		{	// return base_class::iterator for end of reversed nonmutable sequence
			return (const_reverse_iterator(ss_begin()));
		}

		msevector(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
			: base_class(static_cast<typename base_class::const_iterator>(start), static_cast<typename base_class::const_iterator>(end), _Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { throw(std::out_of_range("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			typename base_class::const_iterator _F = start;
			typename base_class::const_iterator _L = end;
			(*this).assign(_F, _L);
		}
		void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			typename base_class::const_iterator _F = first;
			typename base_class::const_iterator _L = last;
			_L++;
			(*this).assign(_F, _L);
		}
		ss_iterator_type insert_before(const ss_iterator_type &pos, size_t _M, const _Ty& _X) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			typename base_class::iterator _P = pos;
			(*this).insert(_P, _M, _X);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before(const ss_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			typename base_class::iterator _P = pos;
			(*this).insert(_P, std::move(_X));
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before(const ss_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
		ss_iterator_type insert_before(const ss_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (start.m_owner_cptr != end.m_owner_cptr) { throw(std::out_of_range("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			CSize_t original_pos = pos.position();
			typename base_class::iterator _P = pos;
			typename base_class::const_iterator _F = start;
			typename base_class::const_iterator _L = end;
			(*this).insert(_P, _F, _L);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		/* Note that safety cannot be guaranteed when using an insert() function that takes unsafe typename base_class::iterator and/or pointer parameters. */
		ss_iterator_type insert_before(const ss_iterator_type &pos, const _Ty* start, const _Ty* &end) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			typename base_class::iterator _P = pos;
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before_inclusive(const ss_iterator_type &pos, const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void insert_before_inclusive(const ss_iterator_type &pos, const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void insert_before_inclusive(const ss_iterator_type &pos, const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			CSize_t original_pos = pos.position();
			typename base_class::iterator _P = pos;
			typename base_class::const_iterator _F = first;
			typename base_class::const_iterator _L = last;
			_L++;
			(*this).insert(_P, _F, _L);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
#ifndef MSVC2010_COMPATIBILE
		ss_iterator_type insert_before(const ss_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) {	// insert initializer_list
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			typename base_class::iterator _P = pos;
			(*this).insert(_P, _Ilist);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
#endif /*MSVC2010_COMPATIBILE*/
		/* These insert() functions are just aliases for their corresponding insert_before() functions. */
		ss_iterator_type insert(const ss_iterator_type &pos, size_t _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
		ss_iterator_type insert(const ss_iterator_type &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
		ss_iterator_type insert(const ss_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
		ss_iterator_type insert(const ss_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) { return insert_before(pos, start, end); }
		/* Note that safety cannot be guaranteed when using an insert() function that takes unsafe typename base_class::iterator and/or pointer parameters. */
		ss_iterator_type insert(const ss_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
#ifndef MSVC2010_COMPATIBILE
		ss_iterator_type insert(const ss_iterator_type &pos, _XSTD initializer_list<typename base_class::value_type> _Ilist) { return insert_before(pos, _Ilist); }
#endif /*MSVC2010_COMPATIBILE*/
		ss_iterator_type erase(const ss_iterator_type &pos) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			typename base_class::iterator _P = pos;
			auto retval = pos;
			static_cast<typename base_class::iterator&>(retval) = (*this).erase(_P);
			return retval;
		}
		ss_iterator_type erase(const ss_iterator_type &start, const ss_iterator_type &end) {
			if (start.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			if (end.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			typename base_class::iterator _F = start;
			typename base_class::iterator _L = end;
			auto retval = start;
			static_cast<typename base_class::iterator&>(retval) = (*this).erase(_F, _L);
			return retval;
		}
		ss_iterator_type erase_inclusive(const ss_iterator_type &first, const ss_iterator_type &last) {
			if (first.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (last.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void erase_inclusive() - msevector")); }
			typename base_class::iterator _F = first;
			typename base_class::iterator _L = last;
			_L++;
			auto retval = first;
			static_cast<typename base_class::iterator&>(retval) = (*this).erase(_F, _L);
			return retval;
		}
		void erase_previous_item(const ss_iterator_type &pos) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			if (!(pos.has_previous())) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			typename base_class::iterator _P = pos;
			_P--;
			(*this).erase(_P);
		}
	};

}
#endif /*ndef MSEMSEVECTOR_H*/
