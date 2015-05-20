
// Copyright (c) 2015 Noah Lopez
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//include "msetl.h"
#include "msemsevector.h"
#include "msemstdvector.h"
#include "mseivector.h"
#include "msevector_test.h"
#include <iostream>


int main(int argc, char* argv[])
{
	mse::msevector_test msevector_test;
	msevector_test.run_all();

	{
		double a1[3] = { 1.0, 2.0, 3.0 };
		double *d_pointer1 = &(a1[0]);
		double a2[3] = { 4.0, 5.0, 360 };
		double *d_pointer2 = &(a2[0]);
		mse::mstd::vector<double> v1;
		//v1.insert(v1.begin(), d_pointer1, d_pointer2); /* not good */
		/* std::vector supports "naked" pointers as parameters to the insert() member
		function so mse::mstd::vector does also. Unfortunately there is no way to ensure
		that the naked pointer parameters have valid values. */

#ifdef MSVC2010_COMPATIBILE
		mse::mstd::vector<double> v2(a1, a1+3);
		mse::mstd::vector<double> v3(a2, a2+3);
		mse::mstd::vector<double> v4;
#else /*MSVC2010_COMPATIBILE*/
		mse::mstd::vector<double> v2 = { 1.0, 2.0, 3.0 };
		mse::mstd::vector<double> v3 = { 4.0, 5.0, 360 };
		mse::mstd::vector<double> v4;
#endif /*MSVC2010_COMPATIBILE*/
		try {
			v4.insert(v4.begin(), v2.begin(), v3.begin());
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by a comparision of incompatible "safe" iterators. */
		}
	}

	{
		typedef mse::mstd::vector<int> vint_type;
		mse::mstd::vector<vint_type> vvi;
		{
			vint_type vi;
			vi.push_back(5);
			vvi.push_back(vi);
		}
		auto vi_it = vvi[0].begin();
		vvi.clear();
		/* At this point, the vint_type object is cleared from vvi, but it has not been deallocated/destructed yet because it
		"knows" that there is an iterator, namely vi_it, that is still referencing it. At the moment, std::shared_ptrs are being
		used to achieve this. */
		auto value = (*vi_it); /* So this is actually ok. vi_it still points to a valid item. */
		assert(5 == value);
		vint_type vi2;
		vi_it = vi2.begin();
		/* The vint_type object that vi_it was originally pointing to is now deallocated/destructed, because vi_it no longer
		references it. */
	}

	{
#ifdef MSVC2010_COMPATIBILE
		int a1[4] = { 1, 2, 3, 4 };
		mse::msevector<int> v(a1, a1+4);
#else /*MSVC2010_COMPATIBILE*/
		mse::msevector<int> v = { 1, 2, 3, 4 };
#endif /*MSVC2010_COMPATIBILE*/
		mse::msevector<int>::ipointer ip_vit1(v);
		/*ip_vit1.set_to_beginning();*/ /* This would be redundant as ipointers are set to the beginning at initialization. */
		ip_vit1.advance(2);
		assert(3 == ip_vit1.item());
		auto ip_vit2 = v.ibegin(); /* ibegin() returns an ipointer */
		v.erase(ip_vit2); /* remove the first item */
		assert(3 == ip_vit1.item());
		ip_vit1.set_to_previous();
		assert(2 == ip_vit1.item());
	}

	{
		/* The unsigned types like size_t can cause insidious bugs due to the fact that they can cause signed integers to be
		implicitly converted to unsigned. msetl provides substitutes for size_t and int that change the implicit conversion to
		instead be from unsigned to signed. */

		mse::s_type_test1();

		{
			size_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			int minimum_number_of_security_credits_required_for_access = 7;
			bool access_granted = false;
			if (0 <= number_of_security_credits - minimum_number_of_security_credits_required_for_access) {
				/* You may not even get a compiler warning about the implicit conversion from (signed) int to (unsigned) size_t. */
				access_granted = true; /*oops*/
			}
			else {
				access_granted = false;
				assert(false);
			}
		}

		{
			mse::CSize_t number_of_security_credits; /* initializes to 0 by default */
			number_of_security_credits += 5;
			int minimum_number_of_security_credits_required_for_access = 7;
			bool access_granted = false;
			if (0 <= number_of_security_credits - minimum_number_of_security_credits_required_for_access) {
				access_granted = true;
				assert(false);
			}
			else {
				access_granted = false; /* that's better */
			}
		}

		{
			size_t number_of_security_credits = 0;
			number_of_security_credits += 5;
			mse::CInt minimum_number_of_security_credits_required_for_access = 7;
			mse::CBool access_granted; /* initializes to false by default */
			if (0 <= number_of_security_credits - minimum_number_of_security_credits_required_for_access) {
				access_granted = true;
				assert(false);
			}
			else {
				access_granted = false; /* this works too */
			}
		}

		mse::CSize_t mse_szt1;
		/* size_t szt2 = mse_szt1; */ /* This wouldn't compile. */
#ifdef MSVC2010_COMPATIBILE
		size_t szt1 = mse::as_a_size_t(mse_szt1); /* We exclude automatic conversion from mse::CSize_t to size_t because we
													 consider size_t an intrinsically error prone type. */
#else /*MSVC2010_COMPATIBILE*/
		size_t szt1 = static_cast<size_t>(mse_szt1); /* We exclude automatic conversion from mse::CSize_t to size_t because we
													 consider size_t an intrinsically error prone type. */
#endif /*MSVC2010_COMPATIBILE*/

		try {
			mse::CSize_t mse_szt2;
			mse_szt2 = -3;
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by an "out of range" assignment to an mse::CSize_t. */
		}

		try {
			mse::CSize_t mse_szt3 = 3;
			mse_szt3 -= 1; /* this is fine */
			mse_szt3 -= 4; /* this is gonna throw an exception */
		}
		catch (...) {
			std::cerr << "expected exception" << std::endl;
			/* The exception is triggered by an attempt to set an mse::CSize_t to an "out of range" value. */
		}
	}

	return 0;
}

