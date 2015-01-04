
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef MSETL_H
#define MSETL_H

#include "mseprimitives.h"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>

namespace mse {

	template<typename _Ty>
	class TConstNullInitialized {
	public:
		TConstNullInitialized() : m_ptr(nullptr) {}
		TConstNullInitialized(const _Ty* ptr) : m_ptr(ptr) {}
		void pointer(const _Ty* ptr) { m_ptr = ptr; }
		const _Ty* pointer() const { return m_ptr; }
		_Ty& operator*() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TConstNullInitialized")); }
			return (*m_ptr);
		}
		const _Ty* operator->() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TConstNullInitialized")); }
			return m_ptr;
		}
		TConstNullInitialized<_Ty>& operator=(const _Ty* ptr) {
			m_ptr = ptr;
			return (*this);
		}
		TConstNullInitialized<_Ty>& operator=(const TConstNullInitialized<_Ty>& _Right_cref) {
			m_ptr = _Right_cref.m_ptr;
			return (*this);
		}
		bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const _Ty* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TConstNullInitialized<_Ty> &_Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const TConstNullInitialized<_Ty> &_Right_cref) const { return (!((*this) == _Right_cref)); }

		const _Ty* m_ptr;
	};

	template<typename _Ty>
	class TNullInitialized {
	public:
		TNullInitialized() : m_ptr(nullptr) {}
		TNullInitialized(_Ty* ptr) : m_ptr(ptr) {}
		void pointer(_Ty* ptr) { m_ptr = ptr; }
		_Ty* pointer() const { return m_ptr; }
		_Ty& operator*() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TNullInitialized")); }
			return (*m_ptr);
		}
		_Ty* operator->() const {
			if (nullptr == m_ptr) { throw(std::out_of_range("attempt to dereference null pointer - mse::TNullInitialized")); }
			return m_ptr;
		}
		TNullInitialized<_Ty>& operator=(_Ty* ptr) {
			m_ptr = ptr;
			return (*this);
		}
		TNullInitialized<_Ty>& operator=(const TNullInitialized<_Ty>& _Right_cref) {
			m_ptr = _Right_cref.m_ptr;
			return (*this);
		}
		bool operator==(const _Ty* _Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const _Ty* _Right_cref) const { return (!((*this) == _Right_cref)); }
		bool operator==(const TNullInitialized<_Ty> &_Right_cref) const { return (_Right_cref == m_ptr); }
		bool operator!=(const TNullInitialized<_Ty> &_Right_cref) const { return (!((*this) == _Right_cref)); }

		_Ty* m_ptr;
	};

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
		explicit msevector(size_type _N, const _Ty& _V = _Ty(), const _A& _Al = _A())
			: base_class(_N, _V, _Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		msevector(base_class&& _X) : base_class(std::move(_X)), m_mmitset(*this) { m_debug_size = size(); }
		msevector(const base_class& _X) : base_class(_X), m_mmitset(*this) { m_debug_size = size(); }
		msevector(_Myt&& _X) : base_class(std::move(_X)), m_mmitset(*this) { m_debug_size = size(); }
		msevector(const _Myt& _X) : base_class(_X), m_mmitset(*this) { m_debug_size = size(); }
		typedef const_iterator _It;
		/* Note that safety cannot be guaranteed when using these constructors that take unsafe iterator and/or pointer parameters. */
		msevector(_It _F, _It _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { m_debug_size = size(); }
		msevector(const _Ty*  _F, const _Ty*  _L, const _A& _Al = _A()) : base_class(_F, _L, _Al), m_mmitset(*this) { m_debug_size = size(); }
		template<class _Iter, class = typename enable_if<_Is_iterator<_Iter>::value, void>::type>
		msevector(_Iter _First, _Iter _Last) : base_class(_First, _Last), m_mmitset(*this) { m_debug_size = size(); }
		template<class _Iter, class = typename enable_if<_Is_iterator<_Iter>::value, void>::type>
		msevector(_Iter _First, _Iter _Last, const typename base_class::_Alloc& _Al) : base_class(_First, _Last, _Al), m_mmitset(*this) { m_debug_size = size(); }
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
		void reserve(size_type _Count)
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
		void resize(size_type _N, const _Ty& _X = _Ty()) {
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
		const_reference operator[](size_type _P) const {
			return at(_P);
		}
		reference operator[](size_type _P) {
			return at(_P);
		}
		reference front() {	// return first element of mutable sequence
			if (0 == size()) { throw(std::out_of_range("front() on empty - reference front() - msevector")); }
			return base_class::front();
		}
		const_reference front() const {	// return first element of nonmutable sequence
			if (0 == size()) { throw(std::out_of_range("front() on empty - const_reference front() - msevector")); }
			return base_class::front();
		}
		reference back() {	// return last element of mutable sequence
			if (0 == size()) { throw(std::out_of_range("back() on empty - reference back() - msevector")); }
			return base_class::back();
		}
		const_reference back() const {	// return last element of nonmutable sequence
			if (0 == size()) { throw(std::out_of_range("back() on empty - const_reference back() - msevector")); }
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
		void assign(size_type _N, const _Ty& _X = _Ty()) {
			base_class::assign(_N, _X);
			m_debug_size = size();
			m_mmitset.reset();
		}
		iterator insert(iterator _P, _Ty&& _X) {
			return (emplace(_P, std::move(_X)));
		}
		iterator insert(iterator _P, const _Ty& _X = _Ty()) {
			CInt di = std::distance(base_class::begin(), _P); CSize_t d = di;
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator insert() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			iterator retval = base_class::insert(_P, _X);
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
		iterator insert(iterator _P, size_type _M, const _Ty& _X) {
			CInt di = std::distance(base_class::begin(), _P); CSize_t d = di;
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator insert() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			iterator retval = base_class::insert(_P, _M, _X);
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
		template<class _Iter>
		typename std::enable_if<std::_Is_iterator<_Iter>::value, iterator>::type
			insert(const_iterator _Where, _Iter _First, _Iter _Last) {	// insert [_First, _Last) at _Where
				CInt di = std::distance(base_class::cbegin(), _Where); CSize_t d = di;
				if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator insert() - msevector")); }

				auto _M = CInt(std::distance(_First, _Last));
				auto original_size = CSize_t((*this).size());
				auto original_capacity = CSize_t((*this).capacity());

				//if (0 > _M) { throw(std::out_of_range("invalid argument - iterator insert() - msevector")); }
				auto retval = base_class::insert(_Where, _First, _Last);
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
		template<class... _Valty>
		void emplace_back(_Valty&&... _Val)
		{	// insert by moving into element at end
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			base_class::emplace_back(std::forward<_Valty>(_Val)...);
			m_debug_size = size();

			assert((original_size + 1) == CSize_t((*this).size()));
			m_mmitset.shift_inclusive_range(original_size, original_size, 1); /*shift the end markers*/
			auto new_capacity = CSize_t((*this).capacity());
			bool realloc_occured = (new_capacity != original_capacity);
			if (realloc_occured) {
				m_mmitset.sync_iterators_to_index();
			}
		}
		template<class... _Valty>
		iterator emplace(const_iterator _Where, _Valty&&... _Val)
		{	// insert by moving _Val at _Where
			CInt di = std::distance(base_class::cbegin(), _Where); CSize_t d = di;
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator emplace() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			auto retval = base_class::emplace(_Where, std::forward<_Valty>(_Val)...);
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
		iterator erase(iterator _P) {
			CInt di = std::distance(base_class::begin(), _P); CSize_t d = di;
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator erase() - msevector")); }

			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			if (end() == _P) { throw(std::out_of_range("invalid argument - iterator erase(iterator _P) - msevector")); }
			iterator retval = base_class::erase(_P);
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
		iterator erase(iterator _F, iterator _L) {
			CInt di = std::distance(base_class::begin(), _F); CSize_t d = di;
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator erase() - msevector")); }
			CInt di2 = std::distance(base_class::begin(), _L); CSize_t d2 = di2;
			if ((0 > di2) || (CSize_t((*this).size()) < di2)) { throw(std::out_of_range("index out of range - iterator erase() - msevector")); }

			auto _M = CInt(std::distance(_F, _L));
			auto original_size = CSize_t((*this).size());
			auto original_capacity = CSize_t((*this).capacity());

			if ((end() == _F)/* || (0 > _M)*/) { throw(std::out_of_range("invalid argument - iterator erase(iterator _F, iterator _L) - msevector")); }
			iterator retval = base_class::erase(_F, _L);
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

		msevector(_XSTD initializer_list<value_type> _Ilist,
			const _Alloc& _Al = allocator_type())
			: base_class(_Ilist, _Al), m_mmitset(*this) {	// construct from initializer_list
			m_debug_size = size();
		}
		_Myt& operator=(_XSTD initializer_list<value_type> _Ilist) {	// assign initializer_list
			operator=((base_class)_Ilist);
			m_mmitset.reset();
			return (*this);
		}
		void assign(_XSTD initializer_list<value_type> _Ilist) {	// assign initializer_list
			base_class::assign(_Ilist);
			m_debug_size = size();
			m_mmitset.reset();
		}
		iterator insert(const_iterator _Where, _XSTD initializer_list<value_type> _Ilist) {	// insert initializer_list
			CInt di = std::distance(base_class::cbegin(), _Where); CSize_t d = di;
			if ((0 > di) || (CSize_t((*this).size()) < di)) { throw(std::out_of_range("index out of range - iterator insert() - msevector")); }

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

		size_type m_debug_size;

		/* Note that, at the moment, mm_const_iterator_type inherits publicly from const_iterator. This is not intended to be a permanent
		characteristc of mm_const_iterator_type and any reference to, or interpretation of, an mm_const_iterator_type as an const_iterator is (and has
		always been) depricated. mm_const_iterator_type endeavors to support (and continue to support) the subset of the const_iterator
		interface that is compatible with the security/safety goals of mm_const_iterator_type.
		In particular, keep in mind that const_iterator does not have a virtual destructor, so deallocating an mm_const_iterator_type as an
		const_iterator would result in memory leaks. */
		class mm_const_iterator_type : public const_iterator {
		public:
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
				const_iterator::operator=(m_owner_cref.cbegin());
				if (1 <= m_owner_cref.size()) {
					m_points_to_an_item = true;
				}
				else { assert(false == m_points_to_an_item); }
			}
			void set_to_end_marker() {
				m_index = m_owner_cref.size();
				const_iterator::operator=(m_owner_cref.cend());
				m_points_to_an_item = false;
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					const_iterator::operator++();
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
					const_iterator::operator--();
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
					const_iterator::operator+=(n);
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
				mm_const_iterator_type retval; retval.m_owner_cptr = m_owner_cptr;
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			mm_const_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const mm_const_iterator_type &rhs) const {
				if ((&(rhs.m_owner_cref)) != (&((*this).m_owner_cref))) { throw(std::out_of_range("invalid argument - difference_type operator-(const mm_const_iterator_type &rhs) const - msevector::mm_const_iterator_type")); }
				auto retval = (static_cast<const const_iterator&>(*this) - static_cast<const const_iterator&>(rhs));
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
					sync_const_iterator_to_index();
					return const_iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - pointer operator->() const - mm_const_iterator_type - msevector"));
				}
			}
			const_reference operator[](difference_type _Off) const { return (*(*this + _Off)); }
			/*
			mm_const_iterator_type& operator=(const const_iterator& _Right_cref)
			{
			CInt d = std::distance<iterator>(m_owner_cref.cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cref.size() >= d)) {
			if (m_owner_cref.size() == d) {
			assert(m_owner_cref.cend() == _Right_cref);
			m_points_to_an_item = false;
			} else {
			m_points_to_an_item = true;
			}
			m_index = CSize_t(d);
			const_iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_const_iterator_type& operator=(const const_iterator& _Right_cref) - mm_const_iterator_type - msevector"));
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
					const_iterator::operator=(_Right_cref);
				}
				else {
					throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_const_iterator_type& operator=(const iterator& _Right_cref) - mm_const_iterator_type - msevector"));
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
				const_iterator::operator=(m_owner_cref.cbegin());
				const_iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CBool m_points_to_an_item;
			mse::CSize_t m_index;
			const _Myt& m_owner_cref;
			friend class mm_iterator_set_type;
			friend class _Myt;
		};
		/* Note that, at the moment, mm_iterator_type inherits publicly from iterator. This is not intended to be a permanent
		characteristc of mm_iterator_type and any reference to, or interpretation of, an mm_iterator_type as an iterator is (and has
		always been) depricated. mm_iterator_type endeavors to support (and continue to support) the subset of the iterator
		interface that is compatible with the security/safety goals of mm_iterator_type.
		In particular, keep in mind that iterator does not have a virtual destructor, so deallocating an mm_iterator_type as an
		iterator would result in memory leaks. */
		class mm_iterator_type : public iterator {
		public:
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
				iterator::operator=(m_owner_ref.begin());
				if (1 <= m_owner_ref.size()) {
					m_points_to_an_item = true;
				}
				else { assert(false == m_points_to_an_item); }
			}
			void set_to_end_marker() {
				m_index = m_owner_ref.size();
				iterator::operator=(m_owner_ref.end());
				m_points_to_an_item = false;
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					iterator::operator++();
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
					iterator::operator--();
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
					iterator::operator+=(n);
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
				mm_iterator_type retval; retval.m_owner_ptr = m_owner_ptr;
				retval = (*this);
				retval.advance(n);
				return retval;
			}
			mm_iterator_type operator-(difference_type n) const { return ((*this) + (-n)); }
			difference_type operator-(const mm_iterator_type& rhs) const {
				if ((&rhs.m_owner_ref) != (&(*this).m_owner_ref)) { throw(std::out_of_range("invalid argument - difference_type operator-(const mm_iterator_type& rhs) const - msevector::mm_iterator_type")); }
				auto retval = (static_cast<const iterator&>(*this) - static_cast<const iterator&>(rhs));
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
					return m_owner_cref.at(m_index - 1);
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - reference previous_item() - mm_const_iterator_type - msevector"));
				}
			}
			pointer operator->() {
				if (points_to_an_item()) {
					sync_iterator_to_index();
					return iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - pointer operator->() - mm_iterator_type - msevector"));
				}
			}
			reference operator[](difference_type _Off) { return (*(*this + _Off)); }
			/*
			mm_iterator_type& operator=(const iterator& _Right_cref)
			{
			CInt d = std::distance<iterator>(m_owner_ref.begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ref.size() >= d)) {
			if (m_owner_ref.size() == d) {
			assert(m_owner_ref.end() == _Right_cref);
			m_points_to_an_item = false;
			} else {
			m_points_to_an_item = true;
			}
			m_index = CSize_t(d);
			iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_iterator_type& operator=(const iterator& _Right_cref) - mm_const_iterator_type - msevector"));
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
					iterator::operator=(_Right_cref);
				}
				else {
					throw(std::out_of_range("doesn't seem to be a valid assignment value - mm_iterator_type& operator=(const iterator& _Right_cref) - mm_const_iterator_type - msevector"));
				}
				return (*this);
			}
			bool operator==(const mm_iterator_type& _Right_cref) const { return ((_Right_cref.m_index == m_index) && (_Right_cref.m_owner_ref == m_owner_ref)); }
			bool operator!=(const mm_iterator_type& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const mm_iterator_type& _Right) const {
				if (&((*this).m_owner_ref) != &(_Right.m_owner_ref)) { throw(std::out_of_range("invalid argument - mm_iterator_type& operator<(const iterator& _Right) - mm_iterator_type - msevector")); }
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
				iterator::operator=(m_owner_ref.begin());
				iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CBool m_points_to_an_item;
			mse::CSize_t m_index;
			_Myt& m_owner_ref;
			friend class mm_iterator_set_type;
			friend class _Myt;
		};

	private:
		typedef std::size_t CHashKey1;
		class mm_const_iterator_handle_type {
		public:
			mm_const_iterator_handle_type(const CHashKey1& key_cref, std::shared_ptr<mm_const_iterator_type>& shptr_cref) : m_shptr(shptr_cref), m_key(key_cref) {}
		private:
			std::shared_ptr<mm_const_iterator_type> m_shptr;
			CHashKey1 m_key;
			friend class _Myt;
			friend class mm_iterator_set_type;
		};
		class mm_iterator_handle_type {
		public:
			mm_iterator_handle_type(const CHashKey1& key_cref, std::shared_ptr<mm_iterator_type>& shptr_ref) : m_shptr(shptr_ref), m_key(key_cref) {}
		private:
			std::shared_ptr<mm_iterator_type> m_shptr;
			CHashKey1 m_key;
			friend class _Myt;
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
				CMMConstIterators::value_type new_item(key, shptr);
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
				CMMIterators::value_type new_item(key, shptr);
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

			friend class _Myt;
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
			typedef typename difference_type distance_type;	// retained
			typedef typename mm_const_iterator_type::pointer pointer;
			typedef typename mm_const_iterator_type::reference reference;

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
			void advance(typename difference_type n) { const_item_pointer().advance(n); }
			void regress(typename difference_type n) { const_item_pointer().regress(n); }
			cipointer& operator +=(int n) { const_item_pointer().operator +=(n); return (*this); }
			cipointer& operator -=(int n) { const_item_pointer().operator -=(n); return (*this); }
			cipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			cipointer operator-(int n) const { return ((*this) + (-n)); }
			typename difference_type operator-(const cipointer& _Right_cref) const { return const_item_pointer() - (_Right_cref.const_item_pointer()); }
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
			friend class _Myt;
		};
		class ipointer {
		public:
			typedef typename mm_iterator_type::iterator_category iterator_category;
			typedef typename mm_iterator_type::value_type value_type;
			typedef typename mm_iterator_type::difference_type difference_type;
			typedef typename difference_type distance_type;	// retained
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
			void advance(typename difference_type n) { item_pointer().advance(n); }
			void regress(typename difference_type n) { item_pointer().regress(n); }
			ipointer& operator +=(int n) { item_pointer().operator +=(n); return (*this); }
			ipointer& operator -=(int n) { item_pointer().operator -=(n); return (*this); }
			ipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			ipointer operator-(int n) const { return ((*this) + (-n)); }
			typename difference_type operator-(const ipointer& _Right_cref) const { return item_pointer() - (_Right_cref.item_pointer()); }
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
			friend class _Myt;
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
			: base_class(static_cast<const_iterator>(start.const_item_pointer()), static_cast<const_iterator>(end.const_item_pointer()), _Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) {
			if (start.m_owner_cref != end.m_owner_cref) { throw(std::out_of_range("invalid arguments - void assign(const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
			const_iterator _F = start;
			const_iterator _L = end;
			(*this).assign(_F, _L);
		}
		void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) {
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void assign_inclusive(const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			const_iterator _F = first;
			const_iterator _L = last;
			_L++;
			(*this).assign(_F, _L);
		}
		void assign(const cipointer &start, const cipointer &end) {
			assign(start.const_item_pointer(), end.const_item_pointer());
		}
		void assign_inclusive(const cipointer &first, const cipointer &last) {
			assign_inclusive(first.const_item_pointer(), last.const_item_pointer());
		}
		void insert_before(mm_iterator_type &pos, size_type _M, const _Ty& _X) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			iterator _P = pos;
			(*this).insert(_P, _M, _X);
		}
		void insert_before(mm_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			iterator _P = pos;
			(*this).insert(pos, 1, std::move(_X));
		}
		void insert_before(mm_iterator_type &pos, const _Ty& _X = _Ty()) { (*this).insert(pos, 1, _X); }
		void insert_before(mm_iterator_type &pos, const mm_const_iterator_type &start, const mm_const_iterator_type &end) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (start.m_owner_cref != end.m_owner_cref) { throw(std::out_of_range("invalid arguments - void insert_before(const mm_const_iterator_type &pos, const mm_const_iterator_type &start, const mm_const_iterator_type &end) - msevector")); }
			iterator _P = pos;
			const_iterator _F = start;
			const_iterator _L = end;
			(*this).insert(_P, _F, _L);
		}
		void insert_before_inclusive(mm_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void insert_before_inclusive(mm_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void insert_before_inclusive(mm_iterator_type &pos, const mm_const_iterator_type &first, const mm_const_iterator_type &last) - msevector")); }
			iterator _P = pos;
			const_iterator _F = first;
			const_iterator _L = last;
			_L++;
			(*this).insert(_P, _F, _L);
		}
		void insert_before(mm_iterator_type &pos, _XSTD initializer_list<value_type> _Ilist) {	// insert initializer_list
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			iterator _P = pos;
			(*this).insert(_P, _Ilist);
		}
		ipointer insert_before(ipointer &pos, size_type _M, const _Ty& _X) {
			CSize_t original_pos = pos.position();
			insert_before(pos.item_pointer(), _M, _X);
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		ipointer insert_before(ipointer &pos, _Ty&& _X) {
			CSize_t original_pos = pos.position();
			insert_before(pos.item_pointer(), std::move(_X));
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		ipointer insert_before(ipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
		ipointer insert_before(ipointer &pos, const cipointer &start, const cipointer &end) {
			CSize_t original_pos = pos.position();
			insert_before(pos.item_pointer(), start.const_item_pointer(), end.const_item_pointer());
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		ipointer insert_before_inclusive(ipointer &pos, const cipointer &first, const cipointer &last) {
			auto end = last; end.set_to_next();
			return insert_before(pos, first, end);
		}
		ipointer insert_before(ipointer &pos, _XSTD initializer_list<value_type> _Ilist) {	// insert initializer_list
			CSize_t original_pos = pos.position();
			(*this).insert(pos.item_pointer(), _Ilist);
			ipointer retval(*this); retval.advance((CInt)original_pos);
			return retval;
		}
		void insert_before(CSize_t pos, _Ty&& _X) {
			iterator _P = (*this).begin() + pos.as_a_size_t();
			(*this).insert(_P, std::move(_X));
		}
		void insert_before(CSize_t pos, const _Ty& _X = _Ty()) {
			iterator _P = (*this).begin() + pos.as_a_size_t();
			(*this).insert(_P, _X);
		}
		void insert_before(CSize_t pos, size_type _M, const _Ty& _X) {
			iterator _P = (*this).begin() + pos.as_a_size_t();
			(*this).insert(_P, _M, _X);
		}
		void insert_before(CSize_t pos, _XSTD initializer_list<value_type> _Ilist) {	// insert initializer_list
			iterator _P = (*this).begin() + pos.as_a_size_t();
			(*this).insert(_P, _Ilist);
		}
		/* These insert() functions are just aliases for their corresponding insert_before() functions. */
		ipointer insert(ipointer &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
		ipointer insert(ipointer &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
		ipointer insert(ipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
		ipointer insert(ipointer &pos, const cipointer &start, const cipointer &end) { return insert_before(pos, start, end); }
		ipointer insert(ipointer &pos, _XSTD initializer_list<value_type> _Ilist) { return insert_before(pos, _Ilist); }
		void erase(mm_iterator_type &pos) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			iterator _P = pos;
			(*this).erase(_P);
		}
		void erase(mm_iterator_type &start, mm_iterator_type &end) {
			if (start.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			if (end.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			iterator _F = start;
			iterator _L = end;
			(*this).erase(_F, _L);
		}
		void erase_inclusive(mm_iterator_type &first, mm_iterator_type &last) {
			if (first.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (last.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void erase_inclusive() - msevector")); }
			iterator _F = first;
			iterator _L = last;
			_L++;
			(*this).erase(_F, _L);
		}
		ipointer erase(ipointer &pos) {
			auto retval = pos;
			//ipointer retval(*this); retval = pos;
			retval.set_to_next();
			erase(pos.item_pointer());
			return retval;
		}
		ipointer erase(ipointer &start, ipointer &end) {
			auto retval = end;
			erase(start.item_pointer(), end.item_pointer());
			return retval;
		}
		ipointer erase_inclusive(ipointer &first, ipointer &last) {
			auto end = last; end.set_to_next();
			return erase(first, end);
		}
		void erase_previous_item(mm_iterator_type &pos) {
			if (pos.m_owner_ref != (*this)) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			if (!(pos.has_previous())) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			iterator _P = pos;
			_P--;
			(*this).erase(_P);
		}
		ipointer erase_previous_item(ipointer &pos) {
			erase_previous_item(pos.item_pointer());
			return pos;
		}


		/* Note that, at the moment, ss_const_iterator_type inherits publicly from const_iterator. This is not intended to be a permanent
		characteristc of ss_const_iterator_type and any reference to, or interpretation of, an ss_const_iterator_type as an const_iterator is (and has
		always been) depricated. ss_const_iterator_type endeavors to support (and continue to support) the subset of the const_iterator
		interface that is compatible with the security/safety goals of ss_const_iterator_type.
		In particular, keep in mind that const_iterator does not have a virtual destructor, so deallocating an ss_const_iterator_type as an
		const_iterator would result in memory leaks. */
		class ss_const_iterator_type : public const_iterator {
		public:
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
				const_iterator::operator=(m_owner_cptr->cbegin());
			}
			void set_to_end_marker() {
				m_index = m_owner_cptr->size();
				const_iterator::operator=(m_owner_cptr->cend());
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					const_iterator::operator++();
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
					const_iterator::operator--();
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
					const_iterator::operator+=(n);
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
				auto retval = (static_cast<const const_iterator&>(*this) - static_cast<const const_iterator&>(rhs));
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
					sync_const_iterator_to_index();
					return const_iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid const_item_pointer - pointer operator->() const - ss_const_iterator_type - msevector"));
				}
			}
			const_reference operator[](difference_type _Off) const { return (*(*this + _Off)); }
			/*
			ss_const_iterator_type& operator=(const const_iterator& _Right_cref)
			{
			CInt d = std::distance<iterator>(m_owner_cptr->cbegin(), _Right_cref);
			if ((0 <= d) && (m_owner_cptr->size() >= d)) {
			if (m_owner_cptr->size() == d) {
			assert(m_owner_cptr->cend() == _Right_cref);
			}
			m_index = CSize_t(d);
			const_iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - ss_const_iterator_type& operator=(const const_iterator& _Right_cref) - ss_const_iterator_type - msevector"));
			}
			return (*this);
			}
			*/
			ss_const_iterator_type& operator=(const ss_const_iterator_type& _Right_cref) {
				((*this).m_owner_cptr) = _Right_cref.m_owner_cptr;
				(*this).m_index = _Right_cref.m_index;
				const_iterator::operator=(_Right_cref);
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
				const_iterator::operator=(m_owner_cptr->cbegin());
				const_iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CSize_t m_index;
			TConstNullInitialized<_Myt> m_owner_cptr;
			friend class _Myt;
		};
		/* Note that, at the moment, ss_iterator_type inherits publicly from iterator. This is not intended to be a permanent
		characteristc of ss_iterator_type and any reference to, or interpretation of, an ss_iterator_type as an iterator is (and has
		always been) depricated. ss_iterator_type endeavors to support (and continue to support) the subset of the iterator
		interface that is compatible with the security/safety goals of ss_iterator_type.
		In particular, keep in mind that iterator does not have a virtual destructor, so deallocating an ss_iterator_type as an
		iterator would result in memory leaks. */
		class ss_iterator_type : public iterator {
		public:
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
				iterator::operator=(m_owner_ptr->begin());
			}
			void set_to_end_marker() {
				m_index = m_owner_ptr->size();
				iterator::operator=(m_owner_ptr->end());
			}
			void set_to_next() {
				if (points_to_an_item()) {
					m_index += 1;
					iterator::operator++();
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
					iterator::operator--();
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
					iterator::operator+=(n);
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
				auto retval = (static_cast<const iterator&>(*this) - static_cast<const iterator&>(rhs));
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
					return m_owner_cref.at(m_index - 1);
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - reference previous_item() - ss_const_iterator_type - msevector"));
				}
			}
			pointer operator->() {
				if (points_to_an_item()) {
					sync_iterator_to_index();
					return iterator::operator->();
				}
				else {
					throw(std::out_of_range("attempt to use invalid item_pointer - pointer operator->() - ss_iterator_type - msevector"));
				}
			}
			reference operator[](difference_type _Off) { return (*(*this + _Off)); }
			/*
			ss_iterator_type& operator=(const iterator& _Right_cref)
			{
			CInt d = std::distance<iterator>(m_owner_ptr->begin(), _Right_cref);
			if ((0 <= d) && (m_owner_ptr->size() >= d)) {
			if (m_owner_ptr->size() == d) {
			assert(m_owner_ptr->end() == _Right_cref);
			}
			m_index = CSize_t(d);
			iterator::operator=(_Right_cref);
			}
			else {
			throw(std::out_of_range("doesn't seem to be a valid assignment value - ss_iterator_type& operator=(const iterator& _Right_cref) - ss_const_iterator_type - msevector"));
			}
			return (*this);
			}
			*/
			ss_iterator_type& operator=(const ss_iterator_type& _Right_cref) {
				((*this).m_owner_ptr) = _Right_cref.m_owner_ptr;
				(*this).m_index = _Right_cref.m_index;
				const_iterator::operator=(_Right_cref);
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
				iterator::operator=(m_owner_ptr->begin());
				iterator::operator+=(mse::as_a_size_t(m_index));
			}
			mse::CSize_t m_index;
			TNullInitialized<_Myt> m_owner_ptr;
			friend class _Myt;
		};
		typedef std::reverse_iterator<ss_iterator_type> ss_reverse_iterator_type;
		typedef std::reverse_iterator<ss_const_iterator_type> ss_const_reverse_iterator_type;

		ss_iterator_type ss_begin()
		{	// return iterator for beginning of mutable sequence
			ss_iterator_type retval; retval.m_owner_ptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_const_iterator_type ss_begin() const
		{	// return iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_iterator_type ss_end()
		{	// return iterator for end of mutable sequence
			ss_iterator_type retval; retval.m_owner_ptr = this;
			retval.set_to_end_marker();
			return retval;
		}

		ss_const_iterator_type ss_end() const
		{	// return iterator for end of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_set_to_end_marker();
			return retval;
		}

		ss_const_iterator_type ss_cbegin() const
		{	// return iterator for beginning of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_beginning();
			return retval;
		}

		ss_const_iterator_type ss_cend() const
		{	// return iterator for end of nonmutable sequence
			ss_const_iterator_type retval; retval.m_owner_cptr = this;
			retval.set_to_set_to_end_marker();
			return retval;
		}

		ss_const_reverse_iterator_type ss_crbegin() const
		{	// return iterator for beginning of reversed nonmutable sequence
			return (ss_rbegin());
		}

		ss_const_reverse_iterator_type ss_crend() const
		{	// return iterator for end of reversed nonmutable sequence
			return (ss_rend());
		}

		ss_reverse_iterator_type ss_rbegin()
		{	// return iterator for beginning of reversed mutable sequence
			return (reverse_iterator(ss_end()));
		}

		ss_const_reverse_iterator_type ss_rbegin() const
		{	// return iterator for beginning of reversed nonmutable sequence
			return (const_reverse_iterator(ss_end()));
		}

		ss_reverse_iterator_type ss_rend()
		{	// return iterator for end of reversed mutable sequence
			return (reverse_iterator(ss_begin()));
		}

		ss_const_reverse_iterator_type ss_rend() const
		{	// return iterator for end of reversed nonmutable sequence
			return (const_reverse_iterator(ss_begin()));
		}

		msevector(const ss_const_iterator_type &start, const ss_const_iterator_type &end, const _A& _Al = _A())
			: base_class(static_cast<const_iterator>(start), static_cast<const_iterator>(end), _Al), m_mmitset(*this) {
			m_debug_size = size();
		}
		void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (start.m_owner_cptr != end.m_owner_cptr) { throw(std::out_of_range("invalid arguments - void assign(const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			const_iterator _F = start;
			const_iterator _L = end;
			(*this).assign(_F, _L);
		}
		void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void assign_inclusive(const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			const_iterator _F = first;
			const_iterator _L = last;
			_L++;
			(*this).assign(_F, _L);
		}
		ss_iterator_type insert_before(ss_iterator_type &pos, size_type _M, const _Ty& _X) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			iterator _P = pos;
			(*this).insert(_P, _M, _X);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before(ss_iterator_type &pos, _Ty&& _X) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			iterator _P = pos;
			(*this).insert(_P, std::move(_X));
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before(ss_iterator_type &pos, const _Ty& _X = _Ty()) { return (*this).insert(pos, 1, _X); }
		ss_iterator_type insert_before(ss_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (start.m_owner_cptr != end.m_owner_cptr) { throw(std::out_of_range("invalid arguments - void insert_before(const ss_const_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) - msevector")); }
			CSize_t original_pos = pos.position();
			iterator _P = pos;
			const_iterator _F = start;
			const_iterator _L = end;
			(*this).insert(_P, _F, _L);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		/* Note that safety cannot be guaranteed when using an insert() function that takes unsafe iterator and/or pointer parameters. */
		ss_iterator_type insert_before(ss_iterator_type &pos, const _Ty* start, const _Ty* &end) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			iterator _P = pos;
			(*this).insert(_P, start, end);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before_inclusive(ss_iterator_type &pos, const ss_const_iterator_type &first, const ss_const_iterator_type &last) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			if (first.m_owner_cref != last.m_owner_cref) { throw(std::out_of_range("invalid arguments - void insert_before_inclusive(ss_iterator_type &pos, const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void insert_before_inclusive(ss_iterator_type &pos, const ss_const_iterator_type &first, const ss_const_iterator_type &last) - msevector")); }
			CSize_t original_pos = pos.position();
			iterator _P = pos;
			const_iterator _F = first;
			const_iterator _L = last;
			_L++;
			(*this).insert(_P, _F, _L);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		ss_iterator_type insert_before(ss_iterator_type &pos, _XSTD initializer_list<value_type> _Ilist) {	// insert initializer_list
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void insert_before() - msevector")); }
			CSize_t original_pos = pos.position();
			iterator _P = pos;
			(*this).insert(_P, _Ilist);
			ss_iterator_type retval = ss_begin();
			retval.advance((CInt)original_pos);
			return retval;
		}
		/* These insert() functions are just aliases for their corresponding insert_before() functions. */
		ss_iterator_type insert(ss_iterator_type &pos, size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
		ss_iterator_type insert(ss_iterator_type &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
		ss_iterator_type insert(ss_iterator_type &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
		ss_iterator_type insert(ss_iterator_type &pos, const ss_const_iterator_type &start, const ss_const_iterator_type &end) { return insert_before(pos, start, end); }
		/* Note that safety cannot be guaranteed when using an insert() function that takes unsafe iterator and/or pointer parameters. */
		ss_iterator_type insert(ss_iterator_type &pos, const _Ty* start, const _Ty* &end) { return insert_before(pos, start, end); }
		ss_iterator_type insert(ss_iterator_type &pos, _XSTD initializer_list<value_type> _Ilist) { return insert_before(pos, _Ilist); }
		ss_iterator_type erase(ss_iterator_type &pos) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			iterator _P = pos;
			auto retval = pos;
			static_cast<iterator&>(retval) = (*this).erase(_P);
			return retval;
		}
		ss_iterator_type erase(ss_iterator_type &start, ss_iterator_type &end) {
			if (start.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			if (end.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase() - msevector")); }
			iterator _F = start;
			iterator _L = end;
			auto retval = start;
			static_cast<iterator&>(retval) = (*this).erase(_F, _L);
			return retval;
		}
		ss_iterator_type erase_inclusive(ss_iterator_type &first, ss_iterator_type &last) {
			if (first.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (last.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase_inclusive() - msevector")); }
			if (!(last.points_to_item())) { throw(std::out_of_range("invalid argument - void erase_inclusive() - msevector")); }
			iterator _F = first;
			iterator _L = last;
			_L++;
			auto retval = first;
			static_cast<iterator&>(retval) = (*this).erase(_F, _L);
			return retval;
		}
		void erase_previous_item(ss_iterator_type &pos) {
			if (pos.m_owner_ptr != this) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			if (!(pos.has_previous())) { throw(std::out_of_range("invalid arguments - void erase_previous_item() - msevector")); }
			iterator _P = pos;
			_P--;
			(*this).erase(_P);
		}
	};


	template<class _Ty, class _A = std::allocator<_Ty> >
	class ivector {
	public:
		typedef ivector<_Ty, _A> _Myt;
		typedef msevector<_Ty, _A> _MV;

		msevector<_Ty, _A>& msevector() const { return (*m_shptr); }
		operator _MV() { return msevector(); }

		explicit ivector(const _A& _Al = _A()) : m_shptr(new _MV(_Al)) {}
		explicit ivector(typename _MV::size_type _N, const _Ty& _V = _Ty(), const _A& _Al = _A()) : m_shptr(new _MV(_N, _V, _Al)) {}
		ivector(_MV&& _X) : m_shptr(new _MV(std::move(_X))) {}
		ivector(const _MV& _X) : m_shptr(new _MV(_X)) {}
		ivector(_Myt&& _X) : m_shptr(new _MV(std::move(_X.msevector()))) {}
		ivector(const _Myt& _X) : m_shptr(new _MV(_X.msevector())) {}
		typedef typename _MV::const_iterator _It;
		ivector(_It _F, _It _L, const _A& _Al = _A()) : m_shptr(new _MV(_F, _L, _Al)) {}

		_Myt& operator=(_MV&& _X) { m_shptr->operator=(std::move(_X)); return (*this); }
		_Myt& operator=(const _MV& _X) { m_shptr->operator=(_X); return (*this); }
		_Myt& operator=(_Myt&& _X) { m_shptr->operator=(std::move(_X.msevector())); return (*this); }
		_Myt& operator=(const _Myt& _X) { m_shptr->operator=(_X.msevector()); return (*this); }
		void reserve(typename _MV::size_type _Count) { m_shptr->reserve(_Count); }
		void resize(typename _MV::size_type _N, const _Ty& _X = _Ty()) { m_shptr->resize(_N, _X); }
		typename _MV::const_reference operator[](typename _MV::size_type _P) const { return m_shptr->operator[](_P); }
		typename _MV::reference operator[](typename _MV::size_type _P) { return m_shptr->operator[](_P); }
		void push_back(_Ty&& _X) { m_shptr->push_back(std::move(_X)); }
		void push_back(const _Ty& _X) { m_shptr->push_back(_X); }
		void pop_back() { m_shptr->pop_back(); }
		void assign(_It _F, _It _L) { m_shptr->assign(_F, _L); }
		void assign(typename _MV::size_type _N, const _Ty& _X = _Ty()) { m_shptr->assign(_N, _X); }
		typename _MV::iterator insert(typename _MV::iterator _P, _Ty&& _X) { return m_shptr->insert(_P, std::move(_X)); }
		typename _MV::iterator insert(typename _MV::iterator _P, const _Ty& _X = _Ty()) { return m_shptr->insert(_P, _X); }
		typename _MV::iterator insert(typename _MV::iterator _P, typename _MV::size_type _M, const _Ty& _X) { return m_shptr->insert(_P, _M, _X); }
		template<class _Iter>
		typename std::enable_if<std::_Is_iterator<_Iter>::value, typename _MV::iterator>::type
			insert(typename _MV::const_iterator _Where, _Iter _First, _Iter _Last) { return m_shptr->insert(_Where, _First, _Last); }
		template<class... _Valty>
		void emplace_back(_Valty&&... _Val) { m_shptr->emplace_back(std::forward<_Valty>(_Val)...); }
		template<class... _Valty>
		typename _MV::iterator emplace(typename _MV::const_iterator _Where, _Valty&&... _Val) { return m_shptr->emplace(_Where, std::forward<_Valty>(_Val)...); }
		typename _MV::iterator erase(typename _MV::iterator _P) { return m_shptr->emplace(_P); }
		typename _MV::iterator erase(typename _MV::iterator _F, typename _MV::iterator _L) { return m_shptr->emplace(_F, _L); }
		void clear() { m_shptr->clear(); }
		void swap(_MV& _X) { m_shptr->swap(_X); }
		void swap(_Myt& _X) { m_shptr->swap(_X.msevector()); }

		ivector(_XSTD initializer_list<typename _MV::value_type> _Ilist, const typename _MV::_Alloc& _Al = _MV::allocator_type()) : m_shptr(new _MV(_Ilist, _Al)) {}
		_Myt& operator=(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->operator=(_Ilist); return (*this); }
		void assign(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->assign(_Ilist); }
		typename _MV::iterator insert(typename _MV::const_iterator _Where, _XSTD initializer_list<typename _MV::value_type> _Ilist) { return m_shptr->insert(_Where, _Ilist); }

		typename _MV::size_type capacity() const _NOEXCEPT{ return m_shptr->capacity(); }
		void shrink_to_fit() { m_shtr->shrink_to_fit(); }
		typename _MV::size_type size() const _NOEXCEPT{ return m_shptr->size(); }
		typename _MV::size_type max_size() const _NOEXCEPT{ return m_shptr->max_size(); }
		bool empty() const _NOEXCEPT{ return m_shptr->empty(); }
		typename _MV::_Alloc get_allocator() const _NOEXCEPT{ return m_shptr->get_allocator(); }
		typename _MV::const_reference at(typename _MV::size_type _Pos) const { return m_shptr->at(_Pos); }
		typename _MV::reference at(typename _MV::size_type _Pos) { return m_shptr->at(_Pos); }
		typename _MV::reference front() { return m_shptr->front(); }
		typename _MV::const_reference front() const { return m_shptr->front(); }
		typename _MV::reference back() { return m_shptr->back(); }
		typename _MV::const_reference back() const { return m_shptr->back(); }

		class cipointer {
		public:
			typedef typename _MV::mm_const_iterator_type::iterator_category iterator_category;
			typedef typename _MV::mm_const_iterator_type::value_type value_type;
			typedef typename _MV::mm_const_iterator_type::difference_type difference_type;
			typedef typename _MV::difference_type distance_type;	// retained
			typedef typename _MV::mm_const_iterator_type::pointer pointer;
			typedef typename _MV::mm_const_iterator_type::reference reference;

			cipointer(const _Myt& owner_cref) : m_msevector_shptr(owner_cref.m_shptr), m_cipointer_shptr(new _MV::cipointer(*(owner_cref.m_shptr))) {}
			cipointer(const cipointer& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_cipointer_shptr(new _MV::cipointer(*(src_cref.m_cipointer_shptr))) {
				(*this) = src_cref;
			}
			~cipointer() {}
			typename _MV::cipointer& msevector_cipointer() const { return (*m_cipointer_shptr); }
			typename _MV::cipointer& mvcip() const { return msevector_cipointer(); }

			void reset() { msevector_cipointer().reset(); }
			bool points_to_an_item() const { return msevector_cipointer().points_to_an_item(); }
			bool points_to_end_marker() const { return msevector_cipointer().points_to_end_marker(); }
			bool points_to_beginning() const { return msevector_cipointer().points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return msevector_cipointer().has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return msevector_cipointer().has_next(); }
			bool has_previous() const { return msevector_cipointer().has_previous(); }
			void set_to_beginning() { msevector_cipointer().set_to_beginning(); }
			void set_to_end_marker() { msevector_cipointer().set_to_end_marker(); }
			void set_to_next() { msevector_cipointer().set_to_next(); }
			void set_to_previous() { msevector_cipointer().set_to_previous(); }
			cipointer& operator ++() { msevector_cipointer().operator ++(); return (*this); }
			cipointer operator++(int) { cipointer _Tmp = *this; ++*this; return (_Tmp); }
			cipointer& operator --() { msevector_cipointer().operator --(); return (*this); }
			cipointer operator--(int) { cipointer _Tmp = *this; --*this; return (_Tmp); }
			void advance(typename _MV::difference_type n) { msevector_cipointer().advance(n); }
			void regress(typename _MV::difference_type n) { msevector_cipointer().regress(n); }
			cipointer& operator +=(int n) { msevector_cipointer().operator +=(n); return (*this); }
			cipointer& operator -=(int n) { msevector_cipointer().operator -=(n); return (*this); }
			cipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			cipointer operator-(int n) const { return ((*this) + (-n)); }
			typename _MV::difference_type operator-(const cipointer& _Right_cref) const { return msevector_cipointer() - (_Right_cref.msevector_cipointer()); }
			typename _MV::const_reference operator*() const { return msevector_cipointer().operator*(); }
			typename _MV::const_reference item() const { return operator*(); }
			typename _MV::const_reference previous_item() const { return msevector_cipointer().previous_item(); }
			typename _MV::const_pointer operator->() const { return msevector_cipointer().operator->(); }
			typename _MV::const_reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
			cipointer& operator=(const cipointer& _Right_cref) { msevector_cipointer().operator=(_Right_cref.msevector_cipointer()); return (*this); }
			bool operator==(const cipointer& _Right_cref) const { return msevector_cipointer().operator==(_Right_cref.msevector_cipointer()); }
			bool operator!=(const cipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const cipointer& _Right) const { return (msevector_cipointer() < _Right.msevector_cipointer()); }
			bool operator<=(const cipointer& _Right) const { return (msevector_cipointer() <= _Right.msevector_cipointer()); }
			bool operator>(const cipointer& _Right) const { return (msevector_cipointer() > _Right.msevector_cipointer()); }
			bool operator>=(const cipointer& _Right) const { return (msevector_cipointer() >= _Right.msevector_cipointer()); }
			void set_to_const_item_pointer(const cipointer& _Right_cref) { msevector_cipointer().set_to_const_item_pointer(_Right_cref.msevector_cipointer()); }
			CSize_t position() const { return msevector_cipointer().position(); }
		private:
			cipointer(const std::shared_ptr<_MV>& msevector_shptr) : m_msevector_shptr(msevector_shptr), m_cipointer_shptr(new _MV::cipointer(*(msevector_shptr))) {}
			std::shared_ptr<_MV> m_msevector_shptr;
			/* m_cipointer_shptr needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
			std::shared_ptr<typename _MV::cipointer> m_cipointer_shptr;
			friend class _Myt;
			friend class ipointer;
		};
		class ipointer {
		public:
			typedef typename _MV::mm_iterator_type::iterator_category iterator_category;
			typedef typename _MV::mm_iterator_type::value_type value_type;
			typedef typename _MV::mm_iterator_type::difference_type difference_type;
			typedef typename _MV::difference_type distance_type;	// retained
			typedef typename _MV::mm_iterator_type::pointer pointer;
			typedef typename _MV::mm_iterator_type::reference reference;

			ipointer(_Myt& owner_ref) : m_msevector_shptr(owner_ref.m_shptr), m_ipointer_shptr(new _MV::ipointer(*(owner_ref.m_shptr))) {}
			ipointer(const ipointer& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_ipointer_shptr(new _MV::ipointer(*(src_cref.m_ipointer_shptr))) {
				(*this) = src_cref;
			}
			~ipointer() {}
			typename _MV::ipointer& msevector_ipointer() const { return (*m_ipointer_shptr); }
			typename _MV::ipointer& mvip() const { return msevector_ipointer(); }
			//const mm_iterator_handle_type& handle() const { return (*m_handle_shptr); }
			operator cipointer() const {
				cipointer retval(m_msevector_shptr);
				retval.const_item_pointer().set_to_beginning();
				retval.const_item_pointer().advance(mse::CInt(msevector_ipointer().position()));
				return retval;
			}

			void reset() { msevector_ipointer().reset(); }
			bool points_to_an_item() const { return msevector_ipointer().points_to_an_item(); }
			bool points_to_end_marker() const { return msevector_ipointer().points_to_end_marker(); }
			bool points_to_beginning() const { return msevector_ipointer().points_to_beginning(); }
			/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
			bool has_next_item_or_end_marker() const { return msevector_ipointer().has_next_item_or_end_marker(); }
			/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
			bool has_next() const { return msevector_ipointer().has_next(); }
			bool has_previous() const { return msevector_ipointer().has_previous(); }
			void set_to_beginning() { msevector_ipointer().set_to_beginning(); }
			void set_to_end_marker() { msevector_ipointer().set_to_end_marker(); }
			void set_to_next() { msevector_ipointer().set_to_next(); }
			void set_to_previous() { msevector_ipointer().set_to_previous(); }
			ipointer& operator ++() { msevector_ipointer().operator ++(); return (*this); }
			ipointer operator++(int) { ipointer _Tmp = *this; ++*this; return (_Tmp); }
			ipointer& operator --() { msevector_ipointer().operator --(); return (*this); }
			ipointer operator--(int) { ipointer _Tmp = *this; --*this; return (_Tmp); }
			void advance(typename _MV::difference_type n) { msevector_ipointer().advance(n); }
			void regress(typename _MV::difference_type n) { msevector_ipointer().regress(n); }
			ipointer& operator +=(int n) { msevector_ipointer().operator +=(n); return (*this); }
			ipointer& operator -=(int n) { msevector_ipointer().operator -=(n); return (*this); }
			ipointer operator+(int n) const { auto retval = (*this); retval += n; return retval; }
			ipointer operator-(int n) const { return ((*this) + (-n)); }
			typename _MV::difference_type operator-(const ipointer& _Right_cref) const { return msevector_ipointer() - (_Right_cref.msevector_ipointer()); }
			typename _MV::reference operator*() const { return msevector_ipointer().operator*(); }
			typename _MV::reference item() const { return operator*(); }
			typename _MV::reference previous_item() const { return msevector_ipointer().previous_item(); }
			typename _MV::pointer operator->() const { return msevector_ipointer().operator->(); }
			typename _MV::reference operator[](typename _MV::difference_type _Off) { return (*(*this + _Off)); }
			ipointer& operator=(const ipointer& _Right_cref) { msevector_ipointer().operator=(_Right_cref.msevector_ipointer()); return (*this); }
			bool operator==(const ipointer& _Right_cref) const { return msevector_ipointer().operator==(_Right_cref.msevector_ipointer()); }
			bool operator!=(const ipointer& _Right_cref) const { return (!(_Right_cref == (*this))); }
			bool operator<(const ipointer& _Right) const { return (msevector_ipointer() < _Right.msevector_ipointer()); }
			bool operator<=(const ipointer& _Right) const { return (msevector_ipointer() <= _Right.msevector_ipointer()); }
			bool operator>(const ipointer& _Right) const { return (msevector_ipointer() > _Right.msevector_ipointer()); }
			bool operator>=(const ipointer& _Right) const { return (msevector_ipointer() >= _Right.msevector_ipointer()); }
			void set_to_item_pointer(const ipointer& _Right_cref) { msevector_ipointer().set_to_item_pointer(_Right_cref.msevector_ipointer()); }
			CSize_t position() const { return msevector_ipointer().position(); }
		private:
			std::shared_ptr<_MV> m_msevector_shptr;
			/* m_ipointer_shptr needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
			std::shared_ptr<typename _MV::ipointer> m_ipointer_shptr;
			friend class _Myt;
		};

		/* begin() and end() are provided so that ivector can be used with stl algorithms. When using ipointers directly,
		set_to_beginning() and set_to_end_marker() member functions are preferred. */
		ipointer begin() {	// return ipointer for beginning of mutable sequence
			ipointer retval(*this);
			retval.m_ipointer_shptr->set_to_beginning();
			return retval;
		}
		cipointer begin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_beginning();
			return retval;
		}
		ipointer end() {	// return ipointer for end of mutable sequence
			ipointer retval(*this);
			retval.m_ipointer_shptr->set_to_end_marker();
			return retval;
		}
		cipointer end() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_end_marker();
			return retval;
		}
		cipointer cbegin() const {	// return ipointer for beginning of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_beginning();
			return retval;
		}
		cipointer cend() const {	// return ipointer for end of nonmutable sequence
			cipointer retval(*this);
			retval.m_cipointer_shptr->set_to_beginning();
			return retval;
		}

		ivector(const cipointer &start, const cipointer &end, const _A& _Al = _A())
			: m_shptr(new _MV(start.msevector_cipointer(), end.msevector_cipointer(), _Al)) {}
		void assign(const cipointer &start, const cipointer &end) {
			m_shptr->assign(start.msevector_cipointer(), end.msevector_cipointer());
		}
		void assign_inclusive(const cipointer &first, const cipointer &last) {
			m_shptr->assign_inclusive(first.msevector_cipointer(), last.msevector_cipointer());
		}
		ipointer insert_before(ipointer &pos, typename _MV::size_type _M, const _Ty& _X) {
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), _M, _X);
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before(ipointer &pos, _Ty&& _X) {
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), std::move(_X));
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before(ipointer &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
		ipointer insert_before(ipointer &pos, const cipointer &start, const cipointer &end) {
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), start.msevector_cipointer(), end.msevector_cipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer insert_before_inclusive(ipointer &pos, const cipointer &first, const cipointer &last) {
			auto end = last; end.set_to_next();
			return insert_before_inclusive(pos, first, end);
		}
		ipointer insert_before(ipointer &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
			auto res = m_shptr->insert_before(pos.msevector_ipointer(), _Ilist);
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		void insert_before(CSize_t pos, _Ty&& _X) {
			m_shptr->insert_before(_P, std::move(_X));
		}
		void insert_before(CSize_t pos, const _Ty& _X = _Ty()) {
			m_shptr->insert_before(_P, _X);
		}
		void insert_before(CSize_t pos, typename _MV::size_type _M, const _Ty& _X) {
			m_shptr->insert_before(_P, _M, _X);
		}
		void insert_before(CSize_t pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
			m_shptr->insert_before(_P, _Ilist);
		}
		ipointer erase(ipointer &pos) {
			auto res = m_shptr->erase(pos.msevector_ipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer erase(ipointer &start, ipointer &end) {
			auto res = m_shptr->erase(start.msevector_ipointer(), end.msevector_ipointer());
			ipointer retval(*this); retval.msevector_ipointer() = res;
			return retval;
		}
		ipointer erase_inclusive(ipointer &first, ipointer &last) {
			auto end = last; end.set_to_next();
			return erase_inclusive(first, end);
		}
		bool operator==(const _Myt& _Right) const {	// test for ivector equality
			return ((*(_Right.m_shptr)) == (*m_shptr));
		}
		bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for ivectors
			return ((*m_shptr) < (*(_Right.m_shptr)));
			}

	private:
		std::shared_ptr<_MV> m_shptr;
	};

	template<class _Ty, class _Alloc> inline bool operator!=(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test for ivector inequality
			return (!(_Left == _Right));
		}

	template<class _Ty, class _Alloc> inline bool operator>(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test if _Left > _Right for ivectors
			return (_Right < _Left);
		}

	template<class _Ty, class _Alloc> inline bool operator<=(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test if _Left <= _Right for ivectors
			return (!(_Right < _Left));
		}

	template<class _Ty, class _Alloc> inline bool operator>=(const ivector<_Ty, _Alloc>& _Left,
		const ivector<_Ty, _Alloc>& _Right) {	// test if _Left >= _Right for ivectors
			return (!(_Left < _Right));
		}


	namespace mstd {

		template<class _Ty, class _A = std::allocator<_Ty> >
		class vector {
		public:
			typedef vector<_Ty, _A> _Myt;
			typedef msevector<_Ty, _A> _MV;

			msevector<_Ty, _A>& msevector() const { return (*m_shptr); }
			operator _MV() { return msevector(); }

			explicit vector(const _A& _Al = _A()) : m_shptr(new _MV(_Al)) {}
			explicit vector(typename _MV::size_type _N, const _Ty& _V = _Ty(), const _A& _Al = _A()) : m_shptr(new _MV(_N, _V, _Al)) {}
			vector(_MV&& _X) : m_shptr(new _MV(std::move(_X))) {}
			vector(const _MV& _X) : m_shptr(new _MV(_X)) {}
			vector(_Myt&& _X) : m_shptr(new _MV(std::move(_X.msevector()))) {}
			vector(const _Myt& _X) : m_shptr(new _MV(_X.msevector())) {}
			typedef typename _MV::const_iterator _It;
			vector(_It _F, _It _L, const _A& _Al = _A()) : m_shptr(new _MV(_F, _L, _Al)) {}
			vector(const _Ty* _F, const _Ty* _L, const _A& _Al = _A()) : m_shptr(new _MV(_F, _L, _Al)) {}
			template<class _Iter, class = typename enable_if<_Is_iterator<_Iter>::value, void>::type>
			vector(_Iter _First, _Iter _Last) : base_class(_First, _Last) { m_debug_size = size(); }
			template<class _Iter, class = typename enable_if<_Is_iterator<_Iter>::value, void>::type>
			vector(_Iter _First, _Iter _Last, const typename _MV::_Alloc& _Al) : base_class(_First, _Last, _Al) { m_debug_size = size(); }

			_Myt& operator=(_MV&& _X) { m_shptr->operator=(std::move(_X)); return (*this); }
			_Myt& operator=(const _MV& _X) { m_shptr->operator=(_X); return (*this); }
			_Myt& operator=(_Myt&& _X) { m_shptr->operator=(std::move(_X.msevector())); return (*this); }
			_Myt& operator=(const _Myt& _X) { m_shptr->operator=(_X.msevector()); return (*this); }
			void reserve(typename _MV::size_type _Count) { m_shptr->reserve(_Count); }
			void resize(typename _MV::size_type _N, const _Ty& _X = _Ty()) { m_shptr->resize(_N, _X); }
			typename _MV::const_reference operator[](typename _MV::size_type _P) const { return m_shptr->operator[](_P); }
			typename _MV::reference operator[](typename _MV::size_type _P) { return m_shptr->operator[](_P); }
			void push_back(_Ty&& _X) { m_shptr->push_back(std::move(_X)); }
			void push_back(const _Ty& _X) { m_shptr->push_back(_X); }
			void pop_back() { m_shptr->pop_back(); }
			void assign(_It _F, _It _L) { m_shptr->assign(_F, _L); }
			void assign(typename _MV::size_type _N, const _Ty& _X = _Ty()) { m_shptr->assign(_N, _X); }
			typename _MV::iterator insert(typename _MV::iterator _P, _Ty&& _X) { return m_shptr->insert(_P, std::move(_X)); }
			typename _MV::iterator insert(typename _MV::iterator _P, const _Ty& _X = _Ty()) { return m_shptr->insert(_P, _X); }
			typename _MV::iterator insert(typename _MV::iterator _P, typename _MV::size_type _M, const _Ty& _X) { return m_shptr->insert(_P, _M, _X); }
			template<class _Iter>
			typename std::enable_if<std::_Is_iterator<_Iter>::value, typename _MV::iterator>::type
				insert(typename _MV::const_iterator _Where, _Iter _First, _Iter _Last) { return m_shptr->insert(_Where, _First, _Last); }
			template<class... _Valty>
			void emplace_back(_Valty&&... _Val) { m_shptr->emplace_back(std::forward<_Valty>(_Val)...); }
			template<class... _Valty>
			typename _MV::iterator emplace(typename _MV::const_iterator _Where, _Valty&&... _Val) { return m_shptr->emplace(_Where, std::forward<_Valty>(_Val)...); }
			typename _MV::iterator erase(typename _MV::iterator _P) { return m_shptr->emplace(_P); }
			typename _MV::iterator erase(typename _MV::iterator _F, typename _MV::iterator _L) { return m_shptr->emplace(_F, _L); }
			void clear() { m_shptr->clear(); }
			void swap(_MV& _X) { m_shptr->swap(_X); }
			void swap(_Myt& _X) { m_shptr->swap(_X.msevector()); }

			vector(_XSTD initializer_list<typename _MV::value_type> _Ilist, const typename _MV::_Alloc& _Al = _MV::allocator_type()) : m_shptr(new _MV(_Ilist, _Al)) {}
			_Myt& operator=(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->operator=(_Ilist); return (*this); }
			void assign(_XSTD initializer_list<typename _MV::value_type> _Ilist) { m_shptr->assign(_Ilist); }
			typename _MV::iterator insert(typename _MV::const_iterator _Where, _XSTD initializer_list<typename _MV::value_type> _Ilist) { return m_shptr->insert(_Where, _Ilist); }

			typename _MV::size_type capacity() const _NOEXCEPT{ return m_shptr->capacity(); }
			void shrink_to_fit() { m_shtr->shrink_to_fit(); }
			typename _MV::size_type size() const _NOEXCEPT{ return m_shptr->size(); }
			typename _MV::size_type max_size() const _NOEXCEPT{ return m_shptr->max_size(); }
			bool empty() const _NOEXCEPT{ return m_shptr->empty(); }
			typename _MV::_Alloc get_allocator() const _NOEXCEPT{ return m_shptr->get_allocator(); }
			typename _MV::const_reference at(typename _MV::size_type _Pos) const { return m_shptr->at(_Pos); }
			typename _MV::reference at(typename _MV::size_type _Pos) { return m_shptr->at(_Pos); }
			typename _MV::reference front() { return m_shptr->front(); }
			typename _MV::const_reference front() const { return m_shptr->front(); }
			typename _MV::reference back() { return m_shptr->back(); }
			typename _MV::const_reference back() const { return m_shptr->back(); }


			class const_iterator {
			public:
				typedef typename _MV::ss_const_iterator_type::iterator_category iterator_category;
				typedef typename _MV::ss_const_iterator_type::value_type value_type;
				typedef typename _MV::ss_const_iterator_type::difference_type difference_type;
				typedef typename _MV::difference_type distance_type;	// retained
				typedef typename _MV::ss_const_iterator_type::pointer pointer;
				typedef typename _MV::ss_const_iterator_type::reference reference;

				const_iterator() : /*m_msevector_shptr(owner_cref.m_shptr), */m_ss_const_iterator_shptr(new _MV::ss_const_iterator_type()) {}
				const_iterator(const const_iterator& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_ss_const_iterator_shptr(new _MV::ss_const_iterator_type()) {
					(*this) = src_cref;
				}
				~const_iterator() {}
				typename _MV::ss_const_iterator_type& msevector_ss_const_iterator_type() const { return (*m_ss_const_iterator_shptr); }
				typename _MV::ss_const_iterator_type& mvssci() const { return msevector_ss_const_iterator_type(); }

				void reset() { msevector_ss_const_iterator_type().reset(); }
				bool points_to_an_item() const { return msevector_ss_const_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msevector_ss_const_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msevector_ss_const_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msevector_ss_const_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msevector_ss_const_iterator_type().has_next(); }
				bool has_previous() const { return msevector_ss_const_iterator_type().has_previous(); }
				void set_to_beginning() { msevector_ss_const_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msevector_ss_const_iterator_type().set_to_end_marker(); }
				void set_to_next() { msevector_ss_const_iterator_type().set_to_next(); }
				void set_to_previous() { msevector_ss_const_iterator_type().set_to_previous(); }
				const_iterator& operator ++() { msevector_ss_const_iterator_type().operator ++(); return (*this); }
				const_iterator operator++(int) { const_iterator _Tmp = *this; ++*this; return (_Tmp); }
				const_iterator& operator --() { msevector_ss_const_iterator_type().operator --(); return (*this); }
				const_iterator operator--(int) { const_iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MV::difference_type n) { msevector_ss_const_iterator_type().advance(n); }
				void regress(typename _MV::difference_type n) { msevector_ss_const_iterator_type().regress(n); }
				const_iterator& operator +=(int n) { msevector_ss_const_iterator_type().operator +=(n); return (*this); }
				const_iterator& operator -=(int n) { msevector_ss_const_iterator_type().operator -=(n); return (*this); }
				const_iterator operator+(int n) const { auto retval = (*this); retval += n; return retval; }
				const_iterator operator-(int n) const { return ((*this) + (-n)); }
				typename _MV::difference_type operator-(const const_iterator& _Right_cref) const { return msevector_ss_const_iterator_type() - (_Right_cref.msevector_ss_const_iterator_type()); }
				typename _MV::const_reference operator*() const { return msevector_ss_const_iterator_type().operator*(); }
				typename _MV::const_reference item() const { return operator*(); }
				typename _MV::const_reference previous_item() const { return msevector_ss_const_iterator_type().previous_item(); }
				typename _MV::const_pointer operator->() const { return msevector_ss_const_iterator_type().operator->(); }
				typename _MV::const_reference operator[](typename _MV::difference_type _Off) const { return (*(*this + _Off)); }
				const_iterator& operator=(const const_iterator& _Right_cref) { msevector_ss_const_iterator_type().operator=(_Right_cref.msevector_ss_const_iterator_type()); return (*this); }
				bool operator==(const const_iterator& _Right_cref) const { return msevector_ss_const_iterator_type().operator==(_Right_cref.msevector_ss_const_iterator_type()); }
				bool operator!=(const const_iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() < _Right.msevector_ss_const_iterator_type()); }
				bool operator<=(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() <= _Right.msevector_ss_const_iterator_type()); }
				bool operator>(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() > _Right.msevector_ss_const_iterator_type()); }
				bool operator>=(const const_iterator& _Right) const { return (msevector_ss_const_iterator_type() >= _Right.msevector_ss_const_iterator_type()); }
				void set_to_const_item_pointer(const const_iterator& _Right_cref) { msevector_ss_const_iterator_type().set_to_const_item_pointer(_Right_cref.msevector_ss_const_iterator_type()); }
				CSize_t position() const { return msevector_ss_const_iterator_type().position(); }
			private:
				const_iterator(std::shared_ptr<_MV> msevector_shptr) : m_msevector_shptr(msevector_shptr), m_ss_const_iterator_shptr(new _MV::ss_const_iterator_type()) {
					(*m_ss_const_iterator_shptr) = msevector_shptr->ss_cbegin();
				}
				std::shared_ptr<_MV> m_msevector_shptr;
				/* m_ss_const_iterator_shptr needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
				std::shared_ptr<typename _MV::ss_const_iterator_type> m_ss_const_iterator_shptr;
				friend class _Myt;
				friend class iterator;
			};
			class iterator {
			public:
				typedef typename _MV::ss_iterator_type::iterator_category iterator_category;
				typedef typename _MV::ss_iterator_type::value_type value_type;
				typedef typename _MV::ss_iterator_type::difference_type difference_type;
				typedef typename _MV::difference_type distance_type;	// retained
				typedef typename _MV::ss_iterator_type::pointer pointer;
				typedef typename _MV::ss_iterator_type::reference reference;

				iterator() : /*m_msevector_shptr(owner_ref.m_shptr), */m_ss_iterator_shptr(new _MV::ss_iterator_type()) {}
				iterator(const iterator& src_cref) : m_msevector_shptr(src_cref.m_msevector_shptr), m_ss_iterator_shptr(new _MV::ss_iterator_type()) {
					(*this) = src_cref;
				}
				~iterator() {}
				typename _MV::ss_iterator_type& msevector_ss_iterator_type() const { return (*m_ss_iterator_shptr); }
				typename _MV::ss_iterator_type& mvssi() const { return msevector_ss_iterator_type(); }
				//const mm_iterator_handle_type& handle() const { return (*m_handle_shptr); }
				operator const_iterator() const {
					const_iterator retval(m_msevector_shptr);
					retval.msevector_ss_const_iterator_type().set_to_beginning();
					retval.msevector_ss_const_iterator_type().advance(CInt(msevector_ss_iterator_type().position()));
					return retval;
				}

				void reset() { msevector_ss_iterator_type().reset(); }
				bool points_to_an_item() const { return msevector_ss_iterator_type().points_to_an_item(); }
				bool points_to_end_marker() const { return msevector_ss_iterator_type().points_to_end_marker(); }
				bool points_to_beginning() const { return msevector_ss_iterator_type().points_to_beginning(); }
				/* has_next_item_or_end_marker() is just an alias for points_to_an_item(). */
				bool has_next_item_or_end_marker() const { return msevector_ss_iterator_type().has_next_item_or_end_marker(); }
				/* has_next() is just an alias for points_to_an_item() that's familiar to java programmers. */
				bool has_next() const { return msevector_ss_iterator_type().has_next(); }
				bool has_previous() const { return msevector_ss_iterator_type().has_previous(); }
				void set_to_beginning() { msevector_ss_iterator_type().set_to_beginning(); }
				void set_to_end_marker() { msevector_ss_iterator_type().set_to_end_marker(); }
				void set_to_next() { msevector_ss_iterator_type().set_to_next(); }
				void set_to_previous() { msevector_ss_iterator_type().set_to_previous(); }
				iterator& operator ++() { msevector_ss_iterator_type().operator ++(); return (*this); }
				iterator operator++(int) { iterator _Tmp = *this; ++*this; return (_Tmp); }
				iterator& operator --() { msevector_ss_iterator_type().operator --(); return (*this); }
				iterator operator--(int) { iterator _Tmp = *this; --*this; return (_Tmp); }
				void advance(typename _MV::difference_type n) { msevector_ss_iterator_type().advance(n); }
				void regress(typename _MV::difference_type n) { msevector_ss_iterator_type().regress(n); }
				iterator& operator +=(int n) { msevector_ss_iterator_type().operator +=(n); return (*this); }
				iterator& operator -=(int n) { msevector_ss_iterator_type().operator -=(n); return (*this); }
				iterator operator+(int n) const { auto retval = (*this); retval += n; return retval; }
				iterator operator-(int n) const { return ((*this) + (-n)); }
				typename _MV::difference_type operator-(const iterator& _Right_cref) const { return msevector_ss_iterator_type() - (_Right_cref.msevector_ss_iterator_type()); }
				typename _MV::reference operator*() const { return msevector_ss_iterator_type().operator*(); }
				typename _MV::reference item() const { return operator*(); }
				typename _MV::reference previous_item() const { return msevector_ss_iterator_type().previous_item(); }
				typename _MV::pointer operator->() const { return msevector_ss_iterator_type().operator->(); }
				typename _MV::reference operator[](typename _MV::difference_type _Off) { return (*(*this + _Off)); }
				iterator& operator=(const iterator& _Right_cref) { msevector_ss_iterator_type().operator=(_Right_cref.msevector_ss_iterator_type()); return (*this); }
				bool operator==(const iterator& _Right_cref) const { return msevector_ss_iterator_type().operator==(_Right_cref.msevector_ss_iterator_type()); }
				bool operator!=(const iterator& _Right_cref) const { return (!(_Right_cref == (*this))); }
				bool operator<(const iterator& _Right) const { return (msevector_ss_iterator_type() < _Right.msevector_ss_iterator_type()); }
				bool operator<=(const iterator& _Right) const { return (msevector_ss_iterator_type() <= _Right.msevector_ss_iterator_type()); }
				bool operator>(const iterator& _Right) const { return (msevector_ss_iterator_type() > _Right.msevector_ss_iterator_type()); }
				bool operator>=(const iterator& _Right) const { return (msevector_ss_iterator_type() >= _Right.msevector_ss_iterator_type()); }
				void set_to_item_pointer(const iterator& _Right_cref) { msevector_ss_iterator_type().set_to_item_pointer(_Right_cref.msevector_ss_iterator_type()); }
				CSize_t position() const { return msevector_ss_iterator_type().position(); }
			private:
				std::shared_ptr<_MV> m_msevector_shptr;
				/* m_ss_iterator_shptr needs to be declared after m_msevector_shptr so that it's destructor will be called first. */
				std::shared_ptr<typename _MV::ss_iterator_type> m_ss_iterator_shptr;
				friend class _Myt;
			};

			iterator begin()
			{	// return iterator for beginning of mutable sequence
				iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(*(retval.m_ss_iterator_shptr)) = m_shptr->ss_begin();
				return retval;
			}

			const_iterator begin() const
			{	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(*(retval.m_ss_const_iterator_shptr)) = m_shptr->ss_begin();
				return retval;
			}

			iterator end() {	// return iterator for end of mutable sequence
				iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(*(retval.m_ss_iterator_shptr)) = m_shptr->ss_end();
				return retval;
			}
			const_iterator end() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(*(retval.m_ss_const_iterator_shptr)) = m_shptr->ss_end();
				return retval;
			}
			const_iterator cbegin() const {	// return iterator for beginning of nonmutable sequence
				const_iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(*(retval.m_ss_const_iterator_shptr)) = m_shptr->ss_cbegin();
				return retval;
			}
			const_iterator cend() const {	// return iterator for end of nonmutable sequence
				const_iterator retval; retval.m_msevector_shptr = this->m_shptr;
				(*(retval.m_ss_const_iterator_shptr)) = m_shptr->ss_cend();
				return retval;
			}


			vector(const const_iterator &start, const const_iterator &end, const _A& _Al = _A())
				: m_shptr(new _MV(start.msevector_ss_const_iterator_type(), end.msevector_ss_const_iterator_type(), _Al)) {}
			void assign(const const_iterator &start, const const_iterator &end) {
				m_shptr->assign(start.msevector_ss_const_iterator_type(), end.msevector_ss_const_iterator_type());
			}
			void assign_inclusive(const const_iterator &first, const const_iterator &last) {
				m_shptr->assign_inclusive(first.msevector_ss_const_iterator_type(), last.msevector_ss_const_iterator_type());
			}
			iterator insert_before(iterator &pos, typename _MV::size_type _M, const _Ty& _X) {
				auto res = m_shptr->insert_before(pos.msevector_ss_iterator_type(), _M, _X);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(iterator &pos, _Ty&& _X) {
				auto res = m_shptr->insert_before(pos.msevector_ss_iterator_type(), std::move(_X));
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(iterator &pos, const _Ty& _X = _Ty()) { return insert_before(pos, 1, _X); }
			iterator insert_before(iterator &pos, const const_iterator &start, const const_iterator &end) {
				auto res = m_shptr->insert_before(pos.msevector_ss_iterator_type(), start.msevector_ss_const_iterator_type(), end.msevector_ss_const_iterator_type());
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before(iterator &pos, const _Ty* start, const _Ty* end) {
				auto res = m_shptr->insert_before(pos.msevector_ss_iterator_type(), start, end);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator insert_before_inclusive(iterator &pos, const const_iterator &first, const const_iterator &last) {
				auto end = last; end.set_to_next();
				return insert_before_inclusive(pos, first, end);
			}
			iterator insert_before(iterator &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
				auto res = m_shptr->insert_before(pos.msevector_ss_iterator_type(), _Ilist);
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			void insert_before(CSize_t pos, const _Ty& _X = _Ty()) {
				m_shptr->insert_before(_P, _X);
			}
			void insert_before(CSize_t pos, typename _MV::size_type _M, const _Ty& _X) {
				m_shptr->insert_before(_P, _M, _X);
			}
			void insert_before(CSize_t pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) {	// insert initializer_list
				m_shptr->insert_before(_P, _Ilist);
			}
			/* These insert() functions are just aliases for their corresponding insert_before() functions. */
			iterator insert(iterator &pos, typename _MV::size_type _M, const _Ty& _X) { return insert_before(pos, _M, _X); }
			iterator insert(iterator &pos, _Ty&& _X) { return insert_before(pos, std::move(_X)); }
			iterator insert(iterator &pos, const _Ty& _X = _Ty()) { return insert_before(pos, _X); }
			iterator insert(iterator &pos, const const_iterator &start, const const_iterator &end) { return insert_before(pos, start, end); }
			iterator insert(iterator &pos, const _Ty* start, const _Ty* end) { return insert_before(pos, start, end); }
			iterator insert(iterator &pos, _XSTD initializer_list<typename _MV::value_type> _Ilist) { return insert_before(pos, _Ilist); }
			iterator erase(iterator &pos) {
				auto res = m_shptr->erase(pos.msevector_ss_iterator_type());
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator erase(iterator &start, iterator &end) {
				auto res = m_shptr->erase(start.msevector_ss_iterator_type(), end.msevector_ss_iterator_type());
				iterator retval = begin(); retval.msevector_ss_iterator_type() = res;
				return retval;
			}
			iterator erase_inclusive(iterator &first, iterator &last) {
				auto end = last; end.set_to_next();
				return erase_inclusive(first, end);
			}
			bool operator==(const _Myt& _Right) const {	// test for vector equality
				return ((*(_Right.m_shptr)) == (*m_shptr));
			}
			bool operator<(const _Myt& _Right) const {	// test if _Left < _Right for vectors
				return ((*m_shptr) < (*(_Right.m_shptr)));
			}

		private:
			std::shared_ptr<_MV> m_shptr;
		};

		template<class _Ty, class _Alloc> inline bool operator!=(const vector<_Ty, _Alloc>& _Left,
			const vector<_Ty, _Alloc>& _Right) {	// test for vector inequality
			return (!(_Left == _Right));
		}

		template<class _Ty, class _Alloc> inline bool operator>(const vector<_Ty, _Alloc>& _Left,
			const vector<_Ty, _Alloc>& _Right) {	// test if _Left > _Right for vectors
			return (_Right < _Left);
		}

		template<class _Ty, class _Alloc> inline bool operator<=(const vector<_Ty, _Alloc>& _Left,
			const vector<_Ty, _Alloc>& _Right) {	// test if _Left <= _Right for vectors
			return (!(_Right < _Left));
		}

		template<class _Ty, class _Alloc> inline bool operator>=(const vector<_Ty, _Alloc>& _Left,
			const vector<_Ty, _Alloc>& _Right) {	// test if _Left >= _Right for vectors
			return (!(_Left < _Right));
		}
	}
}
#endif /*ndef MSETL_H*/